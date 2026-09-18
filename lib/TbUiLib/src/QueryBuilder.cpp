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

#include "ui/QueryBuilder.h"

#include "base/Macros.h"
#include "el/EvaluationContext.h"
#include "el/Expression.h"

#include "kd/overload.h"

#include <algorithm>
#include <sstream>
#include <string_view>
#include <utility>

namespace tb::ui
{

const std::vector<QueryBuilderField>& queryBuilderFields()
{
  using K = ql::QueryObjectType;
  static const auto fields = std::vector<QueryBuilderField>{
    {"classname",
     "Classname",
     "classname",
     QueryBuilderValueKind::String,
     ql::QueryDomain{K::World, K::Entity},
     false},
    {"name",
     "Name",
     "name",
     QueryBuilderValueKind::String,
     ql::QueryDomain{K::Layer, K::Group},
     false},
    {"entityClassname",
     "Entity Classname",
     "entity.classname",
     QueryBuilderValueKind::String,
     ql::QueryDomain{K::Brush, K::Patch, K::Face},
     false},
    {"property",
     "Property",
     "properties",
     QueryBuilderValueKind::String,
     ql::QueryDomain{K::World, K::Entity},
     true},
    {"entityProperty",
     "Entity Property",
     "entity.properties",
     QueryBuilderValueKind::String,
     ql::QueryDomain{K::Brush, K::Patch, K::Face},
     true},
    {"materials",
     "Materials",
     "materials",
     QueryBuilderValueKind::StringArray,
     ql::QueryDomain{K::Brush, K::Patch},
     false},
    {"tags",
     "Tags",
     "tags",
     QueryBuilderValueKind::StringArray,
     ql::QueryDomain{K::World, K::Entity, K::Brush, K::Patch, K::Face},
     false},
    {"layerName",
     "Layer Name",
     "layerName",
     QueryBuilderValueKind::String,
     ql::QueryDomain{K::Layer, K::Group, K::Entity, K::Brush, K::Patch, K::Face},
     false},
    {"groupName",
     "Group Name",
     "groupName",
     QueryBuilderValueKind::String,
     ql::QueryDomain{K::Layer, K::Group, K::Entity, K::Brush, K::Patch, K::Face},
     false},
    {"visible",
     "Visible",
     "visible",
     QueryBuilderValueKind::Boolean,
     ql::QueryDomain{
       K::World, K::Layer, K::Group, K::Entity, K::Brush, K::Patch, K::Face},
     false},
    {"locked",
     "Locked",
     "locked",
     QueryBuilderValueKind::Boolean,
     ql::QueryDomain{
       K::World, K::Layer, K::Group, K::Entity, K::Brush, K::Patch, K::Face},
     false},
    {"selected",
     "Selected",
     "selected",
     QueryBuilderValueKind::Boolean,
     ql::QueryDomain{
       K::World, K::Layer, K::Group, K::Entity, K::Brush, K::Patch, K::Face},
     false},
  };
  return fields;
}

const QueryBuilderField* findQueryBuilderField(const std::string& id)
{
  const auto& fields = queryBuilderFields();
  const auto it =
    std::find_if(fields.begin(), fields.end(), [&](const auto& f) { return f.id == id; });
  return it != fields.end() ? &*it : nullptr;
}

std::vector<QueryConditionOperator> operatorsFor(const QueryBuilderValueKind valueKind)
{
  switch (valueKind)
  {
  case QueryBuilderValueKind::String:
    return {
      QueryConditionOperator::Equals,
      QueryConditionOperator::NotEquals,
      QueryConditionOperator::Matches};
  case QueryBuilderValueKind::StringArray:
    return {QueryConditionOperator::Contains, QueryConditionOperator::Matches};
  case QueryBuilderValueKind::Boolean:
    return {QueryConditionOperator::Is};
    switchDefault();
  }
  return {};
}

namespace
{

el::ExpressionNode variableNode(const std::string& name)
{
  return el::ExpressionNode{el::VariableExpression{name}};
}

el::ExpressionNode literalNode(el::Value value)
{
  return el::ExpressionNode{el::LiteralExpression{std::move(value)}};
}

/**
 * The field-access node for `path` -- a bare variable for a dot-free path (e.g.
 * "classname"), or a dot-access chain for "entity.classname".
 */
el::ExpressionNode fieldPathNode(const std::string_view path)
{
  const auto dot = path.find('.');
  if (dot == std::string_view::npos)
  {
    return variableNode(std::string{path});
  }
  return el::ExpressionNode{el::DotExpression{
    fieldPathNode(path.substr(0, dot)), std::string{path.substr(dot + 1)}}};
}

/**
 * The field-access node for one condition -- the bracket subscript for a property
 * field (using `condition.propertyKey`), or the field's own path otherwise.
 */
el::ExpressionNode fieldOperandNode(
  const QueryBuilderField& field, const QueryCondition& condition)
{
  auto path = fieldPathNode(field.elPath);
  if (!field.isProperty)
  {
    return path;
  }
  return el::ExpressionNode{el::SubscriptExpression{
    std::move(path), literalNode(el::Value{condition.propertyKey})}};
}

/**
 * The condition node for one row, or nullopt if it isn't complete enough to run yet
 * (empty value, or an empty property key on a field that needs one).
 */
std::optional<el::ExpressionNode> conditionNode(const QueryCondition& condition)
{
  const auto* field = findQueryBuilderField(condition.fieldId);
  if (
    !field || condition.value.empty()
    || (field->isProperty && condition.propertyKey.empty()))
  {
    return std::nullopt;
  }

  auto operand = fieldOperandNode(*field, condition);
  switch (condition.op)
  {
  case QueryConditionOperator::Equals:
    return el::ExpressionNode{el::BinaryExpression{
      el::binop::Equal{}, std::move(operand), literalNode(el::Value{condition.value})}};
  case QueryConditionOperator::NotEquals:
    return el::ExpressionNode{el::BinaryExpression{
      el::binop::NotEqual{},
      std::move(operand),
      literalNode(el::Value{condition.value})}};
  case QueryConditionOperator::Contains:
    return el::ExpressionNode{el::BinaryExpression{
      el::binop::InfixCall{"contains"},
      std::move(operand),
      literalNode(el::Value{condition.value})}};
  case QueryConditionOperator::Matches:
    return el::ExpressionNode{el::BinaryExpression{
      el::binop::InfixCall{"like"},
      std::move(operand),
      literalNode(el::Value{condition.value})}};
  case QueryConditionOperator::Is:
    return el::ExpressionNode{el::BinaryExpression{
      el::binop::Equal{},
      std::move(operand),
      literalNode(el::Value{condition.value == "true"})}};
    switchDefault();
  }
  return std::nullopt;
}

el::ExpressionNode typeHintNode(const ql::QueryObjectType type)
{
  auto name = std::ostringstream{};
  name << type;
  return el::ExpressionNode{el::BinaryExpression{
    el::binop::Equal{}, variableNode("type"), literalNode(el::Value{name.str()})}};
}

/**
 * Left-folds `nodes` (must be non-empty) into a chain using `Op`, e.g.
 * `((n0 Op n1) Op n2) Op ...` -- matching how a real left-to-right parse of the same
 * chain associates, so the result reads (and, printed back out, re-parses) identically
 * without needing any extra grouping.
 */
template <typename Op>
el::ExpressionNode foldChain(std::vector<el::ExpressionNode> nodes)
{
  auto result = std::move(nodes.front());
  for (auto it = nodes.begin() + 1; it != nodes.end(); ++it)
  {
    result =
      el::ExpressionNode{el::BinaryExpression{Op{}, std::move(result), std::move(*it)}};
  }
  return result;
}

} // namespace

std::optional<el::ExpressionNode> buildQueryExpression(
  const std::vector<QueryCondition>& conditions,
  const QueryCombinator combinator,
  const std::optional<ql::QueryObjectType> hint)
{
  auto rowNodes = std::vector<el::ExpressionNode>{};
  for (const auto& condition : conditions)
  {
    if (auto node = conditionNode(condition))
    {
      rowNodes.push_back(std::move(*node));
    }
  }

  if (!hint)
  {
    if (rowNodes.empty())
    {
      return std::nullopt;
    }
    return combinator == QueryCombinator::All
             ? foldChain<el::binop::LogicalAnd>(std::move(rowNodes))
             : foldChain<el::binop::LogicalOr>(std::move(rowNodes));
  }

  if (rowNodes.empty())
  {
    return typeHintNode(*hint);
  }

  // A single row embeds safely under `&&` without grouping regardless of `combinator`
  // (every condition's own operator binds at least as tightly as `&&` does). More than
  // one Any-combined row needs an explicit group, since `&&` binds tighter than `||` --
  // without it, printing this tree back out as text and re-parsing it would silently
  // regroup it as `(hint && row0) || row1 ...` instead of `hint && (row0 || row1 ...)`.
  auto rows = rowNodes.size() == 1 ? std::move(rowNodes.front())
              : combinator == QueryCombinator::All
                ? foldChain<el::binop::LogicalAnd>(std::move(rowNodes))
                : el::ExpressionNode{el::UnaryExpression{
                    el::UnaryOperation::Group,
                    foldChain<el::binop::LogicalOr>(std::move(rowNodes))}};

  return el::ExpressionNode{
    el::BinaryExpression{el::binop::LogicalAnd{}, typeHintNode(*hint), std::move(rows)}};
}

namespace
{

// Returned by value, deliberately: ExpressionNode::accept()'s implementation deduces its
// internal visitor's return type via plain `auto`, which decays a reference return into a
// by-value copy before accept() itself hands it back out through its (reference) return
// type -- so a visitor here returning `const ExpressionNode&` would bind that reference
// to accept()'s already-dangling internal temporary. ExpressionNode wraps a shared_ptr,
// so returning by value is just a cheap refcount bump, not a real copy.
el::ExpressionNode unwrapGroup(const el::ExpressionNode& node)
{
  return node.accept(kdl::overload(
    [&](const el::UnaryExpression& u) -> el::ExpressionNode {
      return u.operation == el::UnaryOperation::Group ? unwrapGroup(u.operand) : node;
    },
    [&](const auto&) -> el::ExpressionNode { return node; }));
}

bool isVariableNamed(const el::ExpressionNode& node, const std::string& name)
{
  return node.accept(kdl::overload(
    [&](const el::VariableExpression& v) { return v.variableName == name; },
    [](const auto&) { return false; }));
}

std::optional<std::string> literalStringValue(const el::ExpressionNode& node)
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

std::optional<bool> literalBooleanValue(const el::ExpressionNode& node)
{
  return node.accept(kdl::overload(
    [](const el::LiteralExpression& lit) -> std::optional<bool> {
      if (lit.value.type() != el::ValueType::Boolean)
      {
        return std::nullopt;
      }
      return el::withEvaluationContext([&](auto&) { return lit.value.booleanValue(); })
        .value();
    },
    [](const auto&) -> std::optional<bool> { return std::nullopt; }));
}

std::optional<ql::QueryObjectType> matchTypeHint(const el::ExpressionNode& node)
{
  return unwrapGroup(node).accept(kdl::overload(
    [](const el::BinaryExpression& b) -> std::optional<ql::QueryObjectType> {
      if (!std::holds_alternative<el::binop::Equal>(b.operation))
      {
        return std::nullopt;
      }
      if (isVariableNamed(b.leftOperand, "type"))
      {
        if (const auto name = literalStringValue(b.rightOperand))
        {
          return ql::queryObjectTypeFromName(*name);
        }
      }
      if (isVariableNamed(b.rightOperand, "type"))
      {
        if (const auto name = literalStringValue(b.leftOperand))
        {
          return ql::queryObjectTypeFromName(*name);
        }
      }
      return std::nullopt;
    },
    [](const auto&) -> std::optional<ql::QueryObjectType> { return std::nullopt; }));
}

/**
 * Whether `node` is the field-access expression for `path` -- a bare variable for a
 * dot-free path (e.g. "classname"), or a DotExpression chain for "entity.classname".
 */
bool matchesFieldPath(const el::ExpressionNode& node, const std::string_view path)
{
  const auto dot = path.find('.');
  if (dot == std::string_view::npos)
  {
    return isVariableNamed(node, std::string{path});
  }

  const auto base = path.substr(0, dot);
  const auto rest = path.substr(dot + 1);
  return node.accept(kdl::overload(
    [&](const el::DotExpression& d) {
      return d.fieldName == rest && matchesFieldPath(d.operand, base);
    },
    [](const auto&) { return false; }));
}

struct MatchedFieldOperand
{
  std::string fieldId;
  std::string propertyKey;
};

std::optional<MatchedFieldOperand> matchFieldOperand(const el::ExpressionNode& node)
{
  for (const auto& field : queryBuilderFields())
  {
    if (!field.isProperty)
    {
      if (matchesFieldPath(node, field.elPath))
      {
        return MatchedFieldOperand{field.id, ""};
      }
      continue;
    }

    const auto propertyKey = node.accept(kdl::overload(
      [&](const el::SubscriptExpression& s) -> std::optional<std::string> {
        return matchesFieldPath(s.leftOperand, field.elPath)
                 ? literalStringValue(s.rightOperand)
                 : std::nullopt;
      },
      [](const auto&) -> std::optional<std::string> { return std::nullopt; }));
    if (propertyKey)
    {
      return MatchedFieldOperand{field.id, *propertyKey};
    }
  }
  return std::nullopt;
}

std::optional<QueryCondition> matchEqualityLeaf(
  const el::ExpressionNode& fieldSide,
  const el::ExpressionNode& valueSide,
  const bool isNotEqual)
{
  const auto field = matchFieldOperand(fieldSide);
  if (!field)
  {
    return std::nullopt;
  }
  const auto* catalogField = findQueryBuilderField(field->fieldId);

  if (catalogField->valueKind == QueryBuilderValueKind::Boolean)
  {
    if (isNotEqual)
    {
      return std::nullopt;
    }
    if (const auto b = literalBooleanValue(valueSide))
    {
      return QueryCondition{
        field->fieldId,
        field->propertyKey,
        QueryConditionOperator::Is,
        *b ? "true" : "false"};
    }
    return std::nullopt;
  }

  if (const auto value = literalStringValue(valueSide))
  {
    return QueryCondition{
      field->fieldId,
      field->propertyKey,
      isNotEqual ? QueryConditionOperator::NotEquals : QueryConditionOperator::Equals,
      *value};
  }
  return std::nullopt;
}

std::optional<QueryCondition> matchLeaf(const el::ExpressionNode& node)
{
  return unwrapGroup(node).accept(kdl::overload(
    [](const el::BinaryExpression& b) -> std::optional<QueryCondition> {
      if (std::holds_alternative<el::binop::Equal>(b.operation))
      {
        if (auto c = matchEqualityLeaf(b.leftOperand, b.rightOperand, false))
        {
          return c;
        }
        return matchEqualityLeaf(b.rightOperand, b.leftOperand, false);
      }
      if (std::holds_alternative<el::binop::NotEqual>(b.operation))
      {
        if (auto c = matchEqualityLeaf(b.leftOperand, b.rightOperand, true))
        {
          return c;
        }
        return matchEqualityLeaf(b.rightOperand, b.leftOperand, true);
      }
      if (const auto* infix = std::get_if<el::binop::InfixCall>(&b.operation))
      {
        if (infix->functionName != "like" && infix->functionName != "contains")
        {
          return std::nullopt;
        }
        const auto field = matchFieldOperand(b.leftOperand);
        const auto value = literalStringValue(b.rightOperand);
        if (!field || !value)
        {
          return std::nullopt;
        }
        const auto op = infix->functionName == "like" ? QueryConditionOperator::Matches
                                                      : QueryConditionOperator::Contains;
        return QueryCondition{field->fieldId, field->propertyKey, op, *value};
      }
      return std::nullopt;
    },
    [](const auto&) -> std::optional<QueryCondition> { return std::nullopt; }));
}

/**
 * Flattens a chain that's uniformly `&&` or uniformly `||` into its leaves. A node
 * whose top-level operator doesn't match `Op` (including a leaf like `classname == "x"`,
 * or a group wrapping the opposite operator) is a single leaf on its own -- matchLeaf
 * will reject it later if it also isn't a shape buildQueryExpression could have
 * produced.
 */
template <typename Op>
void flattenChain(
  const el::ExpressionNode& node, std::vector<const el::ExpressionNode*>& leaves)
{
  const auto& unwrapped = unwrapGroup(node);
  const auto isChain = unwrapped.accept(kdl::overload(
    [](const el::BinaryExpression& b) { return std::holds_alternative<Op>(b.operation); },
    [](const auto&) { return false; }));
  if (!isChain)
  {
    leaves.push_back(&node);
    return;
  }
  unwrapped.accept(kdl::overload(
    [&](const el::BinaryExpression& b) {
      flattenChain<Op>(b.leftOperand, leaves);
      flattenChain<Op>(b.rightOperand, leaves);
    },
    [](const auto&) {}));
}

/**
 * matchLeaf on every leaf in order; nullopt as soon as one fails, so an import is
 * all-or-nothing rather than partial.
 */
std::optional<std::vector<QueryCondition>> matchAllLeaves(
  const std::vector<const el::ExpressionNode*>& leaves)
{
  auto conditions = std::vector<QueryCondition>{};
  conditions.reserve(leaves.size());
  for (const auto* leaf : leaves)
  {
    auto condition = matchLeaf(*leaf);
    if (!condition)
    {
      return std::nullopt;
    }
    conditions.push_back(std::move(*condition));
  }
  return conditions;
}

/**
 * `expression` with no leading `type == "<kind>" && ` hint -- a flat chain that's
 * uniformly `&&` or uniformly `||` (or a single leaf, in which case it doesn't matter
 * which).
 */
std::optional<ImportedQuery> tryImportRowsOnly(const el::ExpressionNode& expression)
{
  auto andLeaves = std::vector<const el::ExpressionNode*>{};
  flattenChain<el::binop::LogicalAnd>(expression, andLeaves);

  auto orLeaves = std::vector<const el::ExpressionNode*>{};
  flattenChain<el::binop::LogicalOr>(expression, orLeaves);

  // exactly one of these actually represents `expression`'s own top-level shape,
  // unless `expression` is a single leaf, in which case both degenerate to it alone
  const auto useAnd = andLeaves.size() >= orLeaves.size();
  auto conditions = matchAllLeaves(useAnd ? andLeaves : orLeaves);
  if (!conditions)
  {
    return std::nullopt;
  }
  return ImportedQuery{
    std::move(*conditions),
    useAnd ? QueryCombinator::All : QueryCombinator::Any,
    std::nullopt};
}

} // namespace

std::optional<ImportedQuery> tryImportQuery(const el::ExpressionNode& expression)
{
  const auto unwrapped = unwrapGroup(expression);

  if (const auto hint = matchTypeHint(unwrapped))
  {
    return ImportedQuery{{}, QueryCombinator::All, hint};
  }

  // `type == "<kind>" && ...` is, structurally, just one more `&&` term --
  // buildQueryExpression never wraps it in a group of its own -- so the hint (if any)
  // is always the leftmost leaf of the *whole* top-level &&-chain, not necessarily the
  // outermost node's immediate left child (a chain of more than two `&&` terms
  // left-associates, nesting the earlier terms deeper on the left).
  auto andLeaves = std::vector<const el::ExpressionNode*>{};
  flattenChain<el::binop::LogicalAnd>(unwrapped, andLeaves);

  if (andLeaves.size() > 1)
  {
    if (const auto hint = matchTypeHint(*andLeaves.front()))
    {
      const auto remaining =
        std::vector<const el::ExpressionNode*>{andLeaves.begin() + 1, andLeaves.end()};

      if (remaining.size() > 1)
      {
        // hint && row && row && ... (combinator All): the rows are already flattened
        // AND-chain leaves alongside the hint, nothing more to do
        auto conditions = matchAllLeaves(remaining);
        return conditions ? std::optional{ImportedQuery{
                              std::move(*conditions), QueryCombinator::All, hint}}
                          : std::nullopt;
      }

      // exactly one remaining term: either "hint && oneRow" (combinator moot), or
      // "hint && (row || row || ...)" (combinator Any) -- try the single leaf first,
      // then whether it's itself an OR-chain buildQueryExpression would have grouped
      if (auto condition = matchLeaf(*remaining.front()))
      {
        return ImportedQuery{{std::move(*condition)}, QueryCombinator::All, hint};
      }

      auto orLeaves = std::vector<const el::ExpressionNode*>{};
      flattenChain<el::binop::LogicalOr>(*remaining.front(), orLeaves);
      if (orLeaves.size() < 2)
      {
        return std::nullopt;
      }
      auto conditions = matchAllLeaves(orLeaves);
      return conditions ? std::optional{ImportedQuery{
                            std::move(*conditions), QueryCombinator::Any, hint}}
                        : std::nullopt;
    }
  }

  return tryImportRowsOnly(unwrapped);
}

} // namespace tb::ui
