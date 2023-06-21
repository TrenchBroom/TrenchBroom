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

#include "ShaderManager.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "IO/SystemPaths.h"
#include "Renderer/RenderError.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderConfig.h"
#include "Renderer/ShaderProgram.h"

#include <kdl/result.h>

#include <cassert>
#include <filesystem>
#include <string>

namespace TrenchBroom::Renderer
{

kdl::result<void, RenderError> ShaderManager::loadProgram(const ShaderConfig& config)
{
  try
  {
    if (!m_programs.emplace(config.name(), createProgram(config)).second)
    {
      return RenderError{"Shader program '" + config.name() + "' already loaded"};
    }
    return kdl::void_success;
  }
  catch (const Exception& e)
  {
    return RenderError{e.what()};
  }
}

ShaderProgram& ShaderManager::program(const ShaderConfig& config)
{
  auto it = m_programs.find(config.name());
  if (it == std::end(m_programs))
  {
    throw RenderException{"Unknown shader program '" + config.name() + "'"};
  }

  return *it->second;
}

ShaderProgram* ShaderManager::currentProgram()
{
  return m_currentProgram;
}

void ShaderManager::setCurrentProgram(ShaderProgram* program)
{
  m_currentProgram = program;
}

std::unique_ptr<ShaderProgram> ShaderManager::createProgram(const ShaderConfig& config)
{
  auto program = createShaderProgram(config.name());

  for (const auto& path : config.vertexShaders())
  {
    auto& shader = loadShader(path, GL_VERTEX_SHADER);
    program->attach(shader);
  }

  for (const auto& path : config.fragmentShaders())
  {
    auto& shader = loadShader(path, GL_FRAGMENT_SHADER);
    program->attach(shader);
  }

  return program;
}

Shader& ShaderManager::loadShader(const std::string& name, const GLenum type)
{
  auto it = m_shaders.find(name);
  if (it == std::end(m_shaders))
  {
    const auto shaderPath =
      IO::SystemPaths::findResourceFile(std::filesystem::path{"shader"} / name);

    auto inserted = false;
    std::tie(it, inserted) =
      m_shaders.emplace(name, Renderer::loadShader(shaderPath, type));
    assert(inserted);
  }

  return *it->second;
}

} // namespace TrenchBroom::Renderer
