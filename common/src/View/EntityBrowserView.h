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

#include "EL/Expression.h"
#include "NotifierConnection.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/GLVertexType.h"
#include "View/CellView.h"

#include "vm/bbox.h" // IWYU pragma: keep
#include "vm/forward.h"
#include "vm/quat.h" // IWYU pragma: keep

#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::Assets
{
class EntityDefinition;
enum class EntityDefinitionSortOrder;
enum class Orientation;
class PointEntityDefinition;
class ResourceId;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Renderer
{
class FontDescriptor;
class MaterialRenderer;
class Transformation;
} // namespace TrenchBroom::Renderer

namespace TrenchBroom::View
{
class MapDocument;

using EntityGroupData = std::string;

struct EntityCellData
{
  using EntityRenderer = Renderer::MaterialRenderer;
  const Assets::PointEntityDefinition* entityDefinition;
  EntityRenderer* modelRenderer;
  Assets::Orientation modelOrientation;
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
  static constexpr auto CameraDirection = vm::vec3f::neg_x();
  static constexpr auto CameraUp = vm::vec3f::pos_z();

  std::weak_ptr<MapDocument> m_document;
  std::optional<EL::Expression> m_defaultScaleModelExpression;
  vm::quatf m_rotation;

  bool m_group = false;
  bool m_hideUnused = false;
  Assets::EntityDefinitionSortOrder m_sortOrder;
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
    std::optional<EL::Expression> defaultModelScaleExpression);

  void setSortOrder(Assets::EntityDefinitionSortOrder sortOrder);
  void setGroup(bool group);
  void setHideUnused(bool hideUnused);
  void setFilterText(const std::string& filterText);

private:
  void doInitLayout(Layout& layout) override;
  void doReloadLayout(Layout& layout) override;

  bool dndEnabled() override;
  QString dndData(const Cell& cell) override;

  void resourcesWereProcessed(const std::vector<Assets::ResourceId>& resources);

  void addEntitiesToLayout(
    Layout& layout,
    const std::vector<Assets::EntityDefinition*>& definitions,
    const Renderer::FontDescriptor& font);
  void addEntityToLayout(
    Layout& layout,
    const Assets::PointEntityDefinition* definition,
    const Renderer::FontDescriptor& font);

  void doClear() override;
  void doRender(Layout& layout, float y, float height) override;
  bool doShouldRenderFocusIndicator() const override;
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

} // namespace TrenchBroom::View
