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

#include "NotifierConnection.h"
#include "el/Expression.h"
#include "render/FontDescriptor.h"
#include "render/GLVertexType.h"
#include "ui/CellView.h"

#include "vm/bbox.h"
#include "vm/quat.h" // IWYU pragma: keep

#include <optional>
#include <string>
#include <vector>

namespace tb
{
class Logger;

namespace mdl
{
class ResourceId;

enum class EntityDefinitionSortOrder;
enum class Orientation;

struct EntityDefinition;
} // namespace mdl

namespace render
{
class FontDescriptor;
class MaterialRenderer;
class Transformation;
} // namespace render

namespace ui
{
class MapDocument;

using EntityGroupData = std::string;

struct EntityCellData
{
  using EntityRenderer = render::MaterialRenderer;
  const mdl::EntityDefinition& entityDefinition;
  EntityRenderer* modelRenderer;
  mdl::Orientation modelOrientation;
  render::FontDescriptor fontDescriptor;
  vm::bbox3f bounds;
  vm::mat4x4f transform;
  vm::vec3f modelScale;
};

class EntityBrowserView : public CellView
{
  Q_OBJECT
private:
  using EntityRenderer = render::MaterialRenderer;

  using TextVertex = render::GLVertexTypes::P2UV2C4::Vertex;
  using StringMap = std::map<render::FontDescriptor, std::vector<TextVertex>>;

  static constexpr auto CameraPosition = vm::vec3f{256.0f, 0.0f, 0.0f};
  static constexpr auto CameraDirection = vm::vec3f{-1, 0, 0};
  static constexpr auto CameraUp = vm::vec3f{0, 0, 1};

  MapDocument& m_document;
  std::optional<el::ExpressionNode> m_defaultScaleModelExpression;
  vm::quatf m_rotation;

  bool m_group = false;
  bool m_hideUnused = false;
  mdl::EntityDefinitionSortOrder m_sortOrder;
  std::string m_filterText;

  NotifierConnection m_notifierConnection;

public:
  EntityBrowserView(
    QScrollBar* scrollBar, GLContextManager& contextManager, MapDocument& document);
  ~EntityBrowserView() override;

public:
  void setDefaultModelScaleExpression(
    std::optional<el::ExpressionNode> defaultModelScaleExpression);

  void setSortOrder(mdl::EntityDefinitionSortOrder sortOrder);
  void setGroup(bool group);
  void setHideUnused(bool hideUnused);
  void setFilterText(const std::string& filterText);

private:
  void doInitLayout(Layout& layout) override;
  void doReloadLayout(Layout& layout) override;

  bool dndEnabled() override;
  QString dndData(const Cell& cell) override;

  void resourcesWereProcessed(const std::vector<mdl::ResourceId>& resources);

  void addEntitiesToLayout(
    Layout& layout,
    const std::vector<const mdl::EntityDefinition*>& definitions,
    const render::FontDescriptor& font);
  void addEntityToLayout(
    Layout& layout,
    const mdl::EntityDefinition& definition,
    const render::FontDescriptor& font);

  void doClear() override;
  void doRender(Layout& layout, float y, float height) override;
  bool shouldRenderFocusIndicator() const override;
  const Color& getBackgroundColor() override;

  void renderBounds(Layout& layout, float y, float height);

  class MeshFunc;
  void renderModels(
    Layout& layout, float y, float height, render::Transformation& transformation);

  vm::mat4x4f itemTransformation(const Cell& cell, float y, float height) const;

  QString tooltip(const Cell& cell) override;

  const EntityCellData& cellData(const Cell& cell) const;
};

} // namespace ui
} // namespace tb
