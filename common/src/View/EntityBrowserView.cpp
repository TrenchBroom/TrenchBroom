/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "View/MapFrame.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <map>

namespace TrenchBroom {
    namespace View {
        EntityCellData::EntityCellData(Assets::PointEntityDefinition* i_entityDefinition, EntityRenderer* i_modelRenderer, const Renderer::FontDescriptor& i_fontDescriptor, const BBox3f& i_bounds) :
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
        CellView(parent, contextManager, buildAttribs(), scrollBar),
        m_entityDefinitionManager(entityDefinitionManager),
        m_entityModelManager(entityModelManager),
        m_logger(logger),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Assets::EntityDefinition::Name) {
            const Quatf hRotation = Quatf(Vec3f::PosZ, Math::radians(-30.0f));
            const Quatf vRotation = Quatf(Vec3f::PosY, Math::radians(20.0f));
            m_rotation = vRotation * hRotation;
        }
        
        EntityBrowserView::~EntityBrowserView() {
            clear();
        }
        
        void EntityBrowserView::setSortOrder(const Assets::EntityDefinition::SortOrder sortOrder) {
            if (sortOrder == m_sortOrder)
                return;
            m_sortOrder = sortOrder;
            reload();
            Refresh();
        }
        
        void EntityBrowserView::setGroup(const bool group) {
            if (group == m_group)
                return;
            m_group = group;
            reload();
            Refresh();
        }
        
        void EntityBrowserView::setHideUnused(const bool hideUnused) {
            if (hideUnused == m_hideUnused)
                return;
            m_hideUnused = hideUnused;
            reload();
            Refresh();
        }
        
        void EntityBrowserView::setFilterText(const String& filterText) {
            if (filterText == m_filterText)
                return;
            m_filterText = filterText;
            reload();
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
            const IO::Path& fontPath = pref(Preferences::RendererFontPath());
            int fontSize = pref(Preferences::BrowserFontSize);
            assert(fontSize > 0);
            
            const Renderer::FontDescriptor font(fontPath, static_cast<size_t>(fontSize));
            
            if (m_group) {
                const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
                Assets::EntityDefinitionGroup::List::const_iterator groupIt, groupEnd;
                
                for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                    const Assets::EntityDefinitionGroup& group = *groupIt;
                    const Assets::EntityDefinitionList& definitions = group.definitions(Assets::EntityDefinition::Type_PointEntity, m_sortOrder);
                    
                    if (!definitions.empty()) {
                        const String displayName = group.displayName();
                        layout.addGroup(displayName, fontSize + 2.0f);
                        
                        Assets::EntityDefinitionList::const_iterator defIt, defEnd;
                        for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                            Assets::PointEntityDefinition* definition = static_cast<Assets::PointEntityDefinition*>(*defIt);
                            addEntityToLayout(layout, definition, font);
                        }
                    }
                }
            } else {
                const Assets::EntityDefinitionList& definitions = m_entityDefinitionManager.definitions(Assets::EntityDefinition::Type_PointEntity, m_sortOrder);
                Assets::EntityDefinitionList::const_iterator it, end;
                for (it = definitions.begin(), end = definitions.end(); it != end; ++it) {
                    Assets::PointEntityDefinition* definition = static_cast<Assets::PointEntityDefinition*>(*it);
                    addEntityToLayout(layout, definition, font);
                }
            }
        }
        
        bool EntityBrowserView::dndEnabled() {
            return true;
        }
        
        void EntityBrowserView::dndWillStart() {
            MapFrame* mapFrame = findMapFrame(this);
            assert(mapFrame != NULL);
            mapFrame->setToolBoxDropTarget();
        }
        
        void EntityBrowserView::dndDidEnd() {
            MapFrame* mapFrame = findMapFrame(this);
            assert(mapFrame != NULL);
            mapFrame->clearDropTarget();
        }

        wxString EntityBrowserView::dndData(const Layout::Group::Row::Cell& cell) {
            static const String prefix("entity:");
            const String name = cell.item().entityDefinition->name();
            return wxString(prefix + name);
        }

        void EntityBrowserView::addEntityToLayout(Layout& layout, Assets::PointEntityDefinition* definition, const Renderer::FontDescriptor& font) {
            if ((!m_hideUnused || definition->usageCount() > 0) &&
                (m_filterText.empty() || StringUtils::containsCaseInsensitive(definition->name(), m_filterText))) {
                
                const float maxCellWidth = layout.maxCellWidth();
                const Renderer::FontDescriptor actualFont = fontManager().selectFontSize(font, definition->name(), maxCellWidth, 5);
                const Vec2f actualSize = fontManager().font(actualFont).measure(definition->name());
                
                const Assets::ModelSpecification spec = definition->defaultModel();
                Assets::EntityModel* model = safeGetModel(m_entityModelManager, spec, m_logger);
                EntityRenderer* modelRenderer = NULL;
                
                BBox3f rotatedBounds;
                if (model != NULL) {
                    const Vec3f center = model->bounds(spec.skinIndex, spec.frameIndex).center();
                    const Mat4x4f transformation = translationMatrix(center) * rotationMatrix(m_rotation) * translationMatrix(-center);
                    rotatedBounds = model->transformedBounds(spec.skinIndex, spec.frameIndex, transformation);
                    modelRenderer = m_entityModelManager.renderer(spec);
                } else {
                    rotatedBounds = BBox3f(definition->bounds());
                    const Vec3f center = rotatedBounds.center();
                    rotatedBounds = rotateBBox(rotatedBounds, m_rotation, center);
                }
                
                const Vec3f size = rotatedBounds.size();
                layout.addItem(EntityCellData(definition, modelRenderer, actualFont, rotatedBounds),
                               size.y(),
                               size.z(),
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

            const Mat4x4f projection = orthoMatrix(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
            const Mat4x4f view = viewMatrix(Vec3f::NegX, Vec3f::PosZ) * translationMatrix(Vec3f(256.0f, 0.0f, 0.0f));
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
            const Mat4x4f& transformation;
            const Color& color;
            typename Vertex::List& vertices;
            
            CollectBoundsVertices(const Mat4x4f& i_transformation, const Color& i_color, typename Vertex::List& i_vertices) :
            transformation(i_transformation),
            color(i_color),
            vertices(i_vertices) {}
            
            void operator()(const Vec3f& v1, const Vec3f& v2) {
                vertices.push_back(Vertex(transformation * v1, color));
                vertices.push_back(Vertex(transformation * v2, color));
            }
        };
        
        void EntityBrowserView::renderBounds(Layout& layout, const float y, const float height) {
            typedef Renderer::VertexSpecs::P3C4::Vertex BoundsVertex;
            BoundsVertex::List vertices;
            
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                Assets::PointEntityDefinition* definition = cell.item().entityDefinition;
                                EntityRenderer* modelRenderer = cell.item().modelRenderer;
                                
                                if (modelRenderer == NULL) {
                                    const Mat4x4f itemTrans = itemTransformation(cell, y, height);
                                    const Color& color = definition->color();
                                    CollectBoundsVertices<BoundsVertex> collect(itemTrans, color, vertices);
                                    eachBBoxEdge(definition->bounds(), collect);
                                }
                            }
                        }
                    }
                }
            }
            
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(vertices);

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
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                EntityRenderer* modelRenderer = cell.item().modelRenderer;
                                
                                if (modelRenderer != NULL) {
                                    const Mat4x4f itemTrans = itemTransformation(cell, y, height);
                                    Renderer::MultiplyModelMatrix multMatrix(transformation, itemTrans);
                                    modelRenderer->render();
                                }
                            }
                        }
                    }
                }
            }
        }

        void EntityBrowserView::renderNames(Layout& layout, const float y, const float height, const Mat4x4f& projection) {
            Renderer::Transformation transformation = Renderer::Transformation(projection, viewMatrix(Vec3f::NegZ, Vec3f::PosY) * translationMatrix(Vec3f(0.0f, 0.0f, -1.0f)));
            
            Renderer::ActivateVbo activate(vertexVbo());
            
            glAssert(glDisable(GL_DEPTH_TEST));
            glAssert(glFrontFace(GL_CCW));
            renderGroupTitleBackgrounds(layout, y, height);
            renderStrings(layout, y, height);
            glAssert(glFrontFace(GL_CW));
        }

        void EntityBrowserView::renderGroupTitleBackgrounds(Layout& layout, const float y, const float height) {
            typedef Renderer::VertexSpecs::P2::Vertex Vertex;
            Vertex::List vertices;
            
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                    vertices.push_back(Vertex(Vec2f(titleBounds.left(), height - (titleBounds.top() - y))));
                    vertices.push_back(Vertex(Vec2f(titleBounds.left(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(Vec2f(titleBounds.right(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(Vec2f(titleBounds.right(), height - (titleBounds.top() - y))));
                }
            }

            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(vertices);
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::BrowserGroupShader);
            shader.set("Color", pref(Preferences::BrowserGroupBackgroundColor));
            
            Renderer::ActivateVbo activate(vertexVbo());
            vertexArray.prepare(vertexVbo());
            vertexArray.render(GL_QUADS);
        }
        
        void EntityBrowserView::renderStrings(Layout& layout, const float y, const float height) {
            typedef std::map<Renderer::FontDescriptor, Renderer::VertexArray> StringRendererMap;
            StringRendererMap stringRenderers;
            
            { // create and upload all vertex arrays
                Renderer::ActivateVbo activate(vertexVbo());
                
                const StringMap stringVertices = collectStringVertices(layout, y, height);
                StringMap::const_iterator it, end;
                for (it = stringVertices.begin(), end = stringVertices.end(); it != end; ++it) {
                    const Renderer::FontDescriptor& descriptor = it->first;
                    const TextVertex::List& vertices = it->second;
                    stringRenderers[descriptor] = Renderer::VertexArray::ref(vertices);
                    stringRenderers[descriptor].prepare(vertexVbo());
                }
            }
            
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::ColoredTextShader);
            shader.set("Texture", 0);
            
            StringRendererMap::iterator it, end;
            for (it = stringRenderers.begin(), end = stringRenderers.end(); it != end; ++it) {
                const Renderer::FontDescriptor& descriptor = it->first;
                Renderer::VertexArray& vertexArray = it->second;
                
                Renderer::TextureFont& font = fontManager().font(descriptor);
                font.activate();
                vertexArray.render(GL_QUADS);
                font.deactivate();
            }
        }
        
        EntityBrowserView::StringMap EntityBrowserView::collectStringVertices(Layout& layout, const float y, const float height) {
            Renderer::FontDescriptor defaultDescriptor(pref(Preferences::RendererFontPath()),
                                                       static_cast<size_t>(pref(Preferences::BrowserFontSize)));
            
            const Color::List textColor(1, pref(Preferences::BrowserTextColor));
            
            StringMap stringVertices;
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const String& title = group.item();
                    if (!title.empty()) {
                        const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        const Vec2f offset(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());
                        
                        Renderer::TextureFont& font = fontManager().font(defaultDescriptor);
                        const Vec2f::List quads = font.quads(title, false, offset);
                        const TextVertex::List titleVertices = TextVertex::fromLists(quads, quads, textColor, quads.size() / 2, 0, 2, 1, 2, 0, 0);
                        VectorUtils::append(stringVertices[defaultDescriptor], titleVertices);
                    }
                    
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds titleBounds = cell.titleBounds();
                                const Vec2f offset(titleBounds.left(), height - (titleBounds.top() - y) - titleBounds.height());
                                
                                Renderer::TextureFont& font = fontManager().font(cell.item().fontDescriptor);
                                const Vec2f::List quads = font.quads(cell.item().entityDefinition->name(), false, offset);
                                const TextVertex::List titleVertices = TextVertex::fromLists(quads, quads, textColor, quads.size() / 2, 0, 2, 1, 2, 0, 0);
                                VectorUtils::append(stringVertices[cell.item().fontDescriptor], titleVertices);
                            }
                        }
                    }
                }
            }
            
            return stringVertices;
        }
        
        Mat4x4f EntityBrowserView::itemTransformation(const Layout::Group::Row::Cell& cell, const float y, const float height) const {
            Assets::PointEntityDefinition* definition = cell.item().entityDefinition;
            
            const Vec3f offset = Vec3f(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y));
            const float scaling = cell.scale();
            const BBox3f& rotatedBounds = cell.item().bounds;
            const Vec3f rotationOffset = Vec3f(0.0f, -rotatedBounds.min.y(), -rotatedBounds.min.z());
            const Vec3f center = definition->bounds().center();
            
            return (translationMatrix(offset) *
                    scalingMatrix<4>(scaling) *
                    translationMatrix(rotationOffset) *
                    translationMatrix(center) *
                    rotationMatrix(m_rotation) *
                    translationMatrix(-center));
        }
        
        wxString EntityBrowserView::tooltip(const Layout::Group::Row::Cell& cell) {
            return cell.item().entityDefinition->name();
        }
    }
}
