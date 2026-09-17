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
#include "ql/BrushNodeLazyMap.h"
#include "ql/EntityNodeLazyMap.h"
#include "ql/GroupNodeLazyMap.h"
#include "ql/LayerNodeLazyMap.h"
#include "ql/NodeVariableStore.h"
#include "ql/PatchNodeLazyMap.h"
#include "ql/QueryDomainInference.h"
#include "ql/WorldNodeLazyMap.h"

#include "kd/ranges/to.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <optional>
#include <ranges>
#include <regex>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace tb::ql
{

namespace
{

// see queryTextFrom's doc comment for the detection rule this implements
bool looksLikeQuery(const std::string_view text)
{
  static const auto structuralChars = std::string_view{R"(<>=!&|()[]"')"};
  if (text.find_first_of(structuralChars) != std::string_view::npos)
  {
    return true;
  }

  static const auto keywordPattern = std::regex{R"(\b(like|contains)\b)"};
  return std::regex_search(text.begin(), text.end(), keywordPattern);
}

// see queryTextFrom's doc comment for what this is used for
std::vector<std::string> fuzzySearchFieldNames()
{
  auto names = std::set<std::string>{};
  const auto add = [&](const auto& fieldNames) {
    names.insert(fieldNames.begin(), fieldNames.end());
  };
  add(worldNodeFieldNames());
  add(layerNodeFieldNames());
  add(groupNodeFieldNames());
  add(entityNodeFieldNames());
  add(brushNodeFieldNames());
  add(patchNodeFieldNames());
  // brushFaceFieldNames() is deliberately not unioned in -- material/normal can never
  // match through a fuzzy query anyway, since the domain of this many unioned fields
  // always resolves to every node kind but Face (see inferQueryDomain's "no narrowing
  // signal" default); including them here would just be misleading dead clauses.
  names.erase(
    "type"); // a fixed discriminator literal ("world"/"entity"/...), not free text
  return names | kdl::ranges::to<std::vector>();
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

} // namespace

std::optional<std::string> queryTextFrom(const std::string_view inputText)
{
  if (inputText.empty())
  {
    return std::nullopt;
  }

  if (looksLikeQuery(inputText))
  {
    return std::string{inputText};
  }

  static const auto fields = fuzzySearchFieldNames();
  return fmt::format(
    "{}",
    fmt::join(
      fields | std::views::transform([&](const auto& field) {
        return fmt::format(R"({} like "{}")", field, inputText);
      }),
      " || "));
}

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
