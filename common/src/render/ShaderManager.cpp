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

#include "ShaderManager.h"

#include "Ensure.h"
#include "io/SystemPaths.h"
#include "render/ShaderConfig.h"

#include "kd/result.h"
#include "kd/result_fold.h"

#include <cassert>
#include <filesystem>
#include <ranges>
#include <string>

namespace tb::render
{

Result<void> ShaderManager::loadProgram(const ShaderConfig& config)
{
  return createProgram(config) | kdl::and_then([&](auto program) -> Result<void> {
           if (!m_programs.emplace(config.name, std::move(program)).second)
           {
             return Error{"Shader program '" + config.name + "' already loaded"};
           }
           return kdl::void_success;
         });
}

ShaderProgram& ShaderManager::program(const ShaderConfig& config)
{
  auto it = m_programs.find(config.name);
  ensure(it != std::end(m_programs), "Shader program was previously loaded");
  return it->second;
}

ShaderProgram* ShaderManager::currentProgram()
{
  return m_currentProgram;
}

void ShaderManager::setCurrentProgram(ShaderProgram* program)
{
  m_currentProgram = program;
}

Result<ShaderProgram> ShaderManager::createProgram(const ShaderConfig& config)
{
  return createShaderProgram(config.name) | kdl::and_then([&](auto program) {
           return config.vertexShaders | std::views::transform([&](const auto& path) {
                    return loadShader(path, GL_VERTEX_SHADER)
                           | kdl::transform(
                             [&](auto shader) { program.attach(shader.get()); });
                  })
                  | kdl::fold | kdl::transform([&]() { return std::move(program); });
         })
         | kdl::and_then([&](auto program) {
             return config.fragmentShaders | std::views::transform([&](const auto& path) {
                      return loadShader(path, GL_FRAGMENT_SHADER)
                             | kdl::transform(
                               [&](auto shader) { program.attach(shader.get()); });
                    })
                    | kdl::fold | kdl::transform([&]() { return std::move(program); });
           })
         | kdl::and_then([&](auto program) {
             return program.link() | kdl::transform([&]() { return std::move(program); });
           });
}

Result<std::reference_wrapper<Shader>> ShaderManager::loadShader(
  const std::string& name, const GLenum type)
{
  auto it = m_shaders.find(name);
  if (it != std::end(m_shaders))
  {
    return std::ref(it->second);
  }

  const auto shaderPath =
    io::SystemPaths::findResourceFile(std::filesystem::path{"shader"} / name);

  return render::loadShader(shaderPath, type) | kdl::transform([&](auto shader) {
           const auto [insertIt, inserted] = m_shaders.emplace(name, std::move(shader));

           assert(inserted);
           unused(inserted);

           return std::ref(insertIt->second);
         });
}

} // namespace tb::render
