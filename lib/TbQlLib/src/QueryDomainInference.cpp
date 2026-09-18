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

#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "ql/BrushFaceBinding.h"
#include "ql/BrushNodeBinding.h"
#include "ql/EntityNodeBinding.h"
#include "ql/GroupNodeBinding.h"
#include "ql/LayerNodeBinding.h"
#include "ql/PatchNodeBinding.h"
#include "ql/WorldNodeBinding.h"

#include "kd/flat_map.h"
#include "kd/overload.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>

namespace tb::ql
{

std::ostream& operator<<(std::ostream& str, const QueryKind kind)
{
  switch (kind)
  {
  case QueryKind::World:
    str << "world";
    break;
  case QueryKind::Layer:
    str << "layer";
    break;
  case QueryKind::Group:
    str << "group";
    break;
  case QueryKind::Entity:
    str << "entity";
    break;
  case QueryKind::Brush:
    str << "brush";
    break;
  case QueryKind::Patch:
    str << "patch";
    break;
  case QueryKind::Face:
    str << "face";
    break;
  }
  return str;
}

namespace
{

std::optional<QueryKind> kindFromName(const std::string& name)
{
  if (name == "world")
  {
    return QueryKind::World;
  }
  if (name == "layer")
  {
    return QueryKind::Layer;
  }
  if (name == "group")
  {
    return QueryKind::Group;
  }
  if (name == "entity")
  {
    return QueryKind::Entity;
  }
  if (name == "brush")
  {
    return QueryKind::Brush;
  }
  if (name == "patch")
  {
    return QueryKind::Patch;
  }
  if (name == "face")
  {
    return QueryKind::Face;
  }
  return std::nullopt;
}

QueryDomain fullDomain()
{
  return {
    QueryKind::World,
    QueryKind::Layer,
    QueryKind::Group,
    QueryKind::Entity,
    QueryKind::Brush,
    QueryKind::Patch,
    QueryKind::Face,
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
    const auto add = [&](const auto kind, const auto& names) {
      for (const auto& name : names)
      {
        t[name].insert(kind);
      }
    };
    add(QueryKind::World, worldNodeFieldNames());
    add(QueryKind::Layer, layerNodeFieldNames());
    add(QueryKind::Group, groupNodeFieldNames());
    add(QueryKind::Entity, entityNodeFieldNames());
    add(QueryKind::Brush, brushNodeFieldNames());
    add(QueryKind::Patch, patchNodeFieldNames());
    add(QueryKind::Face, brushFaceFieldNames());

    // a field present in every kind's table has no narrowing effect -- drop it so the
    // fallthrough to `full` below applies, exactly as for a field absent from all tables
    kdl::erase_if(t, [&](const auto& entry) { return entry.second == full; });
    return t;
  }();

  const auto it = table.find(fieldName);
  return it != table.end() ? it->second : full;
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
      // a string literal never references a variable, so any context will do
      return el::withEvaluationContext([&](auto&) { return lit.value.stringValue(); })
        .value();
    },
    [](const auto&) -> std::optional<std::string> { return std::nullopt; }));
}

/**
 * A literal `type == "kind"` (or `"kind" == type`) comparison is an explicit domain
 * hint -- the only way to narrow a query that references no kind-specific field at all
 * (e.g. `type == "layer" && visible == false`).
 */
std::optional<QueryKind> asTypeHint(const el::BinaryExpression& expr)
{
  if (!std::holds_alternative<el::binop::Equal>(expr.operation))
  {
    return std::nullopt;
  }

  if (isVariableNamed(expr.leftOperand, "type"))
  {
    if (const auto name = asStringLiteral(expr.rightOperand))
    {
      return kindFromName(*name);
    }
  }
  if (isVariableNamed(expr.rightOperand, "type"))
  {
    if (const auto name = asStringLiteral(expr.leftOperand))
    {
      return kindFromName(*name);
    }
  }
  return std::nullopt;
}

QueryDomain inferQueryDomainRecursively(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [](const el::LiteralExpression&) { return fullDomain(); },
    [](const el::VariableExpression& v) { return fieldDomain(v.variableName); },
    [](const el::ArrayExpression& a) {
      auto result = fullDomain();
      for (const auto& element : a.elements)
      {
        result = intersectDomains(result, inferQueryDomainRecursively(element));
      }
      return result;
    },
    [](const el::MapExpression& m) {
      auto result = fullDomain();
      for (const auto& [key, value] : m.elements)
      {
        result = intersectDomains(result, inferQueryDomainRecursively(value));
      }
      return result;
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
      auto result = fullDomain();
      for (const auto& argument : c.arguments)
      {
        result = intersectDomains(result, inferQueryDomainRecursively(argument));
      }
      return result;
    },
    [](const el::SwitchExpression& s) {
      auto result = QueryDomain{};
      for (const auto& case_ : s.cases)
      {
        result = unionDomains(result, inferQueryDomainRecursively(case_));
      }
      return result;
    }));
}

} // namespace

QueryDomain inferQueryDomain(const el::ExpressionNode& expression)
{
  const auto domain = inferQueryDomainRecursively(expression);
  if (domain == fullDomain())
  {
    // no narrowing signal at all -- default to every node kind, never face
    auto result = domain;
    result.erase(QueryKind::Face);
    return result;
  }
  return domain;
}

} // namespace tb::ql
