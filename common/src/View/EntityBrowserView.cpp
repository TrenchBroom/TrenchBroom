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

#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Assets/AssetUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModel.h"
#include "Assets/EntityModelManager.h"
#include "Renderer/GL.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/PrimType.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TextureFont.h"
#include "Renderer/Transformation.h"
#include "Renderer/TexturedIndexRangeRenderer.h"
#include "Renderer/GLVertex.h"
#include "Renderer/VertexArray.h"
#include "View/MapFrame.h"
#include "View/QtUtils.h"

#include <kdl/overload.h>
#include <kdl/skip_iterator.h>
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/quat.h>

#include <map>
#include <string>
#include <vector>

// allow storing std::shared_ptr in QVariant
Q_DECLARE_METATYPE(std::shared_ptr<TrenchBroom::View::EntityCellData>)

namespace TrenchBroom {
    namespace View {
        EntityCellData::EntityCellData(const Assets::PointEntityDefinition* i_entityDefinition, EntityRenderer* i_modelRenderer, const Renderer::FontDescriptor& i_fontDescriptor, const vm::bbox3f& i_bounds) :
        entityDefinition(i_entityDefinition),
        modelRenderer(i_modelRenderer),
        fontDescriptor(i_fontDescriptor),
        bounds(i_bounds) {}

        EntityBrowserView::EntityBrowserView(QScrollBar* scrollBar,
                                             GLContextManager& contextManager,
                                             Assets::EntityDefinitionManager& entityDefinitionManager,
                                             Assets::EntityModelManager& entityModelManager,
                                             Logger& logger) :
        CellView(contextManager, scrollBar),
        m_entityDefinitionManager(entityDefinitionManager),
        m_entityModelManager(entityModelManager),
        m_logger(logger),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Assets::EntityDefinitionSortOrder::Name) {
            const vm::quatf hRotation = vm::quatf(vm::vec3f::pos_z(), vm::to_radians(-30.0f));
            const vm::quatf vRotation = vm::quatf(vm::vec3f::pos_y(), vm::to_radians(20.0f));
            m_rotation = vRotation * hRotation;

            m_entityDefinitionManager.usageCountDidChangeNotifier.addObserver(this, &EntityBrowserView::usageCountDidChange);
        }

        EntityBrowserView::~EntityBrowserView() {
            m_entityDefinitionManager.usageCountDidChangeNotifier.removeObserver(this, &EntityBrowserView::usageCountDidChange);
            clear();
        }

        void EntityBrowserView::setSortOrder(const Assets::EntityDefinitionSortOrder sortOrder) {
            if (sortOrder == m_sortOrder) {
                return;
            }
            m_sortOrder = sortOrder;
            invalidate();
            update();
        }

        void EntityBrowserView::setGroup(const bool group) {
            if (group == m_group) {
                return;
            }
            m_group = group;
            invalidate();
            update();
        }

        void EntityBrowserView::setHideUnused(const bool hideUnused) {
            if (hideUnused == m_hideUnused) {
                return;
            }
            m_hideUnused = hideUnused;
            invalidate();
            update();
        }

        void EntityBrowserView::setFilterText(const std::string& filterText) {
            if (filterText == m_filterText) {
                return;
            }
            m_filterText = filterText;
            invalidate();
            update();
        }

        void EntityBrowserView::usageCountDidChange() {
            invalidate();
            update();
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
                    const auto& definitions = group.definitions(Assets::EntityDefinitionType::PointEntity, m_sortOrder);

                    if (!definitions.empty()) {
                        const auto displayName = group.displayName();
                        layout.addGroup(displayName, static_cast<float>(fontSize)+ 2.0f);

                        for (const auto* definition : definitions) {
                            const auto* pointEntityDefinition = static_cast<const Assets::PointEntityDefinition*>(definition);
                            addEntityToLayout(layout, pointEntityDefinition, font);
                        }
                    }
                }
            } else {
                const auto& definitions = m_entityDefinitionManager.definitions(Assets::EntityDefinitionType::PointEntity, m_sortOrder);
                for (const auto* definition : definitions) {
                    const auto* pointEntityDefinition = static_cast<const Assets::PointEntityDefinition*>(definition);
                    addEntityToLayout(layout, pointEntityDefinition, font);
                }
            }
        }

        bool EntityBrowserView::dndEnabled() {
            return true;
        }

        QString EntityBrowserView::dndData(const Cell& cell) {
            const QString prefix("entity:");
            const QString name = QString::fromStdString(cellData(cell).entityDefinition->name());
            return prefix + name;
        }

        void EntityBrowserView::addEntityToLayout(Layout& layout, const Assets::PointEntityDefinition* definition, const Renderer::FontDescriptor& font) {
            if ((!m_hideUnused || definition->usageCount() > 0) &&
                (m_filterText.empty() || kdl::ci::str_contains(definition->name(), m_filterText))) {

                const auto maxCellWidth = layout.maxCellWidth();
                const auto actualFont = fontManager().selectFontSize(font, definition->name(), maxCellWidth, 5);
                const auto actualSize = fontManager().font(actualFont).measure(definition->name());
                const auto spec = Assets::safeGetModelSpecification(m_logger, definition->name(), [&]() {
                    return definition->defaultModel();
                });

                const auto* frame = m_entityModelManager.frame(spec);
                Renderer::TexturedRenderer* modelRenderer = nullptr;

                vm::bbox3f rotatedBounds;
                if (frame != nullptr) {
                    const auto bounds = frame->bounds();
                    const auto center = bounds.center();
                    const auto transform =vm::translation_matrix(center) * vm::rotation_matrix(m_rotation) *vm::translation_matrix(-center);
                    rotatedBounds = bounds.transform(transform);
                    modelRenderer = m_entityModelManager.renderer(spec);
                } else {
                    rotatedBounds = vm::bbox3f(definition->bounds());
                    const auto center = rotatedBounds.center();
                    const auto transform =vm::translation_matrix(-center) * vm::rotation_matrix(m_rotation) *vm::translation_matrix(center);
                    rotatedBounds = rotatedBounds.transform(transform);
                }

                const auto boundsSize = rotatedBounds.size();
                layout.addItem(QVariant::fromValue(std::make_shared<EntityCellData>(definition, modelRenderer, actualFont, rotatedBounds)),
                               boundsSize.y(),
                               boundsSize.z(),
                               actualSize.x(),
                               static_cast<float>(font.size()) + 2.0f);
            }
        }

        void EntityBrowserView::doClear() {}

        void EntityBrowserView::doRender(Layout& layout, const float y, const float height) {
            const float viewLeft      = static_cast<float>(0);
            const float viewTop       = static_cast<float>(size().height());
            const float viewRight     = static_cast<float>(size().width());
            const float viewBottom    = static_cast<float>(0);

            const vm::mat4x4f projection = vm::ortho_matrix(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
            const vm::mat4x4f view = vm::view_matrix(vm::vec3f::neg_x(), vm::vec3f::pos_z()) *vm::translation_matrix(vm::vec3f(256.0f, 0.0f, 0.0f));
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
            std::vector<Vertex>& vertices;

            CollectBoundsVertices(const vm::mat4x4f& i_transformation, const Color& i_color, std::vector<Vertex>& i_vertices) :
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
            std::vector<BoundsVertex> vertices;

            for (size_t i = 0; i < layout.size(); ++i) {
                const auto& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const auto& cell = row[k];
                                const auto* definition = cellData(cell).entityDefinition;
                                auto* modelRenderer = cellData(cell).modelRenderer;

                                if (modelRenderer == nullptr) {
                                    const auto itemTrans = itemTransformation(cell, y, height);
                                    const auto& color = definition->color();
                                    CollectBoundsVertices<BoundsVertex> collect(itemTrans, color, vertices);
                                    vm::bbox3f(definition->bounds()).for_each_edge(collect);
                                }
                            }
                        }
                    }
                }
            }

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::VertexArray vertexArray = Renderer::VertexArray::move(std::move(vertices));

            vertexArray.prepare(vboManager());
            vertexArray.render(Renderer::PrimType::Lines);
        }

        void EntityBrowserView::renderModels(Layout& layout, const float y, const float height, Renderer::Transformation& transformation) {
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::EntityModelShader);
            shader.set("ApplyTinting", false);
            shader.set("Brightness", pref(Preferences::Brightness));
            shader.set("GrayScale", false);

            glAssert(glFrontFace(GL_CW));

            m_entityModelManager.prepare(vboManager());

            for (size_t i = 0; i < layout.size(); ++i) {
                const auto& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const auto& cell = row[k];
                                auto* modelRenderer = cellData(cell).modelRenderer;

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
            Renderer::Transformation transformation(projection, vm::view_matrix(vm::vec3f::neg_z(), vm::vec3f::pos_y()) *vm::translation_matrix(vm::vec3f(0.0f, 0.0f, -1.0f)));

            glAssert(glDisable(GL_DEPTH_TEST));
            glAssert(glFrontFace(GL_CCW));
            renderGroupTitleBackgrounds(layout, y, height);
            renderStrings(layout, y, height);
            glAssert(glFrontFace(GL_CW));
        }

        void EntityBrowserView::renderGroupTitleBackgrounds(Layout& layout, const float y, const float height) {
            using Vertex = Renderer::GLVertexTypes::P2::Vertex;
            std::vector<Vertex> vertices;

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

            vertexArray.prepare(vboManager());
            vertexArray.render(Renderer::PrimType::Quads);
        }

        void EntityBrowserView::renderStrings(Layout& layout, const float y, const float height) {
            using StringRendererMap = std::map<Renderer::FontDescriptor, Renderer::VertexArray>;
            StringRendererMap stringRenderers;

            { // create and upload all vertex arrays
                const auto stringVertices = collectStringVertices(layout, y, height);
                for (const auto& entry : stringVertices) {
                    const auto& fontDescriptor = entry.first;
                    const auto& vertices = entry.second;
                    stringRenderers[fontDescriptor] = Renderer::VertexArray::ref(vertices);
                    stringRenderers[fontDescriptor].prepare(vboManager());
                }
            }

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::ColoredTextShader);
            shader.set("Texture", 0);

            for (auto& entry : stringRenderers) {
                const auto& fontDescriptor = entry.first;
                auto& vertexArray = entry.second;

                auto& font = fontManager().font(fontDescriptor);
                font.activate();
                vertexArray.render(Renderer::PrimType::Quads);
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
                            kdl::skip_iterator(std::begin(quads), std::end(quads), 0, 2),
                            kdl::skip_iterator(std::begin(quads), std::end(quads), 1, 2),
                            kdl::skip_iterator(std::begin(textColor), std::end(textColor), 0, 0));
                        auto& allTitleVertices = stringVertices[defaultDescriptor];
                        allTitleVertices = kdl::vec_concat(std::move(allTitleVertices), titleVertices);
                    }

                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const auto& cell = row[k];
                                const auto titleBounds = cell.titleBounds();
                                const auto offset = vm::vec2f(titleBounds.left(), height - (titleBounds.top() - y) - titleBounds.height());

                                Renderer::TextureFont& font = fontManager().font(cellData(cell).fontDescriptor);
                                const auto quads = font.quads(cellData(cell).entityDefinition->name(), false, offset);
                                const auto titleVertices = TextVertex::toList(
                                    quads.size() / 2,
                                    kdl::skip_iterator(std::begin(quads), std::end(quads), 0, 2),
                                    kdl::skip_iterator(std::begin(quads), std::end(quads), 1, 2),
                                    kdl::skip_iterator(std::begin(textColor), std::end(textColor), 0, 0));
                                auto& allTitleVertices = stringVertices[cellData(cell).fontDescriptor];
                                allTitleVertices = kdl::vec_concat(std::move(allTitleVertices), titleVertices);
                            }
                        }
                    }
                }
            }

            return stringVertices;
        }

        vm::mat4x4f EntityBrowserView::itemTransformation(const Cell& cell, const float y, const float height) const {
            auto* definition = cellData(cell).entityDefinition;

            const auto offset = vm::vec3f(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y));
            const auto scaling = cell.scale();
            const auto& rotatedBounds = cellData(cell).bounds;
            const auto rotationOffset = vm::vec3f(0.0f, -rotatedBounds.min.y(), -rotatedBounds.min.z());
            const auto boundsCenter = vm::vec3f(definition->bounds().center());

            return (vm::translation_matrix(offset) *
                    vm::scaling_matrix(vm::vec3f::fill(scaling)) *
                    vm::translation_matrix(rotationOffset) *
                    vm::translation_matrix(boundsCenter) *
                    vm::rotation_matrix(m_rotation) *
                    vm::translation_matrix(-boundsCenter));
        }

        QString EntityBrowserView::tooltip(const Cell& cell) {
            return QString::fromStdString(cellData(cell).entityDefinition->name());
        }

        const EntityCellData& EntityBrowserView::cellData(const Cell& cell) const {
            QVariant any = cell.item();
            auto ptr = any.value<std::shared_ptr<EntityCellData>>();
            return *ptr;
        }
    }
}
