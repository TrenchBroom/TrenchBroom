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
#include "el/Expression.h"
#include "el/Types.h"

#include <string_view>

namespace tb::mdl
{

/**
 * Parses the given config file. If the file has the given legacy version, it was written
 * with legacy string escaping and is parsed with that. Otherwise, it is parsed with the
 * default string escaping.
 *
 * @param str the contents of the config file
 * @param legacyVersion the last config file version that uses legacy string escaping
 * @return the parsed expression or an error if the file could not be parsed
 */
Result<el::ExpressionNode> parseConfigExpression(
  std::string_view str, el::IntegerType legacyVersion);

} // namespace tb::mdl
