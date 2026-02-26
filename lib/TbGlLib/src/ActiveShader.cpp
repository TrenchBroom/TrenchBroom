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

#include "gl/ActiveShader.h"

#include "gl/ShaderManager.h"

namespace tb::gl
{

ActiveShader::ActiveShader(
  Gl& gl, ShaderManager& shaderManager, const ShaderConfig& shaderConfig)
  : m_gl{gl}
  , m_shaderManager{shaderManager}
  , m_program{shaderManager.program(shaderConfig)}
{
  m_program.activate(m_gl, m_shaderManager);
}

ActiveShader::~ActiveShader()
{
  m_program.deactivate(m_gl, m_shaderManager);
}

ShaderProgram& ActiveShader::program()
{
  return m_program;
}

} // namespace tb::gl
