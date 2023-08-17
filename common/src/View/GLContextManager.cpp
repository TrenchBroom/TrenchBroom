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

#include "GLContextManager.h"

#include "Exceptions.h"
#include "Renderer/FontManager.h"
#include "Renderer/GL.h"
#include "Renderer/RenderError.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Shaders.h"
#include "Renderer/Vbo.h"

#include "kdl/result_fold.h"
#include "kdl/vector_utils.h"
#include <kdl/result.h>

#include <sstream>
#include <string>

namespace TrenchBroom
{
namespace View
{
std::string GLContextManager::GLVendor = "unknown";
std::string GLContextManager::GLRenderer = "unknown";
std::string GLContextManager::GLVersion = "unknown";

GLContextManager::GLContextManager()
  : m_initialized(false)
  , m_shaderManager(std::make_unique<Renderer::ShaderManager>())
  , m_vboManager(std::make_unique<Renderer::VboManager>(m_shaderManager.get()))
  , m_fontManager(std::make_unique<Renderer::FontManager>())
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
  const GLenum glewState = glewInit();
  if (glewState != GLEW_OK)
  {
    auto str = std::stringstream();
    str << "Error initializing glew: " << glewGetErrorString(glewState);
    throw RenderException(str.str());
  }
}

bool GLContextManager::initialize()
{
  using namespace Renderer::Shaders;

  if (!m_initialized)
  {
    m_initialized = true;

    initializeGlew();

    GLVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    GLRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    GLVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    kdl::fold_results(kdl::vec_transform(
                        std::vector<Renderer::ShaderConfig>{
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
                          TextureBrowserShader,
                          TextureBrowserBorderShader,
                          HandleShader,
                          ColoredHandleShader,
                          CompassShader,
                          CompassOutlineShader,
                          CompassBackgroundShader,
                          LinkLineShader,
                          LinkArrowShader,
                          TriangleShader,
                          UVViewShader,
                        },
                        [&](const auto& shaderConfig) {
                          return m_shaderManager->loadProgram(shaderConfig);
                        }))
      .transform_error([&](const auto& e) { throw RenderException{e.msg}; });

    return true;
  }
  return false;
}

Renderer::VboManager& GLContextManager::vboManager()
{
  return *m_vboManager;
}

Renderer::FontManager& GLContextManager::fontManager()
{
  return *m_fontManager;
}

Renderer::ShaderManager& GLContextManager::shaderManager()
{
  return *m_shaderManager;
}
} // namespace View
} // namespace TrenchBroom
