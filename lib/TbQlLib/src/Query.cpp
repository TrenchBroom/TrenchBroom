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

#include "ql/Query.h"

#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "el/ParseExpression.h"
#include "el/ParseMode.h"
#include "el/Value.h"
#include "el/VariableStore.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ql/BrushFaceVariableStore.h"
#include "ql/NodeVariableStore.h"
#include "ql/QueryDomainInference.h"

#include "kd/overload.h"
#include "kd/ranges/fold.h"
#include "kd/ranges/to.h"
#include "kd/string_format.h"
#include "kd/unpack.h"

#include <algorithm>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace tb::ql
{

namespace
{

// see parseQuery's doc comment for what this is used for
bool isFuzzySearchField(const std::string& name, const QueryField& field)
{
  // a fixed discriminator literal ("world"/"entity"/...)
  if (name == "type")
  {
    return false;
  }

  // a matching brush entity is selected via its brushes anyway, and the keys of this map
  // are field names
  if (name == "entity")
  {
    return false;
  }

  // a text search never finds brush faces
  if (field.domain == QueryDomain{QueryObjectType::Face})
  {
    return false;
  }

  // `like` never matches values of any other type
  return field.valueType == el::ValueType::String
         || field.valueType == el::ValueType::Array
         || field.valueType == el::ValueType::Map
         || field.valueType == el::ValueType::LazyMap;
}

std::vector<std::string> fuzzySearchFieldNames()
{
  return queryFields() | std::views::filter(kdl::unpack(isFuzzySearchField))
         | std::views::keys | kdl::ranges::to<std::vector>();
}

el::ExpressionNode makeFuzzyQuery(const std::string_view text)
{
  static const auto fields = fuzzySearchFieldNames();

  const auto makeClause = [&](const auto& field) {
    return el::ExpressionNode{el::BinaryExpression{
      el::binop::InfixCall{"like"},
      el::ExpressionNode{el::VariableExpression{field}},
      el::ExpressionNode{el::LiteralExpression{el::Value{std::string{text}}}},
    }};
  };

  const auto combineClauses = [](auto lhs, auto rhs) {
    return el::ExpressionNode{
      el::BinaryExpression{el::binop::LogicalOr{}, std::move(lhs), std::move(rhs)}};
  };

  return *kdl::ranges::fold_left_first(
    fields | std::views::transform(makeClause), combineClauses);
}

// a boolean field is a meaningful query on its own, unlike any other field
bool isBooleanFieldName(const std::string& name)
{
  const auto iField = queryFields().find(name);
  return iField != queryFields().end()
         && iField->second.valueType == el::ValueType::Boolean;
}

bool isFieldName(const std::string& name)
{
  return queryFields().contains(name);
}

// an unknown name most likely comes from text that happens to parse, e.g. `light is red`
bool usesOnlyKnownNames(const el::ExpressionNode& expression)
{
  const auto allUseOnlyKnownNames = [](const auto& expressions) {
    return std::ranges::all_of(expressions, usesOnlyKnownNames);
  };

  return expression.accept(kdl::overload(
    [](const el::LiteralExpression&) { return true; },
    [](const el::VariableExpression& v) { return isFieldName(v.variableName); },
    [&](const el::ArrayExpression& a) { return allUseOnlyKnownNames(a.elements); },
    [&](const el::MapExpression& m) {
      return allUseOnlyKnownNames(m.elements | std::views::values);
    },
    [](const el::UnaryExpression& u) { return usesOnlyKnownNames(u.operand); },
    // the parser only accepts built-in functions as infix calls
    [](const el::BinaryExpression& b) {
      return usesOnlyKnownNames(b.leftOperand) && usesOnlyKnownNames(b.rightOperand);
    },
    [](const el::SubscriptExpression& s) {
      return usesOnlyKnownNames(s.leftOperand) && usesOnlyKnownNames(s.rightOperand);
    },
    // the name after the dot is a key, not a field
    [](const el::DotExpression& d) { return usesOnlyKnownNames(d.operand); },
    [&](const el::CallExpression& c) {
      return el::builtinFunctionArity(c.name).has_value()
             && allUseOnlyKnownNames(c.arguments);
    },
    [&](const el::SwitchExpression& s) { return allUseOnlyKnownNames(s.cases); }));
}

bool isTypeVariable(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [](const el::VariableExpression& v) { return v.variableName == "type"; },
    [](const auto&) { return false; }));
}

bool isTypeComparison(const el::BinaryOperation& operation)
{
  return std::visit(
    kdl::overload(
      [](const el::binop::Equal&) { return true; },
      [](const el::binop::NotEqual&) { return true; },
      [](const el::binop::InfixCall& call) {
        return call.functionName == "is" || call.functionName == "in"
               || call.functionName == "contains";
      },
      [](const auto&) { return false; }),
    operation);
}

// a type name without quotes, such as `brush` in `type is brush`, or in an array, as in
// `type in [brush, patch]`, is a string
el::ExpressionNode quoteTypeName(const el::ExpressionNode& expression)
{
  return expression.accept(kdl::overload(
    [&](const el::VariableExpression& v) {
      return queryObjectTypeFromName(v.variableName)
               ? el::ExpressionNode{el::LiteralExpression{el::Value{v.variableName}}, expression.location()}
               : expression;
    },
    [&](const el::ArrayExpression& a) {
      return el::ExpressionNode{
        el::ArrayExpression{
          a.elements | std::views::transform(quoteTypeName)
          | kdl::ranges::to<std::vector>()},
        expression.location()};
    },
    [&](const auto&) { return expression; }));
}

// see parseQuery's doc comment for where type names don't need quotes
el::ExpressionNode quoteTypeNames(const el::ExpressionNode& expression)
{
  const auto withLocation = [&](auto quotedExpression) {
    return el::ExpressionNode{std::move(quotedExpression), expression.location()};
  };
  const auto quoteAll = [](const auto& expressions) {
    return expressions | std::views::transform(quoteTypeNames)
           | kdl::ranges::to<std::vector>();
  };

  return expression.accept(kdl::overload(
    [&](const el::LiteralExpression&) { return expression; },
    [&](const el::VariableExpression&) { return expression; },
    [&](const el::ArrayExpression& a) {
      return withLocation(el::ArrayExpression{quoteAll(a.elements)});
    },
    [&](const el::MapExpression& m) {
      return withLocation(el::MapExpression{
        m.elements
        | std::views::transform(kdl::unpack([](const auto& key, const auto& value) {
            return std::pair{key, quoteTypeNames(value)};
          }))
        | kdl::ranges::to<std::map<std::string, el::ExpressionNode>>()});
    },
    [&](const el::UnaryExpression& u) {
      return withLocation(el::UnaryExpression{u.operation, quoteTypeNames(u.operand)});
    },
    [&](const el::BinaryExpression& b) {
      const auto quoteOperand = [&](const auto& operand, const auto& otherOperand) {
        return isTypeComparison(b.operation) && isTypeVariable(otherOperand)
                 ? quoteTypeName(operand)
                 : quoteTypeNames(operand);
      };
      return withLocation(el::BinaryExpression{
        b.operation,
        quoteOperand(b.leftOperand, b.rightOperand),
        quoteOperand(b.rightOperand, b.leftOperand)});
    },
    [&](const el::SubscriptExpression& s) {
      return withLocation(el::SubscriptExpression{
        quoteTypeNames(s.leftOperand), quoteTypeNames(s.rightOperand)});
    },
    [&](const el::DotExpression& d) {
      return withLocation(el::DotExpression{quoteTypeNames(d.operand), d.fieldName});
    },
    [&](const el::CallExpression& c) {
      return withLocation(el::CallExpression{c.name, quoteAll(c.arguments)});
    },
    [&](const el::SwitchExpression& s) {
      return withLocation(el::SwitchExpression{quoteAll(s.cases)});
    }));
}

// see parseQuery's doc comment for which expressions are treated as fuzzy text
std::optional<std::string> fuzzySearchText(
  const el::ExpressionNode& expression, const std::string_view inputText)
{
  const auto searchTextUnlessQuery = [&]() -> std::optional<std::string> {
    return usesOnlyKnownNames(expression) ? std::nullopt
                                          : std::optional{std::string{inputText}};
  };

  return expression.accept(kdl::overload(
    [&](const el::LiteralExpression& literalExpression) -> std::optional<std::string> {
      return literalExpression.value.hasType(el::ValueType::String)
               ? literalExpression.value.stringValue()
               : kdl::str_trim(inputText);
    },
    [](const el::VariableExpression& variableExpression) -> std::optional<std::string> {
      return isBooleanFieldName(variableExpression.variableName)
               ? std::nullopt
               : std::optional{variableExpression.variableName};
    },
    [&](const el::UnaryExpression& unaryExpression) -> std::optional<std::string> {
      return unaryExpression.operand.isLiteral() ? std::optional{kdl::str_trim(inputText)}
                                                 : searchTextUnlessQuery();
    },
    [&](const auto&) { return searchTextUnlessQuery(); }));
}

el::ExpressionNode optimizeConstants(const el::ExpressionNode& expression)
{
  return el::withEvaluationContext(
           [&](auto& context) { return expression.optimize(context); })
    .value_or(expression);
}

bool matches(const el::ExpressionNode& expression, const el::VariableStore& store)
{
  return el::withEvaluationContext(
           [&](auto& context) {
             const auto value = expression.evaluate(context);
             return value.hasType(el::ValueType::Boolean) && value.booleanValue();
           },
           store)
    .value_or(false);
}

template <typename N>
constexpr QueryObjectType queryObjectTypeOf()
{
  if constexpr (std::is_same_v<N, mdl::WorldNode>)
  {
    return QueryObjectType::World;
  }
  else if constexpr (std::is_same_v<N, mdl::LayerNode>)
  {
    return QueryObjectType::Layer;
  }
  else if constexpr (std::is_same_v<N, mdl::GroupNode>)
  {
    return QueryObjectType::Group;
  }
  else if constexpr (std::is_same_v<N, mdl::EntityNode>)
  {
    return QueryObjectType::Entity;
  }
  else if constexpr (std::is_same_v<N, mdl::BrushNode>)
  {
    return QueryObjectType::Brush;
  }
  else if constexpr (std::is_same_v<N, mdl::PatchNode>)
  {
    return QueryObjectType::Patch;
  }
}

std::vector<mdl::Node*> selectableNodes(
  const std::vector<mdl::Node*>& nodes, const mdl::Map& map)
{
  return mdl::collectSelectableNodes(
    nodes | std::views::filter([&](const auto* node) { return node != &map.worldNode(); })
      | kdl::ranges::to<std::vector>(),
    map.editorContext());
}

std::vector<mdl::BrushFaceHandle> selectableBrushFaces(
  const std::vector<mdl::BrushFaceHandle>& handles,
  const mdl::EditorContext& editorContext)
{
  return handles | std::views::filter([&](const auto& handle) {
           return editorContext.selectable(handle.node(), handle.face());
         })
         | kdl::ranges::to<std::vector>();
}

} // namespace

el::ExpressionNode parseQuery(const std::string_view inputText)
{
  return el::parseExpression(el::ParseMode::Strict, inputText)
         | kdl::transform(quoteTypeNames) | kdl::transform([&](auto expression) {
             const auto searchText = fuzzySearchText(expression, inputText);
             return searchText | kdl::optional_transform(makeFuzzyQuery)
                    | kdl::optional_value_or(std::move(expression));
           })
         | kdl::value_or(makeFuzzyQuery(inputText));
}

QueryResult evaluateQuery(mdl::Map& map, const el::ExpressionNode& expression)
{
  const auto domain = inferQueryDomain(expression);
  if (domain.empty())
  {
    return std::vector<mdl::Node*>{};
  }

  const auto optimizedExpression = optimizeConstants(expression);
  if (domain == QueryDomain{QueryObjectType::Face})
  {
    return mdl::collectBrushFaces({&map.worldNode()}, [&](const auto& brushFaceHandle) {
      return matches(
        optimizedExpression, makeBrushFaceVariableStore(map, brushFaceHandle));
    });
  }

  return mdl::collectNodesAndDescendants({&map.worldNode()}, [&](const auto& node) {
    using N = std::decay_t<decltype(node)>;
    return domain.contains(queryObjectTypeOf<N>())
           && matches(optimizedExpression, makeNodeVariableStore(map, node));
  });
}

void executeQuery(mdl::Map& map, const el::ExpressionNode& expression)
{
  auto transaction = mdl::Transaction{map, "Search"};
  mdl::deselectAll(map);
  std::visit(
    kdl::overload(
      [&](const std::vector<mdl::Node*>& nodes) {
        mdl::selectNodes(map, selectableNodes(nodes, map));
      },
      [&](const std::vector<mdl::BrushFaceHandle>& handles) {
        mdl::selectBrushFaces(map, selectableBrushFaces(handles, map.editorContext()));
      }),
    evaluateQuery(map, expression));
  transaction.commit();
}

} // namespace tb::ql
