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

#include "kd/reflection_decl.h"

#include <string>
#include <string_view>
#include <vector>

namespace tb::ql
{

/**
 * The words that can be inserted at the cursor while typing a query.
 */
struct QueryCompletion
{
  /**
   * The range of the text that an inserted word replaces, given as offsets into the text.
   * This is the name that contains the cursor or touches it, or an empty range at the
   * cursor if there is no such name.
   */
  size_t start = 0;
  size_t end = 0;

  /**
   * The words that can replace the range, sorted alphabetically, ignoring case.
   */
  std::vector<std::string> words;

  kdl_reflect_decl(QueryCompletion, start, end, words);
};

/**
 * Returns the words that can be inserted at `cursorPosition`, an offset into `inputText`,
 * to continue writing a query.
 *
 * A word is only offered where the text before the cursor, followed by that word, is
 * still the beginning of a valid query:
 *
 * - Where a term can begin, such as at the beginning of the text or after an operator,
 *   these are the names of all fields, the keywords such as `true`, and the functions
 *   that are usually called with prefix notation, such as `vec`.
 * - After a term, these are the functions that are usually called with infix notation,
 *   such as `is` and `like`.
 * - Where an object type name is compared with `type` and therefore doesn't need quotes,
 *   as in `type is brush`, these are only the object type names.
 * - After a dot, these are the keys of the value before the dot: `x`, `y` and `z` for a
 *   vector, `min` and `max` for a bounding box, the fields of the entity map that
 *   `entity` refers to, and the given `propertyKeys` for `properties`, such as the keys
 *   of the properties of the map's entities. The value must be a field, a key of another
 *   such value, or the result of calling a built-in function, also in parentheses, as in
 *   `bounds.min.`, `entity.properties.`, `vec(1, 2, 3).` or `(center).`, since the keys
 *   of any other value are unknown. Only the property keys that are names are offered,
 *   since no other key can follow a dot.
 *
 * Nothing is offered for the key of an entry in a map such as `{key: 1}`, within a string
 * or a comment, or if the text before the cursor isn't the beginning of a valid query.
 * Only the words that begin with the part of the name before the cursor are returned,
 * ignoring case. Apart from the rest of the name that the cursor is in, the text after
 * the cursor is ignored.
 */
QueryCompletion completeQuery(
  std::string_view inputText,
  size_t cursorPosition,
  const std::vector<std::string>& propertyKeys);

} // namespace tb::ql
