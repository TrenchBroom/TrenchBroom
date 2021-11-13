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

#include "VboManager.h"

#include "GL.h"
#include "Macros.h"
#include "Vbo.h"

#include <algorithm> // for std::max

namespace TrenchBroom {
namespace Renderer {
/**
 * e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
 */
static GLenum typeToOpenGL(const VboType type) {
  switch (type) {
    case VboType::ArrayBuffer:
      return GL_ARRAY_BUFFER;
    case VboType::ElementArrayBuffer:
      return GL_ELEMENT_ARRAY_BUFFER;
      switchDefault()
  }
}

static GLenum usageToOpenGL(const VboUsage usage) {
  switch (usage) {
    case VboUsage::StaticDraw:
      return GL_STATIC_DRAW;
    case VboUsage::DynamicDraw:
      return GL_DYNAMIC_DRAW;
      switchDefault()
  }
}

// VboManager

VboManager::VboManager(ShaderManager* shaderManager)
  : m_peakVboCount(0u)
  , m_currentVboCount(0u)
  , m_currentVboSize(0u)
  , m_shaderManager(shaderManager) {}

Vbo* VboManager::allocateVbo(VboType type, const size_t capacity, const VboUsage usage) {
  auto* result = new Vbo(typeToOpenGL(type), capacity, usageToOpenGL(usage));

  m_currentVboSize += capacity;
  m_currentVboCount++;
  m_peakVboCount = std::max(m_peakVboCount, m_currentVboCount);

  return result;
}

void VboManager::destroyVbo(Vbo* vbo) {
  m_currentVboSize -= vbo->capacity();
  m_currentVboCount--;

  vbo->free();
  delete vbo;
}

size_t VboManager::peakVboCount() const {
  return m_peakVboCount;
}

size_t VboManager::currentVboCount() const {
  return m_currentVboCount;
}

size_t VboManager::currentVboSize() const {
  return m_currentVboSize;
}

ShaderManager& VboManager::shaderManager() {
  return *m_shaderManager;
}
} // namespace Renderer
} // namespace TrenchBroom
