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

#include "CompilationVariables.h"

#include "io/SystemPaths.h"
#include "mdl/Game.h" // IWYU pragma: keep
#include "mdl/GameFactory.h"
#include "mdl/Map.h"
#include "mdl/Map_World.h"
#include "ui/MapDocument.h"

#include "kdl/path_utils.h"
#include "kdl/vector_utils.h"

#include <string>
#include <thread>

namespace tb::ui
{

namespace CompilationVariableNames
{
const std::string WORK_DIR_PATH = "WORK_DIR_PATH";
const std::string MAP_DIR_PATH = "MAP_DIR_PATH";
const std::string MAP_BASE_NAME = "MAP_BASE_NAME";
const std::string MAP_FULL_NAME = "MAP_FULL_NAME";
const std::string CPU_COUNT = "CPU_COUNT";
const std::string GAME_DIR_PATH = "GAME_DIR_PATH";
const std::string MODS = "MODS";
const std::string APP_DIR_PATH = "APP_DIR_PATH";
} // namespace CompilationVariableNames

CommonVariables::CommonVariables(const mdl::Map& map)
{
  const auto filename = map.path().filename();
  const auto gamePath = map.game()->gamePath();

  auto mods = std::vector<std::string>{};
  mods.push_back(defaultMod(map));
  mods = kdl::vec_concat(std::move(mods), enabledMods(map));

  using namespace CompilationVariableNames;
  set(MAP_BASE_NAME, el::Value{kdl::path_remove_extension(filename).string()});
  set(GAME_DIR_PATH, el::Value{gamePath.string()});
  set(
    MODS,
    el::Value{kdl::vec_transform(mods, [](const auto& mod) { return el::Value{mod}; })});

  const auto& factory = mdl::GameFactory::instance();
  for (const auto& tool : map.game()->config().compilationTools)
  {
    const auto toolPath =
      factory.compilationToolPath(map.game()->config().name, tool.name);
    // e.g. variable name might be "qbsp", and the value is the path to the user's local
    // qbsp executable
    set(tool.name, el::Value{toolPath.string()});
  }
}

CommonCompilationVariables::CommonCompilationVariables(const mdl::Map& map)
  : CommonVariables{map}
{
  const auto filename = map.path().filename();
  const auto filePath = map.path().parent_path();
  const auto appPath = io::SystemPaths::appDirectory();

  using namespace CompilationVariableNames;
  set(MAP_FULL_NAME, el::Value{filename.string()});
  set(MAP_DIR_PATH, el::Value{filePath.string()});
  set(APP_DIR_PATH, el::Value{appPath.string()});
}

CompilationWorkDirVariables::CompilationWorkDirVariables(const mdl::Map& map)
  : CommonCompilationVariables{map}
{
}

CompilationVariables::CompilationVariables(
  const mdl::Map& map, const std::string& workDir)
  : CommonCompilationVariables{map}
{
  const auto cpuCount = size_t(std::max(std::thread::hardware_concurrency(), 1u));

  using namespace CompilationVariableNames;
  set(CPU_COUNT, el::Value{cpuCount});
  set(WORK_DIR_PATH, el::Value{workDir});
}

LaunchGameEngineVariables::LaunchGameEngineVariables(const mdl::Map& map)
  : CommonVariables{map}
{
}

} // namespace tb::ui
