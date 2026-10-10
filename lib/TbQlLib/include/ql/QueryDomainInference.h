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

#include "kd/flat_set.h"

#include <iosfwd>
#include <optional>
#include <string>

namespace tb
{
namespace el
{
class ExpressionNode;
} // namespace el

namespace ql
{

/**
 * The types of objects that a query can be evaluated against.
 */
enum class QueryObjectType
{
  World,
  Layer,
  Group,
  Entity,
  Brush,
  Patch,
  Face,
};

std::ostream& operator<<(std::ostream& str, QueryObjectType type);

std::optional<QueryObjectType> queryObjectTypeFromName(const std::string& name);

/**
 * The set of object types a query should be evaluated against.
 */
using QueryDomain = kdl::flat_set<QueryObjectType>;

/**
 * Infers the set of object types that the given expression should be evaluated against.
 *
 * This is necessary because a field which the current object type doesn't have evaluates
 * to Undefined, so without narrowing the domain first, a negated query like `!(classname
 * == "func_detail")` would also match every object that doesn't have a classname field at
 * all.
 *
 * When nothing in the given expression narrows the domain, it defaults to every object
 * type except Face, since a query that asks for nothing in particular shouldn't start
 * matching individual faces too.
 *
 * An empty result means that the expression is unsatisfiable, so the caller should
 * short-circuit rather than iterate anything.
 */
QueryDomain inferQueryDomain(const el::ExpressionNode& expression);

} // namespace ql
} // namespace tb
