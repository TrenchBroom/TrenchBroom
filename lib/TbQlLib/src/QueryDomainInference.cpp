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
#include "kd/overload.h"
#include "kd/range_fold.h"
#include "kd/unpack.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <ostream>
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

const QueryDomain& fieldDomain(const std::string& fieldName)
{
  static const auto full = fullDomain();
  static const auto table = [] {
    auto t = kdl::flat_map<std::string, QueryDomain>{};
    const auto add = [&](const auto type, const auto& names) {
      for (const auto& name : names)
      {
        t[name].insert(type);
      }
    };
    add(QueryObjectType::World, worldNodeFieldNames());
    add(QueryObjectType::Layer, layerNodeFieldNames());
    add(QueryObjectType::Group, groupNodeFieldNames());
    add(QueryObjectType::Entity, entityNodeFieldNames());
    add(QueryObjectType::Brush, brushNodeFieldNames());
    add(QueryObjectType::Patch, patchNodeFieldNames());
    add(QueryObjectType::Face, brushFaceFieldNames());

    // a field present in every kind's table has no narrowing effect: drop it so the
    // fallthrough to `full` below applies, exactly as for a field absent from all tables
    kdl::erase_if(
      t, kdl::unpack([&](const auto&, const auto& domain) { return domain == full; }));
    return t;
  }();

  const auto iDomain = table.find(fieldName);
  return iDomain != table.end() ? iDomain->second : full;
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

/**
 * A literal `type == "kind"` (or `"kind" == type`) comparison is an explicit domain hint.
 * In the absence of references to kind-specific fields, this is the only way to narrow a
 * query (e.g. `type == "layer" && visible == false`).
 */
std::optional<QueryObjectType> asTypeHint(const el::BinaryExpression& expr)
{
  return std::visit(
    kdl::overload(
      [&](const el::binop::Equal&) -> std::optional<QueryObjectType> {
        if (isVariableNamed(expr.leftOperand, "type"))
        {
          if (const auto name = asStringLiteral(expr.rightOperand))
          {
            return queryObjectTypeFromName(*name);
          }
        }
        if (isVariableNamed(expr.rightOperand, "type"))
        {
          if (const auto name = asStringLiteral(expr.leftOperand))
          {
            return queryObjectTypeFromName(*name);
          }
        }
        return std::nullopt;
      },
      [](const auto&) -> std::optional<QueryObjectType> { return std::nullopt; }),
    expr.operation);
}

QueryDomain inferQueryDomainRecursively(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [](const el::LiteralExpression&) { return fullDomain(); },
    [](const el::VariableExpression& v) { return fieldDomain(v.variableName); },
    [](const el::ArrayExpression& a) {
      const auto elementDomains =
        a.elements | std::views::transform(inferQueryDomainRecursively);
      return kdl::fold_left(elementDomains, fullDomain(), intersectDomains);
    },
    [](const el::MapExpression& m) {
      const auto elementDomains = m.elements | std::views::values
                                  | std::views::transform(inferQueryDomainRecursively);
      return kdl::fold_left(elementDomains, fullDomain(), intersectDomains);
    },
    [](const el::UnaryExpression& u) { return inferQueryDomainRecursively(u.operand); },
    [](const el::BinaryExpression& b) {
      if (const auto hint = asTypeHint(b))
      {
        return QueryDomain{*hint};
      }

      const auto lhs = inferQueryDomainRecursively(b.leftOperand);
      const auto rhs = inferQueryDomainRecursively(b.rightOperand);
      return std::visit(
        kdl::overload(
          [&](const el::binop::LogicalOr&) { return unionDomains(lhs, rhs); },
          // a switch case pairs a condition with a result value; either side
          // referencing a field is a signal, so a case unions like LogicalOr
          [&](const el::binop::Case&) { return unionDomains(lhs, rhs); },
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
      return kdl::fold_left(argumentDomains, fullDomain(), intersectDomains);
    },
    [](const el::SwitchExpression& s) {
      const auto caseDomains =
        s.cases | std::views::transform(inferQueryDomainRecursively);
      return kdl::fold_left(caseDomains, fullDomain(), unionDomains);
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

QueryDomain inferQueryDomain(const el::ExpressionNode& expression)
{
  auto domain = inferQueryDomainRecursively(expression);
  if (domain == fullDomain())
  {
    // no narrowing signal at all -- default to every object type except face
    domain.erase(QueryObjectType::Face);
    return domain;
  }
  return domain;
}

} // namespace tb::ql
