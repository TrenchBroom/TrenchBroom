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

#include "ui/EntityBrowserView.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "el/VariableStore.h"
#include "gl/ActiveShader.h"
#include "gl/FontDescriptor.h"
#include "gl/FontManager.h"
#include "gl/GL.h"
#include "gl/MaterialIndexRangeRenderer.h"
#include "gl/MaterialRenderFunc.h"
#include "gl/PrimType.h"
#include "gl/Shaders.h"
#include "gl/TextureFont.h"
#include "gl/VertexArray.h"
#include "mdl/AssetUtils.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionGroup.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityDefinitionUtils.h"
#include "mdl/EntityModel.h"
#include "mdl/EntityModelManager.h"
#include "mdl/Map.h"
#include "render/Transformation.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/string_compare.h"
#include "kd/string_utils.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/quat.h"
#include "vm/vec.h"

#include <algorithm>
#include <string>
#include <vector>

namespace tb::ui
{

EntityBrowserView::EntityBrowserView(
  AppController& appController, QScrollBar* scrollBar, MapDocument& document)
  : CellView{appController, scrollBar}
  , m_document{document}
  , m_sortOrder{mdl::EntityDefinitionSortOrder::Name}
{
  const auto hRotation = vm::quatf{vm::vec3f{0, 0, 1}, vm::to_radians(-30.0f)};
  const auto vRotation = vm::quatf{vm::vec3f{0, 1, 0}, vm::to_radians(20.0f)};
  m_rotation = vRotation * hRotation;

  m_notifierConnection += m_document.resourcesWereProcessedNotifier.connect(
    this, &EntityBrowserView::resourcesWereProcessed);
}

EntityBrowserView::~EntityBrowserView()
{
  clear();
}

void EntityBrowserView::setDefaultModelScaleExpression(
  std::optional<el::ExpressionNode> defaultScaleExpression)
{
  m_defaultScaleModelExpression = std::move(defaultScaleExpression);
}

void EntityBrowserView::setSortOrder(const mdl::EntityDefinitionSortOrder sortOrder)
{
  if (sortOrder != m_sortOrder)
  {
    m_sortOrder = sortOrder;
    invalidate();
    update();
  }
}

void EntityBrowserView::setGroup(const bool group)
{
  if (group != m_group)
  {
    m_group = group;
    invalidate();
    update();
  }
}

void EntityBrowserView::setHideUnused(const bool hideUnused)
{
  if (hideUnused != m_hideUnused)
  {
    m_hideUnused = hideUnused;
    invalidate();
    update();
  }
}

void EntityBrowserView::setFilterText(const std::string& filterText)
{
  if (filterText != m_filterText)
  {
    m_filterText = filterText;
    invalidate();
    update();
  }
}

void EntityBrowserView::doInitLayout(Layout& layout)
{
  layout.setOuterMargin(5.0f);
  layout.setGroupMargin(5.0f);
  layout.setRowMargin(5.0f);
  layout.setCellMargin(5.0f);
  layout.setCellWidth(93.0f, 93.0f);
  layout.setCellHeight(64.0f, 128.0f);
  layout.setMaxUpScale(1.5f);
}

void EntityBrowserView::doReloadLayout(Layout& layout)
{
  const auto& fontPath = pref(Preferences::RendererFontPath);
  const auto fontSize = pref(Preferences::BrowserFontSize);
  contract_assert(fontSize > 0);

  const auto& entityDefinitionManager = m_document.map().entityDefinitionManager();
  const auto font = gl::FontDescriptor{fontPath, static_cast<size_t>(fontSize)};

  if (m_group)
  {
    for (const auto& group : entityDefinitionManager.groups())
    {
      if (const auto definitions = filterAndSort(
            group.definitions, mdl::EntityDefinitionType::Point, m_sortOrder);
          !definitions.empty())
      {
        layout.addGroup(displayName(group), static_cast<float>(fontSize) + 2.0f);

        addEntitiesToLayout(layout, definitions, font);
      }
    }
  }
  else
  {
    const auto& definitions =
      entityDefinitionManager.definitions(mdl::EntityDefinitionType::Point, m_sortOrder);
    addEntitiesToLayout(layout, definitions, font);
  }
}

bool EntityBrowserView::dndEnabled()
{
  return true;
}

QString EntityBrowserView::dndData(const Cell& cell)
{
  static const auto prefix = QString{"entity:"};
  const auto name = QString::fromStdString(cellData(cell).entityDefinition.name);
  return prefix + name;
}

void EntityBrowserView::resourcesWereProcessed(const std::vector<gl::ResourceId>&)
{
  invalidate();
}

void EntityBrowserView::addEntitiesToLayout(
  Layout& layout,
  const std::vector<const mdl::EntityDefinition*>& definitions,
  const gl::FontDescriptor& font)
{
  for (const auto* definition : definitions)
  {
    addEntityToLayout(layout, *definition, font);
  }
}

namespace
{
bool matchesFilterText(
  const mdl::EntityDefinition& definition, const std::string& filterText)
{
  return filterText.empty()
         || std::ranges::all_of(
           kdl::str_split(filterText, " "), [&](const auto& pattern) {
             return kdl::ci::str_contains(definition.name, pattern);
           });
}
} // namespace

void EntityBrowserView::addEntityToLayout(
  Layout& layout, const mdl::EntityDefinition& definition, const gl::FontDescriptor& font)
{
  auto& map = m_document.map();

  if (
    (!m_hideUnused || definition.usageCount() > 0)
    && matchesFilterText(definition, m_filterText))
  {
    contract_assert(definition.pointEntityDefinition != std::nullopt);
    const auto& pointEntityDefinition = *definition.pointEntityDefinition;

    const auto maxCellWidth = layout.maxCellWidth();
    const auto actualFont =
      fontManager().selectFontSize(font, definition.name, maxCellWidth, 5);
    const auto actualSize = fontManager().font(actualFont).measure(definition.name);
    const auto spec =
      mdl::safeGetModelSpecification(map.logger(), definition.name, [&]() {
        return pointEntityDefinition.modelDefinition.defaultModelSpecification();
      });

    const auto modelScale = vm::vec3f{mdl::safeGetModelScale(
      pointEntityDefinition.modelDefinition,
      el::NullVariableStore{},
      m_defaultScaleModelExpression)};

    auto* modelRenderer = static_cast<gl::MaterialRenderer*>(nullptr);
    auto bounds = vm::bbox3f{};
    auto transform = vm::mat4x4f{};
    auto modelOrientation = mdl::Orientation::Oriented;

    const auto& entityModelManager = map.entityModelManager();
    const auto* model = entityModelManager.model(spec.path);
    const auto* modelData = model ? model->data() : nullptr;
    const auto* modelFrame = modelData ? modelData->frame(spec.frameIndex) : nullptr;
    if (modelFrame)
    {
      modelRenderer = entityModelManager.renderer(spec);
      modelOrientation = modelData->orientation();

      bounds = modelFrame->bounds();

      const auto scalingMatrix = vm::scaling_matrix(modelScale);
      const auto center = bounds.center();
      const auto scaledCenter = scalingMatrix * center;
      transform = vm::translation_matrix(scaledCenter) * vm::rotation_matrix(m_rotation)
                  * scalingMatrix * vm::translation_matrix(-center);
    }
    else
    {
      bounds = vm::bbox3f{pointEntityDefinition.bounds};

      const auto center = bounds.center();
      transform = vm::translation_matrix(-center) * vm::rotation_matrix(m_rotation)
                  * vm::translation_matrix(center);
    }

    const auto rotatedBounds = bounds.transform(transform);
    const auto rotatedBoundsSize = rotatedBounds.size();

    layout.addItem(
      EntityCellData{
        definition,
        modelRenderer,
        modelOrientation,
        actualFont,
        bounds,
        transform,
        modelScale},
      definition.name,
      rotatedBoundsSize.y(),
      rotatedBoundsSize.z(),
      actualSize.x(),
      static_cast<float>(font.size()) + 2.0f);
  }
}

void EntityBrowserView::doClear() {}

void EntityBrowserView::doRender(Layout& layout, const float y, const float height)
{
  const auto viewLeft = static_cast<float>(0);
  const auto viewTop = static_cast<float>(size().height());
  const auto viewRight = static_cast<float>(size().width());
  const auto viewBottom = static_cast<float>(0);

  const auto projection =
    vm::ortho_matrix(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
  const auto view =
    vm::view_matrix(CameraDirection, CameraUp) * vm::translation_matrix(CameraPosition);
  auto transformation = render::Transformation{projection, view};

  renderBounds(layout, y, height);
  renderModels(layout, y, height, transformation);
}

bool EntityBrowserView::shouldRenderFocusIndicator() const
{
  return false;
}

const Color& EntityBrowserView::getBackgroundColor()
{
  return pref(Preferences::BrowserBackgroundColor);
}

void EntityBrowserView::renderBounds(Layout& layout, const float y, const float height)
{
  using BoundsVertex = gl::VertexTypes::P3C4::Vertex;
  auto vertices = std::vector<BoundsVertex>{};

  for (const auto& group : layout.groups())
  {
    if (group.intersectsY(y, height))
    {
      for (const auto& row : group.rows())
      {
        if (row.intersectsY(y, height))
        {
          for (const auto& cell : row.cells())
          {
            const auto& definition = cellData(cell).entityDefinition;
            const auto& pointEntityDefinition = *definition.pointEntityDefinition;
            auto* modelRenderer = cellData(cell).modelRenderer;

            if (modelRenderer == nullptr)
            {
              const auto itemTrans = itemTransformation(cell, y, height);
              const auto& color = definition.color;
              vm::bbox3f{pointEntityDefinition.bounds}.for_each_edge(
                [&](const vm::vec3f& v1, const vm::vec3f& v2) {
                  vertices.emplace_back(itemTrans * v1, color.to<RgbaF>().toVec());
                  vertices.emplace_back(itemTrans * v2, color.to<RgbaF>().toVec());
                });
            }
          }
        }
      }
    }
  }

  auto shader = gl::ActiveShader{shaderManager(), gl::Shaders::VaryingPCShader};
  auto vertexArray = gl::VertexArray::move(std::move(vertices));

  vertexArray.prepare(vboManager());
  vertexArray.render(gl::PrimType::Lines);
}

void EntityBrowserView::renderModels(
  Layout& layout,
  const float y,
  const float height,
  render::Transformation& transformation)
{
  glAssert(glFrontFace(GL_CW));

  auto& entityModelManager = m_document.map().entityModelManager();
  entityModelManager.prepare(vboManager());

  auto shader = gl::ActiveShader{shaderManager(), gl::Shaders::EntityModelShader};
  shader.set("ApplyTinting", false);
  shader.set("Brightness", pref(Preferences::Brightness));
  shader.set("GrayScale", false);

  shader.set("CameraPosition", CameraPosition);
  shader.set("CameraDirection", CameraDirection);
  shader.set("CameraRight", vm::cross(CameraDirection, CameraUp));
  shader.set("CameraUp", CameraUp);
  shader.set("ViewMatrix", transformation.viewMatrix());

  for (const auto& group : layout.groups())
  {
    if (group.intersectsY(y, height))
    {
      for (const auto& row : group.rows())
      {
        if (row.intersectsY(y, height))
        {
          for (const auto& cell : row.cells())
          {
            if (auto* modelRenderer = cellData(cell).modelRenderer)
            {
              shader.set(
                "Orientation", static_cast<int>(cellData(cell).modelOrientation));

              const auto itemTrans = itemTransformation(cell, y, height);
              shader.set("ModelMatrix", itemTrans);

              const auto multMatrix =
                render::MultiplyModelMatrix{transformation, itemTrans};

              auto renderFunc = gl::DefaultMaterialRenderFunc{
                pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter)};
              modelRenderer->render(shader.program(), renderFunc);
            }
          }
        }
      }
    }
  }
}

vm::mat4x4f EntityBrowserView::itemTransformation(
  const Cell& cell, const float y, const float height) const
{
  const auto& cellData = this->cellData(cell);

  const auto offset =
    vm::vec3f{0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y)};
  const auto scaling = cell.scale();
  const auto& rotatedBounds = cellData.bounds.transform(cellData.transform);
  const auto rotationOffset =
    vm::vec3f{0.0f, -rotatedBounds.min.y(), -rotatedBounds.min.z()};

  return vm::translation_matrix(offset) * vm::scaling_matrix(vm::vec3f::fill(scaling))
         * vm::translation_matrix(rotationOffset) * cellData.transform;
}

QString EntityBrowserView::tooltip(const Cell& cell)
{
  return QString::fromStdString(cellData(cell).entityDefinition.name);
}

const EntityCellData& EntityBrowserView::cellData(const Cell& cell) const
{
  return cell.itemAs<EntityCellData>();
}

} // namespace tb::ui
