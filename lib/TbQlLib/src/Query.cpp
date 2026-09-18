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
#include "el/ParseExpression.h"
#include "el/ParseMode.h"
#include "el/VariableStore.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ql/BrushFaceVariableStore.h"
#include "ql/NodeVariableStore.h"
#include "ql/QueryDomainInference.h"

#include <type_traits>

namespace tb::ql
{

namespace
{

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

} // namespace

Result<el::ExpressionNode> parseQuery(const std::string_view queryText)
{
  return el::parseExpression(el::ParseMode::Strict, queryText);
}

QueryResult executeQuery(mdl::Map& map, const el::ExpressionNode& expression)
{
  const auto domain = inferQueryDomain(expression);
  if (domain.empty())
  {
    return std::vector<mdl::Node*>{};
  }

  if (domain == QueryDomain{QueryObjectType::Face})
  {
    return mdl::collectBrushFaces(
      std::vector<mdl::Node*>{&map.worldNode()}, [&](const auto& brushFaceHandle) {
        return matches(expression, makeBrushFaceVariableStore(map, brushFaceHandle));
      });
  }

  return mdl::collectNodesAndDescendants(
    std::vector<mdl::Node*>{&map.worldNode()}, [&](auto& node) {
      using N = std::decay_t<decltype(node)>;
      return domain.contains(queryObjectTypeOf<N>())
             && matches(expression, makeNodeVariableStore(map, node));
    });
}

} // namespace tb::ql
