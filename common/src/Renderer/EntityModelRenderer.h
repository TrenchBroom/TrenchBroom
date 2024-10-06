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

#include "Color.h"
#include "Renderer/Renderable.h"

#include <unordered_map>

namespace tb
{
class Logger;
}

namespace tb::asset
{
class EntityModelManager;
}

namespace tb::mdl
{
class EditorContext;
class EntityNode;
} // namespace tb::mdl

namespace tb::Renderer
{
class RenderBatch;
struct ShaderConfig;
class MaterialRenderer;

class EntityModelRenderer : public DirectRenderable
{
private:
  Logger& m_logger;

  asset::EntityModelManager& m_entityModelManager;
  const mdl::EditorContext& m_editorContext;

  std::unordered_map<const mdl::EntityNode*, MaterialRenderer*> m_entities;

  bool m_applyTinting = false;
  Color m_tintColor;

  bool m_showHiddenEntities = false;

public:
  EntityModelRenderer(
    Logger& logger,
    asset::EntityModelManager& entityModelManager,
    const mdl::EditorContext& editorContext);
  ~EntityModelRenderer() override;

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
      updateEntity(*cur);
      ++cur;
    }
  }

  void addEntity(const mdl::EntityNode* entityNode);
  void removeEntity(const mdl::EntityNode* entityNode);
  void updateEntity(const mdl::EntityNode* entityNode);
  void clear();

  bool applyTinting() const;
  void setApplyTinting(bool applyTinting);
  const Color& tintColor() const;
  void setTintColor(const Color& tintColor);

  bool showHiddenEntities() const;
  void setShowHiddenEntities(bool showHiddenEntities);

  void render(RenderBatch& renderBatch);

private:
  void doPrepareVertices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;
};

} // namespace tb::Renderer
