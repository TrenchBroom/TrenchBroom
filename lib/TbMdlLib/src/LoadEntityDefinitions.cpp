/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/LoadEntityDefinitions.h"

#include "fs/DiskIO.h"
#include "mdl/DefParser.h"
#include "mdl/EntParser.h"
#include "mdl/FgdParser.h"

#include "kd/path_utils.h"

namespace tb::mdl
{
namespace
{

template <typename Parser, typename... Args>
Result<std::vector<EntityDefinition>> loadEntityDefinitions(
  const std::filesystem::path& path, ParserStatus& status, Args&&... parserArgs)
{
  return fs::Disk::openFile(path) | kdl::and_then([&](auto file) {
           auto reader = file->reader().buffer();
           auto parser = Parser{reader.stringView(), std::forward<Args>(parserArgs)...};
           return parser.parseDefinitions(status);
         });
}

} // namespace

Result<std::vector<EntityDefinition>> loadEntityDefinitions(
  const std::filesystem::path& path, const Color& defaultColor, ParserStatus& status)
{
  const auto extension = kdl::path_to_lower(path.extension());
  if (extension == ".fgd")
  {
    return loadEntityDefinitions<FgdParser>(path, status, defaultColor, path);
  }
  if (extension == ".def")
  {
    return loadEntityDefinitions<DefParser>(path, status, defaultColor);
  }
  if (extension == ".ent")
  {
    return loadEntityDefinitions<EntParser>(path, status, defaultColor);
  }

  return Error{fmt::format("Unknown entity definition format: {}", path)};
}

} // namespace tb::mdl
