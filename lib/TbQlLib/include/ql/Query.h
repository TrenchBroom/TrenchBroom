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
