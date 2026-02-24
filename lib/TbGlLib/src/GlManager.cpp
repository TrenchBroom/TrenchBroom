/*
 Copyright (C) 2026 Kristian Duske

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

#include "gl/GlManager.h"

#include "gl/FontManager.h"
#include "gl/ResourceManager.h"
#include "gl/ShaderManager.h"
#include "gl/ShaderProgram.h"
#include "gl/Shaders.h"
#include "gl/VboManager.h"

#include "kd/result_fold.h"

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

GlInfo initializeGlInfo()
{
  return {
    reinterpret_cast<const char*>(glGetString(GL_VENDOR)),
    reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
    reinterpret_cast<const char*>(glGetString(GL_VERSION)),
  };
}

void initializeShaders(ShaderManager& shaderManager)
{
  using namespace Shaders;

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
    return shaderManager.loadProgram(shaderConfig);
  }) | kdl::fold
    | kdl::transform_error([&](const auto& e) { throw std::runtime_error{e.msg}; });
}

} // namespace

GlInfo GlManager::m_glInfo = GlInfo{
  .vendor = "unknown",
  .renderer = "unknown",
  .version = "unknown",
};

GlManager::GlManager(FindResourceFunc findResourceFunc)
  : m_resourceManager{std::make_unique<ResourceManager>()}
  , m_shaderManager{std::make_unique<ShaderManager>(findResourceFunc)}
  , m_vboManager{std::make_unique<VboManager>(*m_shaderManager)}
  , m_fontManager{std::make_unique<FontManager>(findResourceFunc)}
{
}

GlManager::~GlManager() = default;

bool GlManager::initialize()
{
  if (m_initialized)
  {
    return false;
  }

  initializeGlew();
  m_glInfo = initializeGlInfo();
  initializeShaders(*m_shaderManager);

  m_initialized = true;
  return true;
}

ResourceManager& GlManager::resourceManager()
{
  return *m_resourceManager;
}

VboManager& GlManager::vboManager()
{
  return *m_vboManager;
}

FontManager& GlManager::fontManager()
{
  return *m_fontManager;
}

ShaderManager& GlManager::shaderManager()
{
  return *m_shaderManager;
}

const GlInfo& GlManager::glInfo()
{
  return m_glInfo;
}

} // namespace tb::gl