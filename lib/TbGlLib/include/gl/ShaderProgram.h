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

#include "Color.h"
#include "Macros.h"
#include "Result.h"
#include "gl/GlUtils.h"

#include "vm/mat.h"
#include "vm/vec.h"

#include <string>
#include <unordered_map>

namespace tb::gl
{
class Gl;
class ShaderManager;
class Shader;

class ShaderProgram
{
private:
  using UniformVariableCache = std::unordered_map<std::string, GLint>;
  using AttributeLocationCache = std::unordered_map<std::string, GLint>;

  std::string m_name;

  GLuint m_programId;

  mutable UniformVariableCache m_variableCache;
  mutable AttributeLocationCache m_attributeCache;

public:
  ShaderProgram(std::string name, GLuint programId);

  deleteCopy(ShaderProgram);

  ShaderProgram(ShaderProgram&& other) noexcept;
  ShaderProgram& operator=(ShaderProgram&& other) noexcept;

  void attach(Gl& gl, Shader& shader) const;
  Result<void> link(Gl& gl);

  void activate(Gl& gl, ShaderManager& shaderManager);
  void deactivate(Gl& gl, ShaderManager& shaderManager);

  void set(Gl& gl, const std::string& name, bool value);
  void set(Gl& gl, const std::string& name, int value);
  void set(Gl& gl, const std::string& name, size_t value);
  void set(Gl& gl, const std::string& name, float value);
  void set(Gl& gl, const std::string& name, const vm::vec2f& value);
  void set(Gl& gl, const std::string& name, const vm::vec3f& value);
  void set(Gl& gl, const std::string& name, const vm::vec4f& value);
  void set(Gl& gl, const std::string& name, const vm::mat2x2f& value);
  void set(Gl& gl, const std::string& name, const vm::mat3x3f& value);
  void set(Gl& gl, const std::string& name, const vm::mat4x4f& value);

  template <typename C>
  void set(Gl& gl, const std::string& name, const C& value)
    requires(AnyColor<C>)
  {
    set(gl, name, value.template to<RgbaF>().toVec());
  }

  GLint findAttributeLocation(Gl& gl, const std::string& name) const;

  void destroy(Gl& gl);

private:
  GLint findUniformLocation(Gl& gl, const std::string& name) const;
  bool checkActive(Gl& gl) const;
};

Result<ShaderProgram> createShaderProgram(Gl& gl, std::string name);

} // namespace tb::gl
