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

#include "gl/ShaderProgram.h"

#include "gl/GlInterface.h"
#include "gl/Shader.h"
#include "gl/ShaderManager.h"

#include "kd/contracts.h"
#include "kd/result.h"

#include <cassert>
#include <string>

namespace tb::gl
{

ShaderProgram::ShaderProgram(std::string name, const GLuint programId)
  : m_name{std::move(name)}
  , m_programId{programId}
{
  contract_pre(m_programId != 0);
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
  : m_name{std::move(other.m_name)}
  , m_programId{std::exchange(other.m_programId, 0)}
{
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
  m_name = std::move(other.m_name);
  m_programId = std::exchange(other.m_programId, 0);
  return *this;
}

void ShaderProgram::attach(Gl& gl, Shader& shader) const
{
  contract_pre(m_programId != 0);

  shader.attach(gl, m_programId);
}

namespace
{

std::string getInfoLog(Gl& gl, const GLuint programId)
{
  auto infoLogLength = GLint{};
  gl.getProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0)
  {
    auto infoLog = std::string{};
    infoLog.resize(size_t(infoLogLength));

    gl.getProgramInfoLog(programId, infoLogLength, &infoLogLength, infoLog.data());
    return infoLog;
  }

  return "Unknown error";
}
} // namespace

Result<void> ShaderProgram::link(Gl& gl)
{
  gl.linkProgram(m_programId);

  auto linkStatus = GLint(0);
  gl.getProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);

  if (linkStatus == 0)
  {
    return Error{
      "Could not link shader program '" + m_name + "': " + getInfoLog(gl, m_programId)};
  }

  return kdl::void_success;
}

void ShaderProgram::activate(Gl& gl, ShaderManager& shaderManager)
{
  contract_pre(m_programId != 0);

  gl.useProgram(m_programId);
  assert(checkActive(gl));

  shaderManager.setCurrentProgram(this);
}

void ShaderProgram::deactivate(Gl& gl, ShaderManager& shaderManager)
{
  gl.useProgram(0);

  shaderManager.setCurrentProgram(nullptr);
}

void ShaderProgram::set(Gl& gl, const std::string& name, const bool value)
{
  return set(gl, name, int(value));
}

void ShaderProgram::set(Gl& gl, const std::string& name, const int value)
{
  assert(checkActive(gl));
  gl.uniform1i(findUniformLocation(gl, name), value);
}

void ShaderProgram::set(Gl& gl, const std::string& name, const size_t value)
{
  assert(checkActive(gl));
  gl.uniform1i(findUniformLocation(gl, name), int(value));
}

void ShaderProgram::set(Gl& gl, const std::string& name, const float value)
{
  assert(checkActive(gl));
  gl.uniform1f(findUniformLocation(gl, name), value);
}

void ShaderProgram::set(Gl& gl, const std::string& name, const vm::vec2f& value)
{
  assert(checkActive(gl));
  gl.uniform2f(findUniformLocation(gl, name), value.x(), value.y());
}

void ShaderProgram::set(Gl& gl, const std::string& name, const vm::vec3f& value)
{
  assert(checkActive(gl));
  gl.uniform3f(findUniformLocation(gl, name), value.x(), value.y(), value.z());
}

void ShaderProgram::set(Gl& gl, const std::string& name, const vm::vec4f& value)
{
  assert(checkActive(gl));
  gl.uniform4f(findUniformLocation(gl, name), value.x(), value.y(), value.z(), value.w());
}

void ShaderProgram::set(Gl& gl, const std::string& name, const vm::mat2x2f& value)
{
  assert(checkActive(gl));
  gl.uniformMatrix2fv(
    findUniformLocation(gl, name), 1, false, reinterpret_cast<const float*>(value.v));
}

void ShaderProgram::set(Gl& gl, const std::string& name, const vm::mat3x3f& value)
{
  assert(checkActive(gl));
  gl.uniformMatrix3fv(
    findUniformLocation(gl, name), 1, false, reinterpret_cast<const float*>(value.v));
}

void ShaderProgram::set(Gl& gl, const std::string& name, const vm::mat4x4f& value)
{
  assert(checkActive(gl));
  gl.uniformMatrix4fv(
    findUniformLocation(gl, name), 1, false, reinterpret_cast<const float*>(value.v));
}

GLint ShaderProgram::findAttributeLocation(Gl& gl, const std::string& name) const
{
  auto it = m_attributeCache.find(name);
  if (it == std::end(m_attributeCache))
  {
    auto index = gl.getAttribLocation(m_programId, name.c_str());
    contract_assert(index != -1);

    auto inserted = false;
    std::tie(it, inserted) = m_attributeCache.emplace(name, index);
    contract_assert(inserted);
  }

  return it->second;
}

void ShaderProgram::destroy(Gl& gl)
{
  if (m_programId != 0)
  {
    gl.deleteProgram(m_programId);
    m_programId = 0;
  }
}

GLint ShaderProgram::findUniformLocation(Gl& gl, const std::string& name) const
{
  auto it = m_variableCache.find(name);
  if (it == std::end(m_variableCache))
  {
    auto index = gl.getUniformLocation(m_programId, name.c_str());
    contract_assert(index != -1);

    auto inserted = false;
    std::tie(it, inserted) = m_variableCache.emplace(name, index);
    contract_assert(inserted);
  }

  return it->second;
}

bool ShaderProgram::checkActive(Gl& gl) const
{
  auto currentProgramId = GLint(-1);
  gl.getIntegerv(GL_CURRENT_PROGRAM, &currentProgramId);
  return GLuint(currentProgramId) == m_programId;
}

Result<ShaderProgram> createShaderProgram(Gl& gl, std::string name)
{
  auto programId = gl.createProgram();

  if (programId == 0)
  {
    return Error{"Could not create shader '" + name + "'"};
  }

  return ShaderProgram{std::move(name), programId};
}

} // namespace tb::gl
