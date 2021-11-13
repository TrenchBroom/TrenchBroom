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

#include "Macros.h"

namespace TrenchBroom {
namespace Renderer {
class RenderContext;
class VboManager;

class Renderable {
public:
  Renderable() = default;
  virtual ~Renderable() = default;

  void render(RenderContext& renderContext);

private:
  virtual void doRender(RenderContext& renderContext) = 0;

  defineCopyAndMove(Renderable)
};

class DirectRenderable : public Renderable {
public:
  DirectRenderable() = default;
  ~DirectRenderable() override = default;

  void prepareVertices(VboManager& vboManager);

private:
  virtual void doPrepareVertices(VboManager& vboManager) = 0;

  defineCopyAndMove(DirectRenderable)
};

class IndexedRenderable : public Renderable {
public:
  IndexedRenderable() = default;
  ~IndexedRenderable() override = default;

  virtual void prepareVerticesAndIndices(VboManager& vboManager) = 0;

  defineCopyAndMove(IndexedRenderable)
};
} // namespace Renderer
} // namespace TrenchBroom
