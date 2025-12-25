/*
 Copyright (C) 2010 Kristian Duske

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

#include "Result.h"
#include "mdl/MapFormat.h"

#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace tb::mdl
{

/**
 * Scans the input stream to find game type and map format comments and
 * returns the name of the game and the map format.
 *
 * If no game comment is found or the game is unknown, an empty optional is returned as
 * the game name. If no map format comment is found or the format is unknown,
 * MapFormat::Unknown is returned as the map format.
 *
 * An error is returned if the stream is in a bad state.
 */
Result<std::pair<std::optional<std::string>, MapFormat>> readMapHeader(
  std::istream& stream);

/**
 * Writes comments into the given stream that can be used to identify the game and map
 * format via readMapHeader.
 */
void writeMapHeader(std::ostream& stream, std::string_view gameName, MapFormat mapFormat);

} // namespace tb::mdl
