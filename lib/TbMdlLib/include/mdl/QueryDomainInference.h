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

#include <set>

namespace tb::el
{
class ExpressionNode;
}

namespace tb::mdl
{

/**
 * The node kinds (plus the face pseudo-kind) that a search/filter query expression can
 * target, as inferred by `inferQueryDomain`.
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

using QueryDomain = std::set<QueryKind>;

/**
 * Infers which node kinds (or the face domain) a parsed search/filter query expression
 * can possibly match, by walking the expression and looking at which schema fields (see
 * `NodeVariableStore`/`FaceVariableStore`) it references.
 *
 * This is more than a performance optimization: it is what makes negation behave
 * correctly. For example, `!(classname == "x")` must only ever be evaluated against
 * entities, since `classname` is `Undefined` -- and thus not-equal -- on every other node
 * kind, which would otherwise make the negation spuriously match everything else in the
 * map.
 *
 * Returns:
 *  - an empty set if the query is provably unsatisfiable (it references two fields that
 *    are never both bound on the same kind),
 *  - exactly `{QueryKind::Face}` if the query should be evaluated in face domain,
 *  - otherwise a non-empty subset of the six node kinds to iterate. If the expression
 *    gives no domain-narrowing signal at all, this is all six node kinds.
 */
QueryDomain inferQueryDomain(const el::ExpressionNode& expression);

} // namespace tb::mdl
