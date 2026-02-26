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

#include "Macros.h"

namespace tb
{
namespace gl
{
class Gl;
class VboManager;
} // namespace gl

namespace render
{
class RenderContext;

class Renderable
{
public:
  Renderable();
  virtual ~Renderable();

  virtual void render(RenderContext& renderContext) = 0;

  defineCopyAndMove(Renderable);
};

class DirectRenderable : public Renderable
{
public:
  DirectRenderable();
  ~DirectRenderable() override;

  virtual void prepare(gl::Gl& gl, gl::VboManager& vboManager) = 0;

  defineCopyAndMove(DirectRenderable);
};

class IndexedRenderable : public Renderable
{
public:
  IndexedRenderable();
  ~IndexedRenderable() override;

  virtual void prepare(gl::Gl& gl, gl::VboManager& vboManager) = 0;

  defineCopyAndMove(IndexedRenderable);
};

} // namespace render
} // namespace tb
