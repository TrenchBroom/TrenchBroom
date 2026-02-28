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

#include "gl/VboManager.h"

#include "Macros.h"
#include "gl/GlUtils.h"
#include "gl/Vbo.h"

#include <algorithm>
#include <memory>

namespace tb::gl
{
namespace
{

/**
 * e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
 */
GLenum typeToOpenGL(const VboType type)
{
  switch (type)
  {
  case VboType::ArrayBuffer:
    return GL_ARRAY_BUFFER;
  case VboType::ElementArrayBuffer:
    return GL_ELEMENT_ARRAY_BUFFER;
    switchDefault();
  }
}

GLenum usageToOpenGL(const VboUsage usage)
{
  switch (usage)
  {
  case VboUsage::StaticDraw:
    return GL_STATIC_DRAW;
  case VboUsage::DynamicDraw:
    return GL_DYNAMIC_DRAW;
    switchDefault();
  }
}

} // namespace

// VboManager

VboManager::VboManager() = default;
VboManager::~VboManager() = default;

std::unique_ptr<Vbo> VboManager::allocateVbo(
  Gl& gl, VboType type, const size_t capacity, const VboUsage usage)
{
  auto result =
    std::make_unique<Vbo>(gl, typeToOpenGL(type), capacity, usageToOpenGL(usage));

  m_currentVboSize += capacity;
  m_currentVboCount++;
  m_peakVboCount = std::max(m_peakVboCount, m_currentVboCount);

  return result;
}

void VboManager::destroyVbo(std::unique_ptr<Vbo> vbo)
{
  m_currentVboSize -= vbo->capacity();
  m_currentVboCount--;

  m_vbosToDestroy.push_back(std::move(vbo));
}

size_t VboManager::peakVboCount() const
{
  return m_peakVboCount;
}

size_t VboManager::currentVboCount() const
{
  return m_currentVboCount;
}

size_t VboManager::currentVboSize() const
{
  return m_currentVboSize;
}

void VboManager::destroyPendingVbos(Gl& gl)
{
  for (auto& vbo : m_vbosToDestroy)
  {
    vbo->free(gl);
  }
  m_vbosToDestroy.clear();
}

} // namespace tb::gl
