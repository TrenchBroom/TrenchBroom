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

#include "mdl/MapHeader.h"

#include "kd/string_compare.h"

#include <iostream>

namespace tb::mdl
{
namespace
{

constexpr auto GameHeader = "Game";
constexpr auto FormatHeader = "Format";

Result<std::optional<std::string>> readInfoComment(
  std::istream& stream, const std::string& name)
{
  const auto expectedHeader = "// " + name + ": ";

  auto lineBuffer = std::string{};
  std::getline(stream, lineBuffer);
  if (stream.fail())
  {
    return Error{"Failed to read info comment from stream"};
  }

  const auto lineView = std::string_view{lineBuffer};
  if (!kdl::cs::str_is_prefix(lineView, expectedHeader))
  {
    return std::nullopt;
  }

  auto result = lineView.substr(expectedHeader.size());
  if (!result.empty() && result.back() == '\r')
  {
    result = result.substr(0, result.length() - 1);
  }
  return std::string{result};
}

} // namespace

Result<std::pair<std::optional<std::string>, MapFormat>> readMapHeader(
  std::istream& stream)
{
  return readInfoComment(stream, GameHeader).join(readInfoComment(stream, FormatHeader))
         | kdl::transform([](auto gameName, auto formatName) {
             const auto format =
               formatName ? formatFromName(*formatName) : MapFormat::Unknown;
             return std::pair{std::move(gameName), format};
           });
}

void writeMapHeader(
  std::ostream& stream, const std::string_view gameName, const MapFormat mapFormat)
{
  stream << "// " << GameHeader << ": " << gameName << "\n"
         << "// " << FormatHeader << ": " << formatName(mapFormat) << "\n";
}

} // namespace tb::mdl
