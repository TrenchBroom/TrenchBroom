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

#include "mdl/QueryDomainInference.h"

#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "el/Value.h"

#include "kd/overload.h"
#include "kd/result.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace tb::mdl
{
namespace
{

const auto AllKinds = QueryDomain{
  QueryKind::World,
  QueryKind::Layer,
  QueryKind::Group,
  QueryKind::Entity,
  QueryKind::Brush,
  QueryKind::Patch,
  QueryKind::Face,
};

const auto AllNodeKinds = QueryDomain{
  QueryKind::World,
  QueryKind::Layer,
  QueryKind::Group,
  QueryKind::Entity,
  QueryKind::Brush,
  QueryKind::Patch,
};

QueryDomain intersect(const QueryDomain& lhs, const QueryDomain& rhs)
{
  auto result = QueryDomain{};
  std::set_intersection(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::inserter(result, result.end()));
  return result;
}

QueryDomain unite(const QueryDomain& lhs, const QueryDomain& rhs)
{
  auto result = QueryDomain{};
  std::set_union(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::inserter(result, result.end()));
  return result;
}

/**
 * The compatible kind set for each schema field name (see `NodeVariableStore` and
 * `FaceVariableStore`), matching the field -> kind-set table in the query language spec.
 * Fields that place no domain constraint of their own (`type`, `bounds`, `center`,
 * `layerName`, `groupName`, `visible`, `locked`, `selected`, `linked`) map to `AllKinds`
 * rather than `AllNodeKinds`, so that they never spuriously conflict with a genuine
 * face-domain signal elsewhere in the same expression (e.g.
 * `type == "face" && intersects(bounds, ...)`) -- the "no signal at all -> six node
 * kinds" default is applied once, at the top of `inferQueryDomain`, not per field.
 *
 * Note: `linked` has no meaning in face domain (it isn't part of the face schema at
 * all), but is grouped here with the other domain-agnostic fields anyway, matching the
 * query language spec's field table. This means `linked && type == "face"` is not
 * flagged as unsatisfiable even though it's not a meaningful query; considered an
 * acceptable simplification given how contrived that combination is.
 */
std::optional<QueryDomain> fieldDomain(const std::string& name)
{
  if (name == "classname" || name == "properties")
  {
    return QueryDomain{QueryKind::Entity};
  }
  if (name == "name")
  {
    return QueryDomain{QueryKind::Entity, QueryKind::Layer, QueryKind::Group};
  }
  if (name == "entity" || name == "tags")
  {
    return QueryDomain{QueryKind::Entity, QueryKind::Brush, QueryKind::Patch};
  }
  if (name == "materials")
  {
    return QueryDomain{QueryKind::Brush, QueryKind::Patch};
  }
  if (name == "material" || name == "normal")
  {
    return QueryDomain{QueryKind::Face};
  }
  if (
    name == "type" || name == "bounds" || name == "center" || name == "layerName"
    || name == "groupName" || name == "visible" || name == "locked" || name == "selected"
    || name == "linked")
  {
    return AllKinds;
  }
  return std::nullopt; // unknown variable name -- carries no domain information
}

std::optional<QueryKind> queryKindForTypeName(const std::string& name)
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

std::optional<std::string> asVariableName(const el::ExpressionNode& node)
{
  return node.accept(kdl::overload(
    [](const el::VariableExpression& variable) -> std::optional<std::string> {
      return variable.variableName;
    },
    [](const auto&) -> std::optional<std::string> { return std::nullopt; }));
}

std::optional<std::string> asStringLiteral(const el::ExpressionNode& node)
{
  return node.accept(kdl::overload(
    [](const el::LiteralExpression& literal) -> std::optional<std::string> {
      if (literal.value.type() != el::ValueType::String)
      {
        return std::nullopt;
      }
      return el::withEvaluationContext(
               [&](auto& context) { return literal.value.stringValue(context); })
             | kdl::value();
    },
    [](const auto&) -> std::optional<std::string> { return std::nullopt; }));
}

/**
 * Detects the `type == "X"` / `"X" == type` shape and, if `X` is a recognized type name,
 * returns the singleton kind set it names as an explicit domain hint.
 */
std::optional<QueryDomain> typeHintDomain(
  const el::ExpressionNode& lhs, const el::ExpressionNode& rhs)
{
  const auto tryHint =
    [](
      const el::ExpressionNode& variableSide,
      const el::ExpressionNode& literalSide) -> std::optional<QueryDomain> {
    if (asVariableName(variableSide) != std::string{"type"})
    {
      return std::nullopt;
    }
    if (const auto literal = asStringLiteral(literalSide))
    {
      if (const auto kind = queryKindForTypeName(*literal))
      {
        return QueryDomain{*kind};
      }
    }
    return std::nullopt;
  };

  if (const auto hint = tryHint(lhs, rhs))
  {
    return hint;
  }
  return tryHint(rhs, lhs);
}

QueryDomain inferDomainRec(const el::ExpressionNode& expression);

QueryDomain intersectAll(const std::vector<el::ExpressionNode>& nodes)
{
  auto result = AllKinds;
  for (const auto& node : nodes)
  {
    result = intersect(result, inferDomainRec(node));
  }
  return result;
}

QueryDomain uniteAll(const std::vector<el::ExpressionNode>& nodes)
{
  auto result = QueryDomain{};
  for (const auto& node : nodes)
  {
    result = unite(result, inferDomainRec(node));
  }
  return result;
}

QueryDomain inferDomainRec(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [](const el::LiteralExpression&) { return AllKinds; },
    [](const el::VariableExpression& variable) {
      return fieldDomain(variable.variableName).value_or(AllKinds);
    },
    [](const el::ArrayExpression& array) { return intersectAll(array.elements); },
    [](const el::MapExpression& map) {
      auto result = AllKinds;
      for (const auto& [key, value] : map.elements)
      {
        result = intersect(result, inferDomainRec(value));
      }
      return result;
    },
    [](const el::UnaryExpression& unary) { return inferDomainRec(unary.operand); },
    [](const el::BinaryExpression& binary) -> QueryDomain {
      if (std::holds_alternative<el::binop::LogicalOr>(binary.operation))
      {
        return unite(
          inferDomainRec(binary.leftOperand), inferDomainRec(binary.rightOperand));
      }
      if (std::holds_alternative<el::binop::Equal>(binary.operation))
      {
        if (const auto hint = typeHintDomain(binary.leftOperand, binary.rightOperand))
        {
          return *hint;
        }
      }
      return intersect(
        inferDomainRec(binary.leftOperand), inferDomainRec(binary.rightOperand));
    },
    [](const el::SubscriptExpression& subscript) {
      return intersect(
        inferDomainRec(subscript.leftOperand), inferDomainRec(subscript.rightOperand));
    },
    [](const el::DotExpression& dot) { return inferDomainRec(dot.operand); },
    [](const el::CallExpression& call) { return intersectAll(call.arguments); },
    [](const el::SwitchExpression& switchExpression) {
      return uniteAll(switchExpression.cases);
    }));
}

} // namespace

QueryDomain inferQueryDomain(const el::ExpressionNode& expression)
{
  const auto domain = inferDomainRec(expression);
  return domain == AllKinds ? AllNodeKinds : domain;
}

} // namespace tb::mdl
