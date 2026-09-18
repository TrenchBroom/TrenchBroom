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
#include "el/Value.h"
#include "el/VariableStore.h"
#include "ql/BrushFaceLazyMap.h"
#include "ql/BrushNodeLazyMap.h"
#include "ql/EntityNodeLazyMap.h"
#include "ql/GroupNodeLazyMap.h"
#include "ql/LayerNodeLazyMap.h"
#include "ql/PatchNodeLazyMap.h"
#include "ql/WorldNodeLazyMap.h"

#include "kd/flat_map.h"
#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/string_utils.h"

#include <algorithm>
#include <array>
#include <optional>
#include <ostream>
#include <ranges>
#include <set>
#include <string>

namespace tb::ql
{
namespace
{

/*
 * Domain inference specializes a query for each object type in turn. Specializing a
 * query replaces every condition that can't apply to the object type, and every condition
 * that only depends on the type, by a constant, and then folds these constants into the
 * surrounding `!`, `&&` and `||`. The domain consists of the object types for which the
 * specialized query can still be true, that is, for which it hasn't been folded into a
 * constant other than `true`.
 */

/**
 * Returns every object type. Domain inference checks each of these types to decide
 * whether it belongs to the domain, and a condition that doesn't refer to any field
 * applies to all of them.
 */
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

/**
 * Returns the names of the variables that the given expression refers to, including names
 * that aren't fields of any object type.
 *
 * When a query is specialized for an object type, the names that a condition refers to
 * decide whether the condition applies to the type at all, and whether its value only
 * depends on the type.
 */
std::set<std::string> variableNames(const el::ExpressionNode& expression)
{
  const auto variableNamesIn = [](const auto& expressions) {
    auto result = std::set<std::string>{};
    for (const auto& e : expressions)
    {
      result.merge(variableNames(e));
    }
    return result;
  };

  return expression.accept(kdl::overload(
    [](const el::LiteralExpression&) { return std::set<std::string>{}; },
    [](const el::VariableExpression& v) { return std::set{v.variableName}; },
    [&](const el::ArrayExpression& a) { return variableNamesIn(a.elements); },
    [&](const el::MapExpression& m) {
      return variableNamesIn(m.elements | std::views::values);
    },
    [](const el::UnaryExpression& u) { return variableNames(u.operand); },
    [&](const el::BinaryExpression& b) {
      return variableNamesIn(std::array{b.leftOperand, b.rightOperand});
    },
    [&](const el::SubscriptExpression& s) {
      return variableNamesIn(std::array{s.leftOperand, s.rightOperand});
    },
    // the name after the dot is a key, not a variable
    [](const el::DotExpression& d) { return variableNames(d.operand); },
    [&](const el::CallExpression& c) { return variableNamesIn(c.arguments); },
    [&](const el::SwitchExpression& s) { return variableNamesIn(s.cases); }));
}

/**
 * Returns the object types that a condition referring to the given variables applies to.
 * These are the types that have any of the fields it refers to, or every object type if
 * it doesn't refer to any field. Names that aren't fields are ignored.
 *
 * When a query is specialized for an object type that a condition doesn't apply to, the
 * condition is replaced by a constant, since the fields that the type doesn't have
 * evaluate to Undefined and would otherwise make conditions like `classname !=
 * "func_detail"` match every object of that type.
 */
QueryDomain conditionDomain(const std::set<std::string>& variableNames)
{
  const auto domain =
    variableNames
    | std::views::transform([](const auto& name) { return queryFields().find(name); })
    | std::views::filter([](const auto iField) { return iField != queryFields().end(); })
    | std::views::transform(
      [](const auto iField) -> const QueryDomain& { return iField->second.domain; })
    | std::views::join | kdl::ranges::to<kdl::flat_set>();
  return domain.empty() ? fullDomain() : domain;
}

/**
 * Returns the value of the given expression if it is a Boolean literal.
 *
 * Specializing a query replaces conditions by Boolean literals. This function recognizes
 * them when folding them into the surrounding `!`, `&&` and `||`, and when deciding
 * whether a specialized query can still match.
 */
std::optional<bool> asBoolean(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [](const el::LiteralExpression& l) -> std::optional<bool> {
      return l.value.hasType(el::ValueType::Boolean)
               ? std::optional{l.value.booleanValue()}
               : std::nullopt;
    },
    [](const auto&) -> std::optional<bool> { return std::nullopt; }));
}

/**
 * Returns whether a query that has been specialized for an object type can still match
 * objects of that type. It can't if it has been folded into a constant other than `true`,
 * since a query only matches an object if it evaluates to `true`.
 *
 * The inferred domain consists of the object types for which this returns true.
 */
bool canMatch(const el::ExpressionNode& expression)
{
  return !expression.isLiteral() || asBoolean(expression) == true;
}

/**
 * Whether an expression is negated an even (Positive) or odd (Negative) number of times.
 *
 * Specializing a query keeps track of the polarity of each condition, since a condition
 * that doesn't apply to the object type must be replaced by `false` if it is positive,
 * but by `true` if it is negative, so that it can't make the query match either way.
 */
enum class Polarity
{
  Positive,
  Negative,
};

/**
 * Returns the opposite polarity. Specializing a query inverts the polarity when it
 * descends into the operand of `!`.
 */
Polarity invert(const Polarity polarity)
{
  return polarity == Polarity::Positive ? Polarity::Negative : Polarity::Positive;
}

/**
 * Specializes a single condition for the given object type. A single condition is any
 * expression other than `!`, `&&`, `||` and parentheses, such as a comparison.
 *
 * If the condition doesn't apply to the type, it is replaced by a constant that can't
 * make the query match, which depends on the condition's polarity. If its value only
 * depends on the type, it is replaced by that value. Otherwise, its value depends on the
 * individual object, so it is returned unchanged.
 *
 * Specializing a query only folds the constants returned here, so they decide which
 * object types end up in the domain.
 */
el::ExpressionNode specializeCondition(
  const el::ExpressionNode& condition,
  const QueryObjectType type,
  const Polarity polarity)
{
  const auto literal = [&](auto value) {
    return el::ExpressionNode{
      el::LiteralExpression{el::Value{std::move(value)}}, condition.location()};
  };

  const auto names = variableNames(condition);
  if (!conditionDomain(names).contains(type))
  {
    // must not make the query match, whether the condition is negated or not
    return literal(polarity == Polarity::Negative);
  }

  if (std::ranges::all_of(names, [](const auto& name) { return name == "type"; }))
  {
    // the same value for every object of the given type
    return el::withEvaluationContext(
             [&](auto& context) { return literal(condition.evaluate(context)); },
             el::VariableTable{{{"type", el::Value{kdl::str_to_string(type)}}}})
      .value_or(condition);
  }

  return condition;
}

/**
 * Specializes the given expression for the given object type, where `polarity` is the
 * expression's polarity within the query.
 *
 * It descends into the operands of `!`, `&&`, `||` and parentheses, specializes every
 * other expression as a single condition, and folds the resulting constants into the
 * surrounding operations. This implements specializeQuery.
 */
el::ExpressionNode specializeQueryRecursively(
  const el::ExpressionNode& expression,
  const QueryObjectType type,
  const Polarity polarity)
{
  const auto withLocation = [&](auto specializedExpression) {
    return el::ExpressionNode{std::move(specializedExpression), expression.location()};
  };

  // `false && x` is `false` and `true && x` is `x`, while `true || x` is `true` and
  // `false || x` is `x`
  const auto specializeLogicalOperation = [&](const auto& b, const bool absorbing) {
    auto lhs = specializeQueryRecursively(b.leftOperand, type, polarity);
    auto rhs = specializeQueryRecursively(b.rightOperand, type, polarity);
    const auto lhsValue = asBoolean(lhs);
    const auto rhsValue = asBoolean(rhs);
    if (lhsValue == absorbing || rhsValue == absorbing)
    {
      return withLocation(el::LiteralExpression{el::Value{absorbing}});
    }
    if (lhsValue)
    {
      return rhs;
    }
    if (rhsValue)
    {
      return lhs;
    }
    return withLocation(
      el::BinaryExpression{b.operation, std::move(lhs), std::move(rhs)});
  };

  return expression.accept(kdl::overload(
    [&](const el::UnaryExpression& u) {
      switch (u.operation)
      {
      case el::UnaryOperation::Group: {
        auto operand = specializeQueryRecursively(u.operand, type, polarity);
        return operand.isLiteral()
                 ? operand
                 : withLocation(el::UnaryExpression{u.operation, std::move(operand)});
      }
      case el::UnaryOperation::LogicalNegation: {
        auto operand = specializeQueryRecursively(u.operand, type, invert(polarity));
        const auto value = asBoolean(operand);
        return value ? withLocation(el::LiteralExpression{el::Value{!*value}})
                     : withLocation(el::UnaryExpression{u.operation, std::move(operand)});
      }
      default:
        return specializeCondition(expression, type, polarity);
      }
    },
    [&](const el::BinaryExpression& b) {
      return std::visit(
        kdl::overload(
          [&](const el::binop::LogicalAnd&) {
            return specializeLogicalOperation(b, false);
          },
          [&](const el::binop::LogicalOr&) {
            return specializeLogicalOperation(b, true);
          },
          [&](const auto&) { return specializeCondition(expression, type, polarity); }),
        b.operation);
    },
    [&](const auto&) { return specializeCondition(expression, type, polarity); }));
}

} // namespace

/**
 * Prints the name of an object type, which is also the value of the `type` field for
 * objects of that type. Specializing a query uses it to evaluate conditions whose value
 * only depends on the object type.
 */
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

/**
 * Returns the object type with the given name, which is the value of the `type` field for
 * objects of that type. This isn't part of domain inference itself; parsing a query uses
 * it to recognize type names that are written without quotes.
 */
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

/**
 * Returns every field that a query can refer to, together with the object types that have
 * it, collected from the fields that the lazy map of each object type provides.
 *
 * This is all that domain inference knows about the object types: a condition applies to
 * the object types that have the fields it refers to.
 */
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

/**
 * The entry point of domain inference. Specializes the query for every object type, keeps
 * the types for which it can still match, and then applies the rules for faces and
 * layers.
 */
QueryDomain inferQueryDomain(const el::ExpressionNode& expression)
{
  auto domain = fullDomain() | std::views::filter([&](const auto type) {
                  return canMatch(specializeQuery(expression, type));
                })
                | kdl::ranges::to<QueryDomain>();

  if (
    domain != QueryDomain{QueryObjectType::Face}
    && domain != QueryDomain{QueryObjectType::Layer})
  {
    // a query finds either faces or nodes, and a matching layer selects all objects in
    // it, so only search faces and layers on their own
    domain.erase(QueryObjectType::Face);
    domain.erase(QueryObjectType::Layer);
  }

  return domain;
}

/**
 * Specializes the query for the given object type, starting with positive polarity since
 * the query as a whole isn't negated. Domain inference uses the result to decide whether
 * the type belongs to the domain, and evaluating a query uses it to evaluate the query
 * against objects of that type.
 */
el::ExpressionNode specializeQuery(
  const el::ExpressionNode& expression, const QueryObjectType type)
{
  return specializeQueryRecursively(expression, type, Polarity::Positive);
}

} // namespace tb::ql
