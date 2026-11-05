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

#include <concepts>
#include <memory>
#include <vector>

namespace tb
{
namespace gl
{
class Gl;
class VboManager;
} // namespace gl

namespace render
{
class Renderable;
class DirectRenderable;
class IndexedRenderable;
class RenderContext;

class RenderBatch
{
private:
  gl::VboManager& m_vboManager;

  using RenderableList = std::vector<Renderable*>;
  using DirectRenderableList = std::vector<DirectRenderable*>;
  using IndexedRenderableList = std::vector<std::unique_ptr<IndexedRenderable>>;

  DirectRenderableList m_directRenderables;
  IndexedRenderableList m_indexedRenderables;

  RenderableList m_batch;
  std::vector<std::unique_ptr<Renderable>> m_oneshots;

public:
  explicit RenderBatch(gl::VboManager& vboManager);
  ~RenderBatch();

  void add(Renderable* renderable);
  void add(DirectRenderable* renderable);
  void add(IndexedRenderable* renderable);

  /**
   * Same as `add()`, but takes ownership of the given renderable and deletes it in
   * `~RenderBatch`.
   */
  template <std::derived_from<Renderable> T>
  void addOneShot(std::unique_ptr<T> renderable)
  {
    add(renderable.get());
    m_oneshots.push_back(std::move(renderable));
  }

  void render(RenderContext& renderContext);

private:
  void doAdd(Renderable* renderable);

  void prepareRenderables(gl::Gl& gl);

  void renderRenderables(RenderContext& renderContext);
};

} // namespace render
} // namespace tb
