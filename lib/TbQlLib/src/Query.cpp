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
#include "ql/BrushFaceLazyMap.h"
#include "ql/BrushFaceVariableStore.h"
#include "ql/BrushNodeLazyMap.h"
#include "ql/EntityNodeLazyMap.h"
#include "ql/GroupNodeLazyMap.h"
#include "ql/LayerNodeLazyMap.h"
#include "ql/NodeVariableStore.h"
#include "ql/PatchNodeLazyMap.h"
#include "ql/QueryDomainInference.h"
#include "ql/WorldNodeLazyMap.h"

#include "kd/overload.h"
#include "kd/ranges/fold.h"
#include "kd/ranges/to.h"
#include "kd/string_format.h"

#include <algorithm>
#include <optional>
#include <ranges>
#include <set>
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
std::vector<std::string> fuzzySearchFieldNames()
{
  auto names = std::set<std::string>{};
  const auto add = [&](const auto& fieldTypes) {
    std::ranges::copy(fieldTypes | std::views::keys, std::inserter(names, names.end()));
  };
  add(worldNodeFieldTypes());
  add(layerNodeFieldTypes());
  add(groupNodeFieldTypes());
  add(entityNodeFieldTypes());
  add(brushNodeFieldTypes());
  add(patchNodeFieldTypes());
  // brushFaceFieldTypes() is deliberately omitted
  names.erase("type"); // a fixed discriminator literal ("world"/"entity"/...)
  return names | kdl::ranges::to<std::vector>();
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

bool isBooleanField(
  const kdl::flat_map<std::string, el::ValueType>& fieldTypes, const std::string& name)
{
  const auto iType = fieldTypes.find(name);
  return iType != fieldTypes.end() && iType->second == el::ValueType::Boolean;
}

// a boolean field is a meaningful query on its own, unlike any other field
bool isBooleanFieldName(const std::string& name)
{
  return isBooleanField(worldNodeFieldTypes(), name)
         || isBooleanField(layerNodeFieldTypes(), name)
         || isBooleanField(groupNodeFieldTypes(), name)
         || isBooleanField(entityNodeFieldTypes(), name)
         || isBooleanField(brushNodeFieldTypes(), name)
         || isBooleanField(patchNodeFieldTypes(), name)
         || isBooleanField(brushFaceFieldTypes(), name);
}

bool isFieldName(const std::string& name)
{
  return worldNodeFieldTypes().contains(name) || layerNodeFieldTypes().contains(name)
         || groupNodeFieldTypes().contains(name) || entityNodeFieldTypes().contains(name)
         || brushNodeFieldTypes().contains(name) || patchNodeFieldTypes().contains(name)
         || brushFaceFieldTypes().contains(name);
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
      return el::isBuiltinFunction(c.name) && allUseOnlyKnownNames(c.arguments);
    },
    [&](const el::SwitchExpression& s) { return allUseOnlyKnownNames(s.cases); }));
}

// see parseQuery's doc comment for which expressions are treated as fuzzy text
std::optional<std::string> fuzzySearchText(
  const el::ExpressionNode& expression, const std::string_view inputText)
{
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
    [&](const auto&) -> std::optional<std::string> {
      return usesOnlyKnownNames(expression) ? std::nullopt
                                            : std::optional{std::string{inputText}};
    }));
}

bool matches(const el::ExpressionNode& expression, const el::VariableStore& store)
{
  return el::withEvaluationContext(
           [&](auto& context) { return expression.evaluate(context).booleanValue(); },
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
         | kdl::transform([&](auto expression) {
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

  if (domain == QueryDomain{QueryObjectType::Face})
  {
    return mdl::collectBrushFaces({&map.worldNode()}, [&](const auto& brushFaceHandle) {
      return matches(expression, makeBrushFaceVariableStore(map, brushFaceHandle));
    });
  }

  return mdl::collectNodesAndDescendants({&map.worldNode()}, [&](const auto& node) {
    using N = std::decay_t<decltype(node)>;
    return domain.contains(queryObjectTypeOf<N>())
           && matches(expression, makeNodeVariableStore(map, node));
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
