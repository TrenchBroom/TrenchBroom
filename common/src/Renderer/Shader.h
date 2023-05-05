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

#pragma once

#include "Renderer/GL.h"

#include <filesystem>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
class Shader
{
private:
  std::string m_name;
  GLenum m_type;
  GLuint m_shaderId;

public:
  Shader(const std::filesystem::path& path, const GLenum type);
  ~Shader();

  void attach(const GLuint programId);
  void detach(const GLuint programId);

private:
  static std::vector<std::string> loadSource(const std::filesystem::path& path);
};
} // namespace Renderer
} // namespace TrenchBroom
