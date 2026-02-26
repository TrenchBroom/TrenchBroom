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

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace tb::gl
{
class Gl;
class Vbo;

enum class VboType
{
  ArrayBuffer,
  ElementArrayBuffer
};

enum class VboUsage
{
  StaticDraw,
  DynamicDraw
};

class VboManager
{
private:
  size_t m_peakVboCount = 0;
  size_t m_currentVboCount = 0;
  size_t m_currentVboSize = 0;

  std::vector<std::unique_ptr<Vbo>> m_vbosToDestroy;

public:
  VboManager();
  ~VboManager();

  /**
   * Immediately creates and binds to an OpenGL buffer of the given type and capacity.
   * The contents are initially unspecified. See Vbo class.
   */
  std::unique_ptr<Vbo> allocateVbo(
    Gl& gl, VboType type, size_t capacity, VboUsage usage = VboUsage::StaticDraw);
  void destroyVbo(std::unique_ptr<Vbo> vbo);

  size_t peakVboCount() const;
  size_t currentVboCount() const;
  size_t currentVboSize() const;

  void destroyPendingVbos(Gl& gl);
};

} // namespace tb::gl
