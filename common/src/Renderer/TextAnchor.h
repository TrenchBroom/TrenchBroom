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

#pragma once

#include "vecmath/forward.h"
#include "vecmath/vec.h"

namespace TrenchBroom
{
namespace Renderer
{
class Camera;

namespace TextAlignment
{
using Type = unsigned int;
static const Type Top = 1 << 0;
static const Type Bottom = 1 << 1;
static const Type Left = 1 << 2;
static const Type Right = 1 << 3;
static const Type Center = 1 << 4;
} // namespace TextAlignment

class TextAnchor
{
public:
  virtual ~TextAnchor();
  virtual vm::vec3f offset(const Camera& camera, const vm::vec2f& size) const = 0;
  virtual vm::vec3f position(const Camera& camera) const = 0;
};

class TextAnchor3D : public TextAnchor
{
public:
  virtual ~TextAnchor3D() override;
  vm::vec3f offset(const Camera& camera, const vm::vec2f& size) const override;
  vm::vec3f position(const Camera& camera) const override;

private:
  vm::vec2f alignmentFactors(TextAlignment::Type a) const;

private:
  virtual vm::vec3f basePosition() const = 0;
  virtual TextAlignment::Type alignment() const = 0;
  virtual vm::vec2f extraOffsets(TextAlignment::Type a) const;
};

class SimpleTextAnchor : public TextAnchor3D
{
private:
  vm::vec3f m_position;
  TextAlignment::Type m_alignment;
  vm::vec2f m_extraOffsets;

public:
  SimpleTextAnchor(
    const vm::vec3f& position,
    const TextAlignment::Type alignment,
    const vm::vec2f& extraOffsets = vm::vec2f::zero());

private:
  vm::vec3f basePosition() const override;
  TextAlignment::Type alignment() const override;
  vm::vec2f extraOffsets(TextAlignment::Type a) const override;
};
} // namespace Renderer
} // namespace TrenchBroom
