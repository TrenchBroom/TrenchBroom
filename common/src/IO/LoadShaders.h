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

namespace tb
{
class Logger;
} // namespace tb

namespace tb::Assets
{
class Quake3Shader;
} // namespace tb::Assets

namespace tb::Model
{
struct MaterialConfig;
}

namespace tb::IO
{
class FileSystem;

Result<std::vector<Assets::Quake3Shader>> loadShaders(
  const FileSystem& fs, const Model::MaterialConfig& materialConfig, Logger& logger);

} // namespace tb::IO
