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

#pragma once

#include "base/Result.h"
#include "el/ExpressionNode.h"
#include "mdl/BrushFaceHandle.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace tb
{
namespace mdl
{
class Map;
class Node;
} // namespace mdl

namespace ql
{

/**
 * Interprets `inputText` as the search/filter query language's two input tiers: text
 * that looks like an intentional query (containing an EL-structural token -- one of
 * `< > = ! & | ( ) [ ] " '` -- or the standalone word `like`/`contains`) is passed
 * through unchanged; anything else is fuzzy text, sugar for an OR-chain of `field like
 * "<inputText>"` over every field any node kind's schema exposes (`like`'s own
 * semantics already turn a wildcard-free pattern into a substring match, and safely
 * no-op on a field whose value isn't String/Array/Map-shaped, so no per-field type
 * filtering is needed). This keeps a bare word from ever being misread as an EL
 * variable lookup, which would silently evaluate to Undefined/false for everything
 * rather than matching by field.
 *
 * Two consequences worth knowing: brush faces are never reachable this way (the
 * unioned domain always resolves to every node kind but Face -- write an explicit
 * `material like "..."` query to search faces), and `properties` is one of the unioned
 * fields, so an entity is matched by any of its property keys or values too, not just
 * `classname`.
 *
 * Returns the resulting EL source text, ready to hand to parseQuery, or nullopt if
 * `inputText` is empty -- there is no query to run.
 */
std::optional<std::string> queryTextFrom(std::string_view inputText);

/**
 * Parses `queryText` as an EL expression for the search/filter query language. Returns
 * an Error if the text isn't a syntactically valid EL expression.
 */
Result<el::ExpressionNode> parseQuery(std::string_view queryText);

using QueryResult =
  std::variant<std::vector<mdl::Node*>, std::vector<mdl::BrushFaceHandle>>;

/**
 * Evaluates `expression` against every node (or, for a face-domain query, every brush
 * face) in `map`, using QueryDomainInference to narrow which kinds are iterated at all.
 */
QueryResult executeQuery(mdl::Map& map, const el::ExpressionNode& expression);

} // namespace ql
} // namespace tb
