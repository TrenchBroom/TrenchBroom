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

#include "GLContextManager.h"

#include "Exceptions.h"
#include "render/FontManager.h"
#include "render/GL.h"
#include "render/ShaderManager.h"
#include "render/ShaderProgram.h"
#include "render/Shaders.h"
#include "render/VboManager.h"

#include "kd/result.h"
#include "kd/result_fold.h"

#include <fmt/format.h>

#include <ranges>
#include <vector>

namespace tb::ui
{
std::string GLContextManager::GLVendor = "unknown";
std::string GLContextManager::GLRenderer = "unknown";
std::string GLContextManager::GLVersion = "unknown";

GLContextManager::GLContextManager()
  : m_shaderManager{std::make_unique<render::ShaderManager>()}
  , m_vboManager{std::make_unique<render::VboManager>(*m_shaderManager)}
  , m_fontManager{std::make_unique<render::FontManager>()}
{
}

GLContextManager::~GLContextManager() = default;

bool GLContextManager::initialized() const
{
  return m_initialized;
}

static void initializeGlew()
{
  glewExperimental = GL_TRUE;
  if (const auto glewState = glewInit(); glewState != GLEW_OK)
  {
    throw RenderException{fmt::format(
      "Error initializing glew: {}",
      reinterpret_cast<const char*>(glewGetErrorString(glewState)))};
  }
}

bool GLContextManager::initialize()
{
  using namespace render::Shaders;

  if (!m_initialized)
  {
    m_initialized = true;

    initializeGlew();

    GLVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    GLRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    GLVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    const auto shaders = std::vector<render::ShaderConfig>{
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
      | kdl::transform_error([&](const auto& e) { throw RenderException{e.msg}; });

    return true;
  }
  return false;
}

render::VboManager& GLContextManager::vboManager()
{
  return *m_vboManager;
}

render::FontManager& GLContextManager::fontManager()
{
  return *m_fontManager;
}

render::ShaderManager& GLContextManager::shaderManager()
{
  return *m_shaderManager;
}

} // namespace tb::ui
