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

#include "kd/flat_map.h"
#include "kd/flat_set.h"

#include <iosfwd>
#include <optional>
#include <string>

namespace tb
{
namespace el
{
class ExpressionNode;
enum class ValueType;
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
 * A field that a query can refer to.
 */
struct QueryField
{
  /**
   * The object types that have the field.
   */
  QueryDomain domain;

  /**
   * The type of the field's values, which is the same for every object type.
   */
  el::ValueType valueType;
};

/**
 * Returns every field that a query can refer to, by name.
 */
const kdl::flat_map<std::string, QueryField>& queryFields();

/**
 * Infers the set of object types that the given expression should be evaluated against.
 *
 * These are the object types for which the expression, specialized for the type by
 * `specializeQuery`, can still be true. For example, `type == "layer" && name like "a*"`
 * can only be true for layers, and `!(classname == "func_detail")` only for the world and
 * entities. Since this depends on what the expression evaluates to rather than on how it
 * is written, logically equivalent expressions such as `!(a && b)` and `!a || !b` have
 * the same domain.
 *
 * Faces are only included if nothing else is, since a query finds either faces or nodes,
 * but never both. So a query that can be true for every object type only searches nodes,
 * and `type in ["brush", "face"]` only searches brushes.
 *
 * Likewise, layers are only included if nothing else is, since a matching layer selects
 * all objects in it. Otherwise, a query like `bounds intersects bbox(...)` would select
 * every object in a layer whose bounds intersect the box.
 *
 * An empty result means that the query can't find anything, so the caller should
 * short-circuit rather than iterate anything.
 */
QueryDomain inferQueryDomain(const el::ExpressionNode& expression);

/**
 * Specializes the given expression for objects of the given type, that is, simplifies it
 * using what is known about every object of that type: which fields it has, and the value
 * of its `type` field.
 *
 * The operands of `&&`, `||` and `!` are conditions themselves. Any other expression,
 * such as a comparison or a switch, is a single condition, which is replaced by a
 * constant in two cases:
 *
 * - If it only refers to fields that the type doesn't have, it is replaced by `false`, or
 *   by `true` if it is negated, so that it can't make the expression match. Otherwise,
 *   `!(classname is "func_detail")` would match every brush, since `classname` is
 *   Undefined for brushes and therefore never equal to "func_detail".
 * - If it doesn't refer to any field other than `type`, it is replaced by its value for
 *   the type, so `type in ["brush", "patch"]` is replaced by `true` for brushes.
 *
 * Constants are then folded, so for brushes, `!(classname is "func_detail") || materials
 * like "*sky*"` becomes `materials like "*sky*"`, and `type == "brush" && visible`
 * becomes `visible`.
 */
el::ExpressionNode specializeQuery(
  const el::ExpressionNode& expression, QueryObjectType type);

} // namespace ql
} // namespace tb
