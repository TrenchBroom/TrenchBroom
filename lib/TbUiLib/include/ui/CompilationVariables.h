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

#include "el/VariableStore.h"

#include <string>

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{

namespace CompilationVariableNames
{
inline const std::string WORK_DIR_PATH = "WORK_DIR_PATH";
inline const std::string MAP_DIR_PATH = "MAP_DIR_PATH";
inline const std::string MAP_BASE_NAME = "MAP_BASE_NAME";
inline const std::string MAP_FULL_NAME = "MAP_FULL_NAME";
inline const std::string CPU_COUNT = "CPU_COUNT";
inline const std::string GAME_DIR_PATH = "GAME_DIR_PATH";
inline const std::string MODS = "MODS";
inline const std::string APP_DIR_PATH = "APP_DIR_PATH";
} // namespace CompilationVariableNames

class CommonVariables : public el::VariableTable
{
protected:
  explicit CommonVariables(const mdl::Map& map);
};

class CommonCompilationVariables : public CommonVariables
{
protected:
  explicit CommonCompilationVariables(const mdl::Map& map);
};

class CompilationWorkDirVariables : public CommonCompilationVariables
{
public:
  explicit CompilationWorkDirVariables(const mdl::Map& map);
};

class CompilationVariables : public CommonCompilationVariables
{
public:
  CompilationVariables(const mdl::Map& map, const std::string& workDir);
};

class LaunchGameEngineVariables : public CommonVariables
{
public:
  explicit LaunchGameEngineVariables(const mdl::Map& map);
};

} // namespace ui
} // namespace tb
