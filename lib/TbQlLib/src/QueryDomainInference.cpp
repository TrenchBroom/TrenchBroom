/*
 Copyright (C) 2026 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ql/QueryDomainInference.h"

#include "el/Expression.h"
#include "ql/BrushFaceLazyMap.h"
#include "ql/BrushNodeLazyMap.h"
#include "ql/EntityNodeLazyMap.h"
#include "ql/GroupNodeLazyMap.h"
#include "ql/LayerNodeLazyMap.h"
#include "ql/PatchNodeLazyMap.h"
#include "ql/WorldNodeLazyMap.h"

#include "kd/flat_map.h"
#include "kd/optional_utils.h"
#include "kd/overload.h"
#include "kd/ranges/fold.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>

namespace tb::ql
{
namespace
{

QueryDomain fullDomain()
{
  return {
    QueryObjectType::World,
    QueryObjectType::Layer,
    QueryObjectType::Group,
    QueryObjectType::Entity,
    QueryObjectType::Brush,
    QueryObjectType::Patch,
    QueryObjectType::Face,
  };
}

QueryDomain intersectDomains(const QueryDomain& lhs, const QueryDomain& rhs)
{
  auto result = QueryDomain{};
  std::ranges::set_intersection(lhs, rhs, std::inserter(result, result.end()));
  return result;
}

QueryDomain unionDomains(const QueryDomain& lhs, const QueryDomain& rhs)
{
  auto result = QueryDomain{};
  std::ranges::set_union(lhs, rhs, std::inserter(result, result.end()));
  return result;
}

QueryDomain complementDomain(const QueryDomain& domain)
{
  auto result = QueryDomain{};
  std::ranges::set_difference(fullDomain(), domain, std::inserter(result, result.end()));
  return result;
}

const QueryDomain& fieldDomain(const std::string& fieldName)
{
  // a name that isn't a field doesn't narrow the domain
  static const auto full = fullDomain();
  const auto iField = queryFields().find(fieldName);
  return iField != queryFields().end() ? iField->second.domain : full;
}

bool isVariableNamed(const el::ExpressionNode& node, const std::string& name)
{
  return node.accept(kdl::overload(
    [&](const el::VariableExpression& v) { return v.variableName == name; },
    [](const auto&) { return false; }));
}

std::optional<std::string> asStringLiteral(const el::ExpressionNode& node)
{
  return node.accept(kdl::overload(
    [](const el::LiteralExpression& lit) -> std::optional<std::string> {
      if (lit.value.type() != el::ValueType::String)
      {
        return std::nullopt;
      }
      return lit.value.stringValue();
    },
    [](const auto&) -> std::optional<std::string> { return std::nullopt; }));
}

std::optional<QueryObjectType> asTypeName(const el::ExpressionNode& node)
{
  return asStringLiteral(node) | kdl::optional_and_then(queryObjectTypeFromName);
}

// an array literal hints only if every element is a type name
std::optional<QueryDomain> asTypeNames(const el::ExpressionNode& node)
{
  return node.accept(kdl::overload(
    [](const el::ArrayExpression& a) -> std::optional<QueryDomain> {
      auto domain = QueryDomain{};
      for (const auto& element : a.elements)
      {
        const auto type = asTypeName(element);
        if (!type)
        {
          return std::nullopt;
        }
        domain.insert(*type);
      }
      return domain;
    },
    [](const auto&) -> std::optional<QueryDomain> { return std::nullopt; }));
}

std::optional<QueryDomain> asTypeEquality(
  const el::ExpressionNode& lhs, const el::ExpressionNode& rhs)
{
  const auto type = isVariableNamed(lhs, "type")   ? asTypeName(rhs)
                    : isVariableNamed(rhs, "type") ? asTypeName(lhs)
                                                   : std::nullopt;
  return type | kdl::optional_transform([](const auto t) { return QueryDomain{t}; });
}

std::optional<QueryDomain> asTypeMembership(
  const el::ExpressionNode& element, const el::ExpressionNode& array)
{
  return isVariableNamed(element, "type") ? asTypeNames(array) : std::nullopt;
}

/**
 * Comparing `type` with literal kinds is an explicit domain hint: `type == "kind"` and
 * `type is "kind"` (with the operands in either order), `type in ["kind", ...]` and
 * `["kind", ...] contains type`. In the absence of references to kind-specific fields,
 * this is the only way to narrow a query (e.g. `type == "layer" && visible == false`).
 */
std::optional<QueryDomain> asTypeHint(const el::BinaryExpression& expr)
{
  const auto& lhs = expr.leftOperand;
  const auto& rhs = expr.rightOperand;
  return std::visit(
    kdl::overload(
      [&](const el::binop::Equal&) { return asTypeEquality(lhs, rhs); },
      [&](const el::binop::InfixCall& call) -> std::optional<QueryDomain> {
        if (call.functionName == "is")
        {
          return asTypeEquality(lhs, rhs);
        }
        if (call.functionName == "in")
        {
          return asTypeMembership(lhs, rhs);
        }
        if (call.functionName == "contains")
        {
          return asTypeMembership(rhs, lhs);
        }
        return std::nullopt;
      },
      [](const auto&) -> std::optional<QueryDomain> { return std::nullopt; }),
    expr.operation);
}

QueryDomain inferQueryDomainRecursively(const el::ExpressionNode& expression);

QueryDomain inferNegatedQueryDomain(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [&](const el::UnaryExpression& u) {
      switch (u.operation)
      {
      case el::UnaryOperation::Group:
        return inferNegatedQueryDomain(u.operand);
      case el::UnaryOperation::LogicalNegation:
        return inferQueryDomainRecursively(u.operand);
      default:
        return inferQueryDomainRecursively(expression);
      }
    },
    [&](const el::BinaryExpression& b) {
      if (const auto hint = asTypeHint(b))
      {
        return complementDomain(*hint);
      }

      return std::visit(
        kdl::overload(
          [&](const el::binop::LogicalAnd&) {
            return unionDomains(
              inferNegatedQueryDomain(b.leftOperand),
              inferNegatedQueryDomain(b.rightOperand));
          },
          [&](const el::binop::LogicalOr&) {
            return intersectDomains(
              inferNegatedQueryDomain(b.leftOperand),
              inferNegatedQueryDomain(b.rightOperand));
          },
          [&](const auto&) { return inferQueryDomainRecursively(expression); }),
        b.operation);
    },
    [&](const auto&) { return inferQueryDomainRecursively(expression); }));
}

QueryDomain inferQueryDomainRecursively(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [](const el::LiteralExpression&) { return fullDomain(); },
    [](const el::VariableExpression& v) { return fieldDomain(v.variableName); },
    [](const el::ArrayExpression& a) {
      const auto elementDomains =
        a.elements | std::views::transform(inferQueryDomainRecursively);
      return kdl::ranges::fold_left_first(elementDomains, unionDomains)
        .value_or(fullDomain());
    },
    [](const el::MapExpression& m) {
      const auto elementDomains = m.elements | std::views::values
                                  | std::views::transform(inferQueryDomainRecursively);
      return kdl::ranges::fold_left_first(elementDomains, unionDomains)
        .value_or(fullDomain());
    },
    [](const el::UnaryExpression& u) {
      return u.operation == el::UnaryOperation::LogicalNegation
               ? inferNegatedQueryDomain(u.operand)
               : inferQueryDomainRecursively(u.operand);
    },
    [](const el::BinaryExpression& b) {
      if (const auto hint = asTypeHint(b))
      {
        return *hint;
      }

      const auto lhs = inferQueryDomainRecursively(b.leftOperand);
      const auto rhs = inferQueryDomainRecursively(b.rightOperand);
      return std::visit(
        kdl::overload(
          [&](const el::binop::LogicalOr&) { return unionDomains(lhs, rhs); },
          [&](const auto&) { return intersectDomains(lhs, rhs); }),
        b.operation);
    },
    [](const el::SubscriptExpression& s) {
      return intersectDomains(
        inferQueryDomainRecursively(s.leftOperand),
        inferQueryDomainRecursively(s.rightOperand));
    },
    [](const el::DotExpression& d) { return inferQueryDomainRecursively(d.operand); },
    [](const el::CallExpression& c) {
      const auto argumentDomains =
        c.arguments | std::views::transform(inferQueryDomainRecursively);
      return kdl::ranges::fold_left(argumentDomains, fullDomain(), intersectDomains);
    },
    [](const el::SwitchExpression& s) {
      // a switch is true if one of its cases is, and a case `c -> v` is only true if both
      // `c` and `v` are, like `c && v`
      const auto caseDomains =
        s.cases | std::views::transform(inferQueryDomainRecursively);
      return kdl::ranges::fold_left(caseDomains, QueryDomain{}, unionDomains);
    }));
}

} // namespace

std::ostream& operator<<(std::ostream& str, const QueryObjectType type)
{
  switch (type)
  {
  case QueryObjectType::World:
    str << "world";
    break;
  case QueryObjectType::Layer:
    str << "layer";
    break;
  case QueryObjectType::Group:
    str << "group";
    break;
  case QueryObjectType::Entity:
    str << "entity";
    break;
  case QueryObjectType::Brush:
    str << "brush";
    break;
  case QueryObjectType::Patch:
    str << "patch";
    break;
  case QueryObjectType::Face:
    str << "face";
    break;
  }
  return str;
}

std::optional<QueryObjectType> queryObjectTypeFromName(const std::string& name)
{
  if (name == "world")
  {
    return QueryObjectType::World;
  }
  if (name == "layer")
  {
    return QueryObjectType::Layer;
  }
  if (name == "group")
  {
    return QueryObjectType::Group;
  }
  if (name == "entity")
  {
    return QueryObjectType::Entity;
  }
  if (name == "brush")
  {
    return QueryObjectType::Brush;
  }
  if (name == "patch")
  {
    return QueryObjectType::Patch;
  }
  if (name == "face")
  {
    return QueryObjectType::Face;
  }
  return std::nullopt;
}

const kdl::flat_map<std::string, QueryField>& queryFields()
{
  static const auto fields = [] {
    auto result = kdl::flat_map<std::string, QueryField>{};
    const auto add = [&](const auto objectType, const auto& fieldTypes) {
      for (const auto& [name, valueType] : fieldTypes)
      {
        auto& field = result[name];
        field.domain.insert(objectType);
        field.valueType = valueType;
      }
    };
    add(QueryObjectType::World, worldNodeFieldTypes());
    add(QueryObjectType::Layer, layerNodeFieldTypes());
    add(QueryObjectType::Group, groupNodeFieldTypes());
    add(QueryObjectType::Entity, entityNodeFieldTypes());
    add(QueryObjectType::Brush, brushNodeFieldTypes());
    add(QueryObjectType::Patch, patchNodeFieldTypes());
    add(QueryObjectType::Face, brushFaceFieldTypes());
    return result;
  }();
  return fields;
}

QueryDomain inferQueryDomain(const el::ExpressionNode& expression)
{
  auto domain = inferQueryDomainRecursively(expression);
  if (domain == fullDomain())
  {
    // no narrowing signal at all -- default to every object type except face
    domain.erase(QueryObjectType::Face);
  }
  if (domain != QueryDomain{QueryObjectType::Layer})
  {
    // a matching layer selects all objects in it, so only search layers on their own
    domain.erase(QueryObjectType::Layer);
  }
  return domain;
}

} // namespace tb::ql
