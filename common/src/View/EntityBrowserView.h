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

#include "EL/Expression.h"
#include "NotifierConnection.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/GLVertexType.h"
#include "View/CellView.h"

#include "vm/bbox.h"
#include "vm/quat.h" // IWYU pragma: keep

#include <optional>
#include <string>
#include <vector>

namespace tb
{
class Logger;
}

namespace tb::assets
{
class EntityDefinition;
enum class EntityDefinitionSortOrder;
enum class Orientation;
class PointEntityDefinition;
class ResourceId;
} // namespace tb::assets

namespace tb::Renderer
{
class FontDescriptor;
class MaterialRenderer;
class Transformation;
} // namespace tb::Renderer

namespace tb::View
{
class MapDocument;

using EntityGroupData = std::string;

struct EntityCellData
{
  using EntityRenderer = Renderer::MaterialRenderer;
  const assets::PointEntityDefinition* entityDefinition;
  EntityRenderer* modelRenderer;
  assets::Orientation modelOrientation;
  Renderer::FontDescriptor fontDescriptor;
  vm::bbox3f bounds;
  vm::vec3f modelScale;
};

class EntityBrowserView : public CellView
{
  Q_OBJECT
private:
  using EntityRenderer = Renderer::MaterialRenderer;

  using TextVertex = Renderer::GLVertexTypes::P2UV2C4::Vertex;
  using StringMap = std::map<Renderer::FontDescriptor, std::vector<TextVertex>>;

  static constexpr auto CameraPosition = vm::vec3f{256.0f, 0.0f, 0.0f};
  static constexpr auto CameraDirection = vm::vec3f{-1, 0, 0};
  static constexpr auto CameraUp = vm::vec3f{0, 0, 1};

  std::weak_ptr<MapDocument> m_document;
  std::optional<EL::ExpressionNode> m_defaultScaleModelExpression;
  vm::quatf m_rotation;

  bool m_group = false;
  bool m_hideUnused = false;
  assets::EntityDefinitionSortOrder m_sortOrder;
  std::string m_filterText;

  NotifierConnection m_notifierConnection;

public:
  EntityBrowserView(
    QScrollBar* scrollBar,
    GLContextManager& contextManager,
    std::weak_ptr<MapDocument> document);
  ~EntityBrowserView() override;

public:
  void setDefaultModelScaleExpression(
    std::optional<EL::ExpressionNode> defaultModelScaleExpression);

  void setSortOrder(assets::EntityDefinitionSortOrder sortOrder);
  void setGroup(bool group);
  void setHideUnused(bool hideUnused);
  void setFilterText(const std::string& filterText);

private:
  void doInitLayout(Layout& layout) override;
  void doReloadLayout(Layout& layout) override;

  bool dndEnabled() override;
  QString dndData(const Cell& cell) override;

  void resourcesWereProcessed(const std::vector<assets::ResourceId>& resources);

  void addEntitiesToLayout(
    Layout& layout,
    const std::vector<assets::EntityDefinition*>& definitions,
    const Renderer::FontDescriptor& font);
  void addEntityToLayout(
    Layout& layout,
    const assets::PointEntityDefinition* definition,
    const Renderer::FontDescriptor& font);

  void doClear() override;
  void doRender(Layout& layout, float y, float height) override;
  bool shouldRenderFocusIndicator() const override;
  const Color& getBackgroundColor() override;

  void renderBounds(Layout& layout, float y, float height);

  class MeshFunc;
  void renderModels(
    Layout& layout, float y, float height, Renderer::Transformation& transformation);

  vm::mat4x4f itemTransformation(
    const Cell& cell, float y, float height, bool applyModelScale) const;

  QString tooltip(const Cell& cell) override;

  const EntityCellData& cellData(const Cell& cell) const;
};

} // namespace tb::View
