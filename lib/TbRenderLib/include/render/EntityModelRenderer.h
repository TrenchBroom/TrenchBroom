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

#include "base/Color.h"
#include "render/Renderable.h"

#include <unordered_map>
#include <vector>

namespace tb
{
class Logger;

namespace gl
{
class MaterialRenderer;
}

namespace mdl
{
class EditorContext;
class EntityModelManager;
class EntityNode;
} // namespace mdl

namespace render
{
class RenderBatch;
struct ShaderConfig;

class EntityModelRenderer
{
private:
  /**
   * Draws either the opaque or the transparent subset of m_entities as an independent
   * batch entry, so the two subsets can be interleaved with other objects'
   * opaque/transparent passes rather than always drawn together. The transparent pass is
   * additionally sorted back-to-front by distance to the camera each frame.
   */
  class Pass : public DirectRenderable
  {
  private:
    EntityModelRenderer& m_owner;
    bool m_transparent;

  public:
    Pass(EntityModelRenderer& owner, bool transparent);

  private:
    void prepare(gl::Gl& gl, gl::VboManager& vboManager) override;
    void render(RenderContext& context) override;
  };

  Logger& m_logger;

  mdl::EntityModelManager& m_entityModelManager;
  const mdl::EditorContext& m_editorContext;

  std::unordered_map<const mdl::EntityNode*, gl::MaterialRenderer*> m_entities;
  std::vector<const mdl::EntityNode*> m_transparentDrawOrder;

  Pass m_opaquePass;
  Pass m_transparentPass;

  bool m_applyTinting = false;
  Color m_tintColor;

  bool m_showHiddenEntities = false;

public:
  EntityModelRenderer(
    Logger& logger,
    mdl::EntityModelManager& entityModelManager,
    const mdl::EditorContext& editorContext);
  ~EntityModelRenderer();

  template <typename I>
  void setEntities(I cur, I end)
  {
    clear();
    addEntities(cur, end);
  }

  template <typename I>
  void addEntities(I cur, I end)
  {
    while (cur != end)
    {
      addEntity(*cur);
      ++cur;
    }
  }

  template <typename I>
  void updateEntities(I cur, I end)
  {
    while (cur != end)
    {
      updateEntity(**cur);
      ++cur;
    }
  }

  void addEntity(const mdl::EntityNode& entityNode);
  void removeEntity(const mdl::EntityNode& entityNode);
  void updateEntity(const mdl::EntityNode& entityNode);
  void clear();

  bool applyTinting() const;
  void setApplyTinting(bool applyTinting);
  const Color& tintColor() const;
  void setTintColor(const Color& tintColor);

  bool showHiddenEntities() const;
  void setShowHiddenEntities(bool showHiddenEntities);

  void renderOpaque(RenderBatch& renderBatch);
  void renderTransparent(RenderBatch& renderBatch);

private:
  void renderPass(RenderContext& renderContext, bool transparent);
};

} // namespace render
} // namespace tb
