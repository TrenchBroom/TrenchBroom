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

namespace tb::el
{
class ExpressionNode;
}

namespace tb::ql
{

/**
 * The kinds of thing the search/filter query language's object schema can be scoped to
 * -- the six mdl::Node kinds, plus Face for individual brush faces.
 */
enum class QueryKind
{
  World,
  Layer,
  Group,
  Entity,
  Brush,
  Patch,
  Face,
};

std::ostream& operator<<(std::ostream& str, QueryKind kind);

/**
 * A query's inferred target: the set of kinds it should be evaluated against.
 */
using QueryDomain = kdl::flat_set<QueryKind>;

/**
 * Infers which kinds `expression` should be evaluated against, by walking which schema
 * fields it references. This is more than an optimization: without it, a negated query
 * like `!(classname == "func_detail")` would match every other node kind too (since
 * `classname` is Undefined, and hence never equal to "func_detail", there) instead of
 * meaning "worldspawn/entities that aren't func_detail". Narrowing the iteration domain
 * to `{world, entity}` first, before evaluating, is what makes the negation mean what it
 * looks like it means.
 *
 * A field exclusive to one kind (or kind set) narrows the domain to that set; `&&`
 * intersects, `||` broadens (unions); `!` passes its operand's domain through unchanged
 * -- negation doesn't change *what* is being tested, only whether the result is inverted,
 * so `!(classname == "x")` still only makes sense evaluated against world/entities. An
 * explicit `type == "kind"` comparison is a hint with domain `{kind}`.
 *
 * If nothing in the expression narrows the domain, it defaults to every node kind but
 * never Face -- e.g. `visible == false` searches worldspawn and every layer, group,
 * entity, brush and patch, but not individual faces, since a query that says nothing
 * about what it wants should search the map's objects, not silently start matching
 * faces too. Face only
 * appears in the result when something in the query actually asks for it: a field the
 * face domain has (whether or not it's face-exclusive -- `entity` and `tags` exist on
 * both node and face schemas, so referencing either broadens into Face too), or an
 * explicit `type == "face"`.
 *
 * An empty result means the query is provably unsatisfiable -- the caller should
 * short-circuit rather than iterate anything.
 */
QueryDomain inferQueryDomain(const el::ExpressionNode& expression);

} // namespace tb::ql
