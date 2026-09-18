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
#include "ql/QueryDomainInference.h"

#include <optional>
#include <string>
#include <vector>

namespace tb::ui
{

/**
 * The shape of value a QueryBuilderField's condition rows edit, and hence which
 * QueryConditionOperators apply to it (see operatorsFor).
 */
enum class QueryBuilderValueKind
{
  String,
  StringArray,
  Boolean,
};

/**
 * One entry in the query builder's field catalog -- a UI-facing field, its EL access
 * path, and which ql::QueryKinds it's meaningful on.
 *
 * `classname`/`properties` and `entityClassname`/`entityProperty` are deliberately two
 * pairs of fields, not one ambiguous field with a mode switch: `classname`/`properties`
 * are bound directly on WorldNodeBinding/EntityNodeBinding (a world/entity node's own
 * data), while BrushNodeBinding/PatchNodeBinding/BrushFaceBinding only expose their
 * owning entity's data through a separate `entity` field -- there is no single EL path
 * that means "classname, whichever kind of node this is". The builder's "Object type"
 * control (QueryBuilderEditor) resolves the ambiguity for the user by filtering the
 * Field choices down to whichever pair actually applies.
 */
struct QueryBuilderField
{
  std::string id;
  std::string label;
  std::string elPath;
  QueryBuilderValueKind valueKind;
  ql::QueryDomain compatibleKinds;
  bool isProperty = false;
};

/**
 * The v1 query builder's field catalog, in display order. Deliberately excludes the
 * spatial fields (`bounds`/`center`/`normal`) and the `vec()`/`bbox()`/`distanceTo()`/
 * `intersects()` functions -- a later iteration once this ships.
 */
const std::vector<QueryBuilderField>& queryBuilderFields();

/**
 * `nullptr` if `id` isn't one of queryBuilderFields()'s entries.
 */
const QueryBuilderField* findQueryBuilderField(const std::string& id);

enum class QueryConditionOperator
{
  Equals,
  NotEquals,
  Contains,
  Matches,
  Is,
};

/**
 * Which QueryConditionOperators are meaningful for a field of the given value kind, in
 * display order -- the row editor's Operator choices are exactly this list.
 */
std::vector<QueryConditionOperator> operatorsFor(QueryBuilderValueKind valueKind);

/**
 * One condition row. `propertyKey` is only meaningful when the referenced field's
 * `isProperty` is true; `value` is the free-typed right-hand side ("true"/"false" for a
 * Boolean field's single `Is` operator).
 */
struct QueryCondition
{
  std::string fieldId;
  std::string propertyKey;
  QueryConditionOperator op = QueryConditionOperator::Equals;
  std::string value;
};

enum class QueryCombinator
{
  All,
  Any,
};

/**
 * Builds the query expression for `conditions`, joined by `&&` (All) or `||` (Any) --
 * constructed directly as an el::ExpressionNode tree (the same node shapes parsing that
 * text would produce), not by generating and re-parsing text: the builder already has
 * the structured data, so there's nothing to gain by round-tripping it through EL's
 * parser, and every node this builds is handed straight to ql::executeQuery. Callers
 * that want text for display (e.g. to put in the search box) call the result's
 * asString() -- EL's existing pretty-printer, already exercised by dot-access/infix-call
 * round-trip tests, rather than a second hand-rolled serializer here.
 *
 * Conditions with an empty value are skipped entirely, as if not yet specified. If
 * `hint` is set, `type == "<kind>"` is ANDed onto the result regardless of
 * `combinator` -- it narrows what's being tested, it isn't an alternative condition to
 * combine with the others. Returns nullopt if there's nothing to run (no hint, no
 * complete conditions) -- mirroring ql::queryTextFrom's own "nullopt means no query to
 * run".
 */
std::optional<el::ExpressionNode> buildQueryExpression(
  const std::vector<QueryCondition>& conditions,
  QueryCombinator combinator,
  std::optional<ql::QueryKind> hint);

struct ImportedQuery
{
  std::vector<QueryCondition> conditions;
  QueryCombinator combinator = QueryCombinator::All;
  std::optional<ql::QueryKind> hint;
};

/**
 * The reverse of buildQueryExpression, best-effort: `nullopt` if `expression` isn't
 * shaped like something buildQueryExpression could have produced -- an optional leading
 * `type == "<kind>" && `, followed by a flat chain of recognized field/operator/value
 * conditions joined uniformly by either `&&` or `||` (never both). Any other shape
 * (a negation, a nested group, an unrecognized field or function, a mix of `&&`/`||`)
 * aborts the whole import rather than reconstructing a partial, potentially misleading
 * set of rows.
 *
 * Unlike buildQueryExpression, this direction genuinely needs to work on a parsed
 * expression rather than something already structured: the search box is a plain text
 * field, so whatever it currently holds -- whether the builder wrote it or the user
 * typed it by hand -- is only available as text, and has to be parsed before its shape
 * can be recognized at all.
 */
std::optional<ImportedQuery> tryImportQuery(const el::ExpressionNode& expression);

} // namespace tb::ui
