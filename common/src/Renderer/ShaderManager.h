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

#include <map>
#include <memory>
#include <string>

namespace TrenchBroom
{
namespace Renderer
{
class Shader;
class ShaderConfig;
class ShaderProgram;

class ShaderManager
{
private:
  friend class ShaderProgram;
  using ShaderCache = std::map<std::string, std::unique_ptr<Shader>>;
  using ShaderProgramCache =
    std::map<const ShaderConfig*, std::unique_ptr<ShaderProgram>>;

  ShaderCache m_shaders;
  ShaderProgramCache m_programs;
  ShaderProgram* m_currentProgram;

public:
  ShaderManager();
  ~ShaderManager();

public:
  ShaderProgram& program(const ShaderConfig& config);
  ShaderProgram* currentProgram();

private:
  void setCurrentProgram(ShaderProgram* program);
  std::unique_ptr<ShaderProgram> createProgram(const ShaderConfig& config);
  Shader& loadShader(const std::string& name, const GLenum type);
};
} // namespace Renderer
} // namespace TrenchBroom
