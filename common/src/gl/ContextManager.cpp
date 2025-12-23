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

#include "ContextManager.h"

#include "gl/FontManager.h"
#include "gl/GL.h"
#include "gl/ShaderManager.h"
#include "gl/ShaderProgram.h"
#include "gl/Shaders.h"
#include "gl/VboManager.h"

#include "kd/result.h"
#include "kd/result_fold.h"

#include <fmt/format.h>

#include <ranges>
#include <stdexcept>
#include <vector>

namespace tb::gl
{
namespace
{

void initializeGlew()
{
  glewExperimental = GL_TRUE;
  if (const auto glewState = glewInit(); glewState != GLEW_OK)
  {
    throw std::runtime_error{fmt::format(
      "Error initializing glew: {}",
      reinterpret_cast<const char*>(glewGetErrorString(glewState)))};
  }
}

} // namespace

std::string ContextManager::GLVendor = "unknown";
std::string ContextManager::GLRenderer = "unknown";
std::string ContextManager::GLVersion = "unknown";

ContextManager::ContextManager()
  : m_shaderManager{std::make_unique<ShaderManager>()}
  , m_vboManager{std::make_unique<VboManager>(*m_shaderManager)}
  , m_fontManager{std::make_unique<FontManager>()}
{
}

ContextManager::~ContextManager() = default;

bool ContextManager::initialized() const
{
  return m_initialized;
}

bool ContextManager::initialize()
{
  using namespace Shaders;

  if (!m_initialized)
  {
    m_initialized = true;

    initializeGlew();

    GLVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    GLRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    GLVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    const auto shaders = std::vector<ShaderConfig>{
      Grid2DShader,
      VaryingPCShader,
      VaryingPUniformCShader,
      MiniMapEdgeShader,
      EntityModelShader,
      FaceShader,
      PatchShader,
      EdgeShader,
      ColoredTextShader,
      TextBackgroundShader,
      MaterialBrowserShader,
      MaterialBrowserBorderShader,
      HandleShader,
      ColoredHandleShader,
      CompassShader,
      CompassOutlineShader,
      CompassBackgroundShader,
      LinkLineShader,
      LinkArrowShader,
      TriangleShader,
      UVViewShader,
    };

    shaders | std::views::transform([&](const auto& shaderConfig) {
      return m_shaderManager->loadProgram(shaderConfig);
    }) | kdl::fold
      | kdl::transform_error([&](const auto& e) { throw std::runtime_error{e.msg}; });

    return true;
  }
  return false;
}

VboManager& ContextManager::vboManager()
{
  return *m_vboManager;
}

FontManager& ContextManager::fontManager()
{
  return *m_fontManager;
}

ShaderManager& ContextManager::shaderManager()
{
  return *m_shaderManager;
}

} // namespace tb::gl
