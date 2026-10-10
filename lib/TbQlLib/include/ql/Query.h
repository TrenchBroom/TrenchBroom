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

#include "el/ExpressionNode.h"
#include "mdl/BrushFaceHandle.h"

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
 * Parses text entered by the user into a query expression.
 *
 * If `inputText` is a valid query, it is returned as parsed. Otherwise, it is treated as
 * text to search for, and the returned expression matches every node where any field
 * contains that text, for example `name like "foo" || classname like "foo" || ...`.
 * Parsing never fails: text that isn't a valid query, such as `func_*` or `red light`, is
 * searched for as it is.
 *
 * A query that consists of a single variable or literal is also treated as text to search
 * for, since on its own it wouldn't match by any field. `light` searches for "light"
 * instead of looking up a variable of that name, and `"red light"` searches for the
 * quoted text. The boolean fields `visible` and `locked` are the exception: each is a
 * valid query on its own, so `visible` matches every visible object. To search for such a
 * word as text, quote it.
 *
 * Likewise, a query that refers to a name that isn't a field of any object type, or that
 * calls a function that isn't built in, is treated as text to search for, since such a
 * query most likely comes from text that happens to be valid syntax, such as `light is
 * red`. This also applies to a misspelled field name, so `clasname is "light"` searches
 * for that text.
 *
 * Text searches never match brush faces. To search faces, write an explicit query such as
 * `material like "..."`. Since entity properties are among the searched fields, a text
 * search also matches entities by any of their property keys or values.
 */
el::ExpressionNode parseQuery(std::string_view inputText);

using QueryResult =
  std::variant<std::vector<mdl::Node*>, std::vector<mdl::BrushFaceHandle>>;

/**
 * Evaluates `expression` against every node (or, for a face-domain query, every brush
 * face) in `map`.
 */
QueryResult evaluateQuery(mdl::Map& map, const el::ExpressionNode& expression);

/**
 * Replaces the current selection in `map` with the nodes or brush faces that
 * `expression` matches.
 *
 * Matches that can't be selected, such as hidden or locked objects or objects in closed
 * groups, are left out. Brush entities and layers are selected via the objects they
 * contain, but the world is not, since it matches by its own classname and properties.
 */
void executeQuery(mdl::Map& map, const el::ExpressionNode& expression);

} // namespace ql
} // namespace tb
