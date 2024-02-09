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

#include "EntityBrowserView.h"

#include "Assets/AssetUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModel.h"
#include "Assets/EntityModelManager.h"
#include "EL/VariableStore.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/GL.h"
#include "Renderer/GLVertex.h"
#include "Renderer/PrimType.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TextureFont.h"
#include "Renderer/TexturedIndexRangeRenderer.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "View/MapFrame.h"
#include "View/QtUtils.h"

#include "kdl/string_utils.h"
#include <kdl/overload.h>
#include <kdl/skip_iterator.h>
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/quat.h>
#include <vecmath/vec.h>

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom::View
{

EntityBrowserView::EntityBrowserView(
  QScrollBar* scrollBar,
  GLContextManager& contextManager,
  Assets::EntityDefinitionManager& entityDefinitionManager,
  Assets::EntityModelManager& entityModelManager,
  Logger& logger)
  : CellView{contextManager, scrollBar}
  , m_entityDefinitionManager{entityDefinitionManager}
  , m_entityModelManager{entityModelManager}
  , m_logger{logger}
  , m_sortOrder{Assets::EntityDefinitionSortOrder::Name}
{
  const auto hRotation = vm::quatf{vm::vec3f::pos_z(), vm::to_radians(-30.0f)};
  const auto vRotation = vm::quatf{vm::vec3f::pos_y(), vm::to_radians(20.0f)};
  m_rotation = vRotation * hRotation;
}

EntityBrowserView::~EntityBrowserView()
{
  clear();
}

void EntityBrowserView::setDefaultModelScaleExpression(
  std::optional<EL::Expression> defaultScaleExpression)
{
  m_defaultScaleModelExpression = std::move(defaultScaleExpression);
}

void EntityBrowserView::setSortOrder(const Assets::EntityDefinitionSortOrder sortOrder)
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
  const auto& fontPath = pref(Preferences::RendererFontPath());
  const auto fontSize = pref(Preferences::BrowserFontSize);
  assert(fontSize > 0);

  const auto font = Renderer::FontDescriptor{fontPath, static_cast<size_t>(fontSize)};

  if (m_group)
  {
    for (const auto& group : m_entityDefinitionManager.groups())
    {
      const auto& definitions =
        group.definitions(Assets::EntityDefinitionType::PointEntity, m_sortOrder);

      if (!definitions.empty())
      {
        const auto displayName = group.displayName();
        layout.addGroup(displayName, static_cast<float>(fontSize) + 2.0f);

        addEntitiesToLayout(layout, definitions, font);
      }
    }
  }
  else
  {
    const auto& definitions = m_entityDefinitionManager.definitions(
      Assets::EntityDefinitionType::PointEntity, m_sortOrder);
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
  const auto name = QString::fromStdString(cellData(cell).entityDefinition->name());
  return prefix + name;
}

void EntityBrowserView::addEntitiesToLayout(
  Layout& layout,
  const std::vector<Assets::EntityDefinition*>& definitions,
  const Renderer::FontDescriptor& font)
{
  for (const auto* definition : definitions)
  {
    const auto* pointEntityDefinition =
      static_cast<const Assets::PointEntityDefinition*>(definition);
    addEntityToLayout(layout, pointEntityDefinition, font);
  }
}

namespace
{
bool matchesFilterText(
  const Assets::PointEntityDefinition& definition, const std::string& filterText)
{
  return filterText.empty()
         || kdl::all_of(kdl::str_split(filterText, " "), [&](const auto& pattern) {
              return kdl::ci::str_contains(definition.name(), pattern);
            });
}
} // namespace

void EntityBrowserView::addEntityToLayout(
  Layout& layout,
  const Assets::PointEntityDefinition* definition,
  const Renderer::FontDescriptor& font)
{
  if (
    (!m_hideUnused || definition->usageCount() > 0)
    && matchesFilterText(*definition, m_filterText))
  {

    const auto maxCellWidth = layout.maxCellWidth();
    const auto actualFont =
      fontManager().selectFontSize(font, definition->name(), maxCellWidth, 5);
    const auto actualSize = fontManager().font(actualFont).measure(definition->name());
    const auto spec =
      Assets::safeGetModelSpecification(m_logger, definition->name(), [&]() {
        return definition->modelDefinition().defaultModelSpecification();
      });

    const auto* frame = m_entityModelManager.frame(spec);
    const auto modelScale = vm::vec3f{Assets::safeGetModelScale(
      definition->modelDefinition(),
      EL::NullVariableStore{},
      m_defaultScaleModelExpression)};

    auto* modelRenderer = static_cast<Renderer::TexturedRenderer*>(nullptr);
    auto rotatedBounds = vm::bbox3f{};
    auto modelOrientation = Assets::Orientation::Oriented;

    if (frame != nullptr)
    {
      const auto scalingMatrix = vm::scaling_matrix(modelScale);
      const auto bounds = frame->bounds();
      const auto center = bounds.center();
      const auto scaledCenter = scalingMatrix * center;
      const auto transform = vm::translation_matrix(scaledCenter)
                             * vm::rotation_matrix(m_rotation) * scalingMatrix
                             * vm::translation_matrix(-center);

      modelRenderer = m_entityModelManager.renderer(spec);
      rotatedBounds = bounds.transform(transform);
      modelOrientation = frame->orientation();
    }
    else
    {
      rotatedBounds = vm::bbox3f{definition->bounds()};
      const auto center = rotatedBounds.center();
      const auto transform = vm::translation_matrix(-center)
                             * vm::rotation_matrix(m_rotation)
                             * vm::translation_matrix(center);
      rotatedBounds = rotatedBounds.transform(transform);
    }

    const auto boundsSize = rotatedBounds.size();
    layout.addItem(
      EntityCellData{
        definition,
        modelRenderer,
        modelOrientation,
        actualFont,
        rotatedBounds,
        modelScale},
      definition->name(),
      boundsSize.y(),
      boundsSize.z(),
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
  auto transformation = Renderer::Transformation{projection, view};

  renderBounds(layout, y, height);
  renderModels(layout, y, height, transformation);
}

bool EntityBrowserView::doShouldRenderFocusIndicator() const
{
  return false;
}

const Color& EntityBrowserView::getBackgroundColor()
{
  return pref(Preferences::BrowserBackgroundColor);
}

void EntityBrowserView::renderBounds(Layout& layout, const float y, const float height)
{
  using BoundsVertex = Renderer::GLVertexTypes::P3C4::Vertex;
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
            const auto* definition = cellData(cell).entityDefinition;
            auto* modelRenderer = cellData(cell).modelRenderer;

            if (modelRenderer == nullptr)
            {
              const auto itemTrans = itemTransformation(cell, y, height, false);
              const auto& color = definition->color();
              vm::bbox3f{definition->bounds()}.for_each_edge(
                [&](const vm::vec3f& v1, const vm::vec3f& v2) {
                  vertices.emplace_back(itemTrans * v1, color);
                  vertices.emplace_back(itemTrans * v2, color);
                });
            }
          }
        }
      }
    }
  }

  auto shader =
    Renderer::ActiveShader{shaderManager(), Renderer::Shaders::VaryingPCShader};
  auto vertexArray = Renderer::VertexArray::move(std::move(vertices));

  vertexArray.prepare(vboManager());
  vertexArray.render(Renderer::PrimType::Lines);
}

void EntityBrowserView::renderModels(
  Layout& layout,
  const float y,
  const float height,
  Renderer::Transformation& transformation)
{
  glAssert(glFrontFace(GL_CW));

  m_entityModelManager.prepare(vboManager());

  auto shader =
    Renderer::ActiveShader{shaderManager(), Renderer::Shaders::EntityModelShader};
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

              const auto itemTrans = itemTransformation(cell, y, height, true);
              shader.set("ModelMatrix", itemTrans);

              const auto multMatrix =
                Renderer::MultiplyModelMatrix{transformation, itemTrans};
              modelRenderer->render();
            }
          }
        }
      }
    }
  }
}

vm::mat4x4f EntityBrowserView::itemTransformation(
  const Cell& cell, const float y, const float height, const bool applyModelScale) const
{
  const auto& cellData = this->cellData(cell);
  const auto* definition = cellData.entityDefinition;

  const auto offset =
    vm::vec3f{0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y)};
  const auto scaling = cell.scale();
  const auto& rotatedBounds = cellData.bounds;
  const auto modelScale = applyModelScale ? cellData.modelScale : vm::vec3f{1, 1, 1};
  const auto scalingMatrix = vm::scaling_matrix(modelScale);
  const auto rotationOffset =
    vm::vec3f{0.0f, -rotatedBounds.min.y(), -rotatedBounds.min.z()};
  const auto boundsCenter = vm::vec3f{definition->bounds().center()};
  const auto scaledBoundsCenter = scalingMatrix * boundsCenter;

  return vm::translation_matrix(offset) * vm::scaling_matrix(vm::vec3f::fill(scaling))
         * vm::translation_matrix(rotationOffset)
         * vm::translation_matrix(scaledBoundsCenter) * vm::rotation_matrix(m_rotation)
         * scalingMatrix * vm::translation_matrix(-boundsCenter);
}

QString EntityBrowserView::tooltip(const Cell& cell)
{
  return QString::fromStdString(cellData(cell).entityDefinition->name());
}

const EntityCellData& EntityBrowserView::cellData(const Cell& cell) const
{
  return cell.itemAs<EntityCellData>();
}

} // namespace TrenchBroom::View
