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

#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb
{
class Logger;
} // namespace tb

namespace tb::mdl
{
struct MaterialConfig;
class Quake3Shader;
} // namespace tb::mdl

namespace tb::io
{
class FileSystem;

Result<std::vector<mdl::Quake3Shader>> loadShaders(
  const FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  kdl::task_manager& taskManager,
  Logger& logger);

} // namespace tb::io
