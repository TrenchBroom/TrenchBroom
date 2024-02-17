/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "GameConfig.h"

#include "Ensure.h"
#include "IO/DiskFileSystem.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"
#include "kdl/string_utils.h"

#include "vm/bbox_io.h"
#include "vm/vec_io.h"

#include <cassert>
#include <ostream>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{

kdl_reflect_impl(MapFormatConfig);

kdl_reflect_impl(PackageFormatConfig);

kdl_reflect_impl(FileSystemConfig);

kdl_reflect_impl(TextureConfig);

kdl_reflect_impl(EntityConfig);

kdl_reflect_impl(FlagConfig);

kdl_reflect_impl(FlagsConfig);

int FlagsConfig::flagValue(const std::string& flagName) const
{
  for (size_t i = 0; i < flags.size(); ++i)
  {
    if (flags[i].name == flagName)
    {
      return flags[i].value;
    }
  }
  return 0;
}

std::string FlagsConfig::flagName(const size_t index) const
{
  ensure(index < flags.size(), "index out of range");
  return flags[index].name;
}

std::vector<std::string> FlagsConfig::flagNames(const int mask) const
{
  if (mask == 0)
  {
    return {};
  }

  std::vector<std::string> names;
  for (size_t i = 0; i < flags.size(); ++i)
  {
    if (mask & (1 << i))
    {
      names.push_back(flags[i].name);
    }
  }
  return names;
}

kdl_reflect_impl(FaceAttribsConfig);

kdl_reflect_impl(CompilationTool);

kdl_reflect_impl(GameConfig);

std::filesystem::path GameConfig::findInitialMap(const std::string& formatName) const
{
  for (const auto& format : fileFormats)
  {
    if (format.format == formatName)
    {
      if (!format.initialMap.empty())
      {
        return findConfigFile(format.initialMap);
      }
      else
      {
        break;
      }
    }
  }
  return std::filesystem::path{};
}

std::filesystem::path GameConfig::findConfigFile(
  const std::filesystem::path& filePath) const
{
  return path.parent_path() / filePath;
}
} // namespace Model
} // namespace TrenchBroom
