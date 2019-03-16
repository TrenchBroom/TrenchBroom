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

#include "Preferences.h"
#include "Logger.h"
#include "StepIterator.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModel.h"
#include "Assets/EntityModelManager.h"
#include "Assets/ModelDefinition.h"
#include "Renderer/GL.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TextureFont.h"
#include "Renderer/Transformation.h"
#include "Renderer/TexturedIndexRangeRenderer.h"
#include "Renderer/GLVertex.h"
#include "Renderer/VertexArray.h"
#include "View/MapFrame.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/quat.h>
#include <vecmath/bbox.h>

#include <map>

namespace TrenchBroom {
    namespace View {
        EntityCellData::EntityCellData(const Assets::PointEntityDefinition* i_entityDefinition, EntityRenderer* i_modelRenderer, const Renderer::FontDescriptor& i_fontDescriptor, const vm::bbox3f& i_bounds) :
        entityDefinition(i_entityDefinition),
        modelRenderer(i_modelRenderer),
        fontDescriptor(i_fontDescriptor),
        bounds(i_bounds) {}

        EntityBrowserView::EntityBrowserView(wxWindow* parent,
                                             wxScrollBar* scrollBar,
                                             GLContextManager& contextManager,
                                             Assets::EntityDefinitionManager& entityDefinitionManager,
                                             Assets::EntityModelManager& entityModelManager,
                                             Logger& logger) :
        CellView(parent, contextManager, GLAttribs::attribs(), scrollBar),
        m_entityDefinitionManager(entityDefinitionManager),
        m_entityModelManager(entityModelManager),
        m_logger(logger),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Assets::EntityDefinition::Name) {
            const vm::quatf hRotation = vm::quatf(vm::vec3f::pos_z, vm::toRadians(-30.0f));
            const vm::quatf vRotation = vm::quatf(vm::vec3f::pos_y, vm::toRadians(20.0f));
            m_rotation = vRotation * hRotation;

            m_entityDefinitionManager.usageCountDidChangeNotifier.addObserver(this, &EntityBrowserView::usageCountDidChange);
        }

        EntityBrowserView::~EntityBrowserView() {
            clear();
        }

        void EntityBrowserView::setSortOrder(const Assets::EntityDefinition::SortOrder sortOrder) {
            if (sortOrder == m_sortOrder)
                return;
            m_sortOrder = sortOrder;
            invalidate();
            Refresh();
        }

        void EntityBrowserView::setGroup(const bool group) {
            if (group == m_group)
                return;
            m_group = group;
            invalidate();
            Refresh();
        }

        void EntityBrowserView::setHideUnused(const bool hideUnused) {
            if (hideUnused == m_hideUnused)
                return;
            m_hideUnused = hideUnused;
            invalidate();
            Refresh();
        }

        void EntityBrowserView::setFilterText(const String& filterText) {
            if (filterText == m_filterText)
                return;
            m_filterText = filterText;
            invalidate();
            Refresh();
        }

        void EntityBrowserView::usageCountDidChange() {
            invalidate();
            Refresh();
        }

        void EntityBrowserView::doInitLayout(Layout& layout) {
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setCellWidth(93.0f, 93.0f);
            layout.setCellHeight(64.0f, 128.0f);
            layout.setMaxUpScale(1.5f);
        }

        void EntityBrowserView::doReloadLayout(Layout& layout) {
            const auto& fontPath = pref(Preferences::RendererFontPath());
            const auto fontSize = pref(Preferences::BrowserFontSize);
            assert(fontSize > 0);

            const Renderer::FontDescriptor font(fontPath, static_cast<size_t>(fontSize));

            if (m_group) {
                for (const auto& group : m_entityDefinitionManager.groups()) {
                    const auto& definitions = group.definitions(Assets::EntityDefinition::Type_PointEntity, m_sortOrder);

                    if (!definitions.empty()) {
                        const auto displayName = group.displayName();
                        layout.addGroup(displayName, fontSize + 2.0f);

                        for (const auto* definition : definitions) {
                            const auto* pointEntityDefinition = static_cast<const Assets::PointEntityDefinition*>(definition);
                            addEntityToLayout(layout, pointEntityDefinition, font);
                        }
                    }
                }
            } else {
                const auto& definitions = m_entityDefinitionManager.definitions(Assets::EntityDefinition::Type_PointEntity, m_sortOrder);
                for (const auto* definition : definitions) {
                    const auto* pointEntityDefinition = static_cast<const Assets::PointEntityDefinition*>(definition);
                    addEntityToLayout(layout, pointEntityDefinition, font);
                }
            }
        }

        bool EntityBrowserView::dndEnabled() {
            return true;
        }

        void EntityBrowserView::dndWillStart() {
            MapFrame* mapFrame = findMapFrame(this);
            ensure(mapFrame != nullptr, "mapFrame is null");
            mapFrame->setToolBoxDropTarget();
        }

        void EntityBrowserView::dndDidEnd() {
            MapFrame* mapFrame = findMapFrame(this);
            ensure(mapFrame != nullptr, "mapFrame is null");
            mapFrame->clearDropTarget();
        }

        wxString EntityBrowserView::dndData(const Layout::Group::Row::Cell& cell) {
            static const String prefix("entity:");
            const String name = cell.item().entityDefinition->name();
            return wxString(prefix + name);
        }

        void EntityBrowserView::addEntityToLayout(Layout& layout, const Assets::PointEntityDefinition* definition, const Renderer::FontDescriptor& font) {
            if ((!m_hideUnused || definition->usageCount() > 0) &&
                (m_filterText.empty() || StringUtils::containsCaseInsensitive(definition->name(), m_filterText))) {

                const auto maxCellWidth = layout.maxCellWidth();
                const auto actualFont = fontManager().selectFontSize(font, definition->name(), maxCellWidth, 5);
                const auto actualSize = fontManager().font(actualFont).measure(definition->name());

                const auto spec = definition->defaultModel();
                const auto* model = safeGetModel(m_entityModelManager, spec, m_logger);
                Renderer::TexturedRenderer* modelRenderer = nullptr;

                vm::bbox3f rotatedBounds;
                if (model != nullptr) {
                    const auto bounds = model->bounds(spec.skinIndex, spec.frameIndex);
                    const auto center = bounds.center();
                    const auto transform = translationMatrix(center) * rotationMatrix(m_rotation) * translationMatrix(-center);
                    rotatedBounds = bounds.transform(transform);
                    modelRenderer = m_entityModelManager.renderer(spec);
                } else {
                    rotatedBounds = vm::bbox3f(definition->bounds());
                    const auto center = rotatedBounds.center();
                    const auto transform = translationMatrix(-center) * rotationMatrix(m_rotation) * translationMatrix(center);
                    rotatedBounds = rotatedBounds.transform(transform);
                }

                const auto boundsSize = rotatedBounds.size();
                layout.addItem(EntityCellData(definition, modelRenderer, actualFont, rotatedBounds),
                               boundsSize.y(),
                               boundsSize.z(),
                               actualSize.x(),
                               font.size() + 2.0f);
            }
        }

        void EntityBrowserView::doClear() {}

        void EntityBrowserView::doRender(Layout& layout, const float y, const float height) {
            const float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
            const float viewTop       = static_cast<float>(GetClientRect().GetBottom());
            const float viewRight     = static_cast<float>(GetClientRect().GetRight());
            const float viewBottom    = static_cast<float>(GetClientRect().GetTop());

            const vm::mat4x4f projection = vm::orthoMatrix(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
            const vm::mat4x4f view = vm::viewMatrix(vm::vec3f::neg_x, vm::vec3f::pos_z) * translationMatrix(vm::vec3f(256.0f, 0.0f, 0.0f));
            Renderer::Transformation transformation(projection, view);

            renderBounds(layout, y, height);
            renderModels(layout, y, height, transformation);
            renderNames(layout, y, height, projection);
        }

        bool EntityBrowserView::doShouldRenderFocusIndicator() const {
            return false;
        }

        template <typename Vertex>
        struct CollectBoundsVertices {
            const vm::mat4x4f& transformation;
            const Color& color;
            typename Vertex::List& vertices;

            CollectBoundsVertices(const vm::mat4x4f& i_transformation, const Color& i_color, typename Vertex::List& i_vertices) :
            transformation(i_transformation),
            color(i_color),
            vertices(i_vertices) {}

            void operator()(const vm::vec3f& v1, const vm::vec3f& v2) {
                vertices.emplace_back(transformation * v1, color);
                vertices.emplace_back(transformation * v2, color);
            }
        };

        void EntityBrowserView::renderBounds(Layout& layout, const float y, const float height) {
            using BoundsVertex = Renderer::GLVertexTypes::P3C4::Vertex;
            BoundsVertex::List vertices;

            for (size_t i = 0; i < layout.size(); ++i) {
                const auto& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const auto& cell = row[k];
                                const auto* definition = cell.item().entityDefinition;
                                auto* modelRenderer = cell.item().modelRenderer;

                                if (modelRenderer == nullptr) {
                                    const auto itemTrans = itemTransformation(cell, y, height);
                                    const auto& color = definition->color();
                                    CollectBoundsVertices<BoundsVertex> collect(itemTrans, color, vertices);
                                    vm::bbox3f(definition->bounds()).forEachEdge(collect);
                                }
                            }
                        }
                    }
                }
            }

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::VertexArray vertexArray = Renderer::VertexArray::move(std::move(vertices));

            Renderer::ActivateVbo activate(vertexVbo());
            vertexArray.prepare(vertexVbo());
            vertexArray.render(GL_LINES);
        }

        void EntityBrowserView::renderModels(Layout& layout, const float y, const float height, Renderer::Transformation& transformation) {
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::EntityModelShader);
            shader.set("ApplyTinting", false);
            shader.set("Brightness", pref(Preferences::Brightness));
            shader.set("GrayScale", false);

            glAssert(glFrontFace(GL_CW));

            Renderer::ActivateVbo activate(vertexVbo());
            m_entityModelManager.prepare(vertexVbo());

            for (size_t i = 0; i < layout.size(); ++i) {
                const auto& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const auto& cell = row[k];
                                auto* modelRenderer = cell.item().modelRenderer;

                                if (modelRenderer != nullptr) {
                                    const auto itemTrans = itemTransformation(cell, y, height);
                                    Renderer::MultiplyModelMatrix multMatrix(transformation, itemTrans);
                                    modelRenderer->render();
                                }
                            }
                        }
                    }
                }
            }
        }

        void EntityBrowserView::renderNames(Layout& layout, const float y, const float height, const vm::mat4x4f& projection) {
            Renderer::Transformation transformation(projection, viewMatrix(vm::vec3f::neg_z, vm::vec3f::pos_y) * translationMatrix(vm::vec3f(0.0f, 0.0f, -1.0f)));

            Renderer::ActivateVbo activate(vertexVbo());

            glAssert(glDisable(GL_DEPTH_TEST));
            glAssert(glFrontFace(GL_CCW));
            renderGroupTitleBackgrounds(layout, y, height);
            renderStrings(layout, y, height);
            glAssert(glFrontFace(GL_CW));
        }

        void EntityBrowserView::renderGroupTitleBackgrounds(Layout& layout, const float y, const float height) {
            using Vertex = Renderer::GLVertexTypes::P2::Vertex;
            Vertex::List vertices;

            for (size_t i = 0; i < layout.size(); ++i) {
                const auto& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.left(), height - (titleBounds.top() - y))));
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.left(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.right(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.right(), height - (titleBounds.top() - y))));
                }
            }

            Renderer::VertexArray vertexArray = Renderer::VertexArray::move(std::move(vertices));
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            shader.set("Color", pref(Preferences::BrowserGroupBackgroundColor));

            Renderer::ActivateVbo activate(vertexVbo());
            vertexArray.prepare(vertexVbo());
            vertexArray.render(GL_QUADS);
        }

        void EntityBrowserView::renderStrings(Layout& layout, const float y, const float height) {
            using StringRendererMap = std::map<Renderer::FontDescriptor, Renderer::VertexArray>;
            StringRendererMap stringRenderers;

            { // create and upload all vertex arrays
                Renderer::ActivateVbo activate(vertexVbo());

                const auto stringVertices = collectStringVertices(layout, y, height);
                for (const auto& entry : stringVertices) {
                    const auto& fontDescriptor = entry.first;
                    const auto& vertices = entry.second;
                    stringRenderers[fontDescriptor] = Renderer::VertexArray::ref(vertices);
                    stringRenderers[fontDescriptor].prepare(vertexVbo());
                }
            }

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::ColoredTextShader);
            shader.set("Texture", 0);

            for (auto& entry : stringRenderers) {
                const auto& fontDescriptor = entry.first;
                auto& vertexArray = entry.second;

                auto& font = fontManager().font(fontDescriptor);
                font.activate();
                vertexArray.render(GL_QUADS);
                font.deactivate();
            }
        }

        EntityBrowserView::StringMap EntityBrowserView::collectStringVertices(Layout& layout, const float y, const float height) {
            Renderer::FontDescriptor defaultDescriptor(pref(Preferences::RendererFontPath()),
                                                       static_cast<size_t>(pref(Preferences::BrowserFontSize)));

            const std::vector<Color> textColor{ pref(Preferences::BrowserTextColor) };

            StringMap stringVertices;
            for (size_t i = 0; i < layout.size(); ++i) {
                const auto& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const auto& title = group.item();
                    if (!title.empty()) {
                        const auto titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        const auto offset = vm::vec2f(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());

                        auto& font = fontManager().font(defaultDescriptor);
                        const auto quads = font.quads(title, false, offset);
                        const auto titleVertices = TextVertex::toList(
                            quads.size() / 2,
                            stepIterator(std::begin(quads), 0, 2),
                            stepIterator(std::begin(quads), 1, 2),
                            stepIterator(std::begin(textColor), 0, 0));
                        VectorUtils::append(stringVertices[defaultDescriptor], titleVertices);
                    }

                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const auto& cell = row[k];
                                const auto titleBounds = cell.titleBounds();
                                const auto offset = vm::vec2f(titleBounds.left(), height - (titleBounds.top() - y) - titleBounds.height());

                                Renderer::TextureFont& font = fontManager().font(cell.item().fontDescriptor);
                                const auto quads = font.quads(cell.item().entityDefinition->name(), false, offset);
                                const auto titleVertices = TextVertex::toList(
                                    quads.size() / 2,
                                    stepIterator(std::begin(quads), 0, 2),
                                    stepIterator(std::begin(quads), 1, 2),
                                    stepIterator(std::begin(textColor), 0, 0));
                                VectorUtils::append(stringVertices[cell.item().fontDescriptor], titleVertices);
                            }
                        }
                    }
                }
            }

            return stringVertices;
        }

        vm::mat4x4f EntityBrowserView::itemTransformation(const Layout::Group::Row::Cell& cell, const float y, const float height) const {
            auto* definition = cell.item().entityDefinition;

            const auto offset = vm::vec3f(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y));
            const auto scaling = cell.scale();
            const auto& rotatedBounds = cell.item().bounds;
            const auto rotationOffset = vm::vec3f(0.0f, -rotatedBounds.min.y(), -rotatedBounds.min.z());
            const auto boundsCenter = vm::vec3f(definition->bounds().center());

            return (vm::translationMatrix(offset) *
                    vm::scalingMatrix(vm::vec3f::fill(scaling)) *
                    vm::translationMatrix(rotationOffset) *
                    vm::translationMatrix(boundsCenter) *
                    vm::rotationMatrix(m_rotation) *
                    vm::translationMatrix(-boundsCenter));
        }

        wxString EntityBrowserView::tooltip(const Layout::Group::Row::Cell& cell) {
            return cell.item().entityDefinition->name();
        }
    }
}
