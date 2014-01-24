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
#include "Renderer/MeshRenderer.h"
#include "Renderer/RenderResources.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/TextureFont.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "View/ViewUtils.h"

#include <map>

namespace TrenchBroom {
    namespace View {
        EntityCellData::EntityCellData(Assets::PointEntityDefinition* i_entityDefinition, Renderer::MeshRenderer* i_modelRenderer, const Renderer::FontDescriptor& i_fontDescriptor, const BBox3f& i_bounds) :
        entityDefinition(i_entityDefinition),
        modelRenderer(i_modelRenderer),
        fontDescriptor(i_fontDescriptor),
        bounds(i_bounds) {}

        EntityBrowserView::EntityBrowserView(wxWindow* parent, wxWindowID windowId,
                                             wxScrollBar* scrollBar,
                                             Renderer::RenderResources& resources,
                                             Assets::EntityDefinitionManager& entityDefinitionManager,
                                             Assets::EntityModelManager& entityModelManager,
                                             Logger& logger) :
        CellView(parent, windowId, &resources.glAttribs().front(), resources.sharedContext(), scrollBar),
        m_resources(resources),
        m_entityDefinitionManager(entityDefinitionManager),
        m_entityModelManager(entityModelManager),
        m_logger(logger),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Assets::EntityDefinitionManager::Name),
        m_vbo(0xFFFF) {
            const Quatf hRotation = Quatf(Vec3f::PosZ, Math::radians(-30.0f));
            const Quatf vRotation = Quatf(Vec3f::PosY, Math::radians(20.0f));
            m_rotation = vRotation * hRotation;
        }
        
        EntityBrowserView::~EntityBrowserView() {
            clear();
        }
        
        void EntityBrowserView::setSortOrder(const Assets::EntityDefinitionManager::SortOrder sortOrder) {
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
            PreferenceManager& prefs = PreferenceManager::instance();
            
            const String fontName = prefs.get(Preferences::RendererFontName);
            int fontSize = prefs.get(Preferences::BrowserFontSize);
            assert(fontSize > 0);
            
            const Renderer::FontDescriptor font(fontName, static_cast<size_t>(fontSize));
            
            if (m_group) {
                Assets::EntityDefinitionGroups groups = m_entityDefinitionManager.groups(Assets::EntityDefinition::PointEntity, m_sortOrder);
                Assets::EntityDefinitionGroups::const_iterator groupIt, groupEnd;
                
                for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                    const String& groupName = groupIt->first;
                    const Assets::EntityDefinitionList& definitions = groupIt->second;
                    
                    layout.addGroup(groupName, fontSize + 2.0f);
                    
                    Assets::EntityDefinitionList::const_iterator defIt, defEnd;
                    for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                        Assets::PointEntityDefinition* definition = static_cast<Assets::PointEntityDefinition*>(*defIt);
                        addEntityToLayout(layout, definition, font);
                    }
                }
            } else {
                const Assets::EntityDefinitionList& definitions = m_entityDefinitionManager.definitions(Assets::EntityDefinition::PointEntity, m_sortOrder);
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
        
        wxDataObject* EntityBrowserView::dndData(const Layout::Group::Row::Cell& cell) {
            return new wxTextDataObject("entity:" + cell.item().entityDefinition->name());
        }

        void EntityBrowserView::addEntityToLayout(Layout& layout, Assets::PointEntityDefinition* definition, const Renderer::FontDescriptor& font) {
            if ((!m_hideUnused || definition->usageCount() > 0) &&
                (m_filterText.empty() || StringUtils::containsCaseInsensitive(definition->name(), m_filterText))) {
                
                Renderer::FontManager& fontManager =  m_resources.fontManager();
                const float maxCellWidth = layout.maxCellWidth();
                const Renderer::FontDescriptor actualFont = fontManager.selectFontSize(font, definition->name(), maxCellWidth, 5);
                const Vec2f actualSize = fontManager.font(actualFont).measure(definition->name());
                
                const Assets::ModelSpecification spec = definition->defaultModel();
                Assets::EntityModel* model = safeGetModel(m_entityModelManager, spec, m_logger);
                Renderer::MeshRenderer* modelRenderer = NULL;
                
                BBox3f rotatedBounds;
                if (model != NULL) {
                    const Vec3f center = model->bounds(spec.skinIndex, spec.frameIndex).center();
                    const Mat4x4f transformation = translationMatrix(center) * rotationMatrix(m_rotation) * translationMatrix(-center);
                    rotatedBounds = model->transformedBounds(spec.skinIndex, spec.frameIndex, transformation);
                    modelRenderer = m_entityModelManager.renderer(spec);
                } else {
                    rotatedBounds = static_cast<BBox3f>(definition->bounds());
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
                                Renderer::MeshRenderer* modelRenderer = cell.item().modelRenderer;
                                
                                if (modelRenderer == NULL) {
                                    const Mat4x4f itemTrans = itemTransformation(cell, y, height);
                                    CollectBoundsVertices<BoundsVertex> collect(itemTrans, definition->color(), vertices);
                                    eachBBoxEdge(definition->bounds(), collect);
                                }
                            }
                        }
                    }
                }
            }
            
            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(GL_LINES, vertices);

            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            vertexArray.prepare(m_vbo);

            setVboState.active();
            vertexArray.render();
        }

        void EntityBrowserView::renderModels(Layout& layout, const float y, const float height, Renderer::Transformation& transformation) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::EntityModelShader);
            shader.set("ApplyTinting", false);
            shader.set("Brightness", prefs.get(Preferences::Brightness));
            shader.set("GrayScale", false);
            
            glFrontFace(GL_CW);
            m_entityModelManager.activateVbo();

            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                Renderer::MeshRenderer* modelRenderer = cell.item().modelRenderer;
                                
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

            m_entityModelManager.deactivateVbo();
        }

        void EntityBrowserView::renderNames(Layout& layout, const float y, const float height, const Mat4x4f& projection) {
            Renderer::Transformation transformation = Renderer::Transformation(projection, viewMatrix(Vec3f::NegZ, Vec3f::PosY) * translationMatrix(Vec3f(0.0f, 0.0f, -1.0f)));
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            
            glDisable(GL_DEPTH_TEST);
            glFrontFace(GL_CCW);
            renderGroupTitleBackgrounds(layout, y, height);
            renderStrings(layout, y, height);
            glFrontFace(GL_CW);
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

            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(GL_QUADS, vertices);
            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::BrowserGroupShader);

            PreferenceManager& prefs = PreferenceManager::instance();
            shader.set("Color", prefs.get(Preferences::BrowserGroupBackgroundColor));
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            vertexArray.prepare(m_vbo);
            
            setVboState.mapped();
            vertexArray.render();
        }
        
        void EntityBrowserView::renderStrings(Layout& layout, const float y, const float height) {
            typedef std::map<Renderer::FontDescriptor, Renderer::VertexArray> StringRendererMap;
            StringRendererMap stringRenderers;
            
            Renderer::SetVboState activateVbo(m_vbo);
            activateVbo.mapped();

            { // create and upload all vertex arrays
                Renderer::SetVboState mapVbo(m_vbo);
                mapVbo.mapped();
                
                const StringMap stringVertices = collectStringVertices(layout, y, height);
                StringMap::const_iterator it, end;
                for (it = stringVertices.begin(), end = stringVertices.end(); it != end; ++it) {
                    const Renderer::FontDescriptor& descriptor = it->first;
                    const StringVertex::List& vertices = it->second;
                    stringRenderers[descriptor] = Renderer::VertexArray::ref(GL_QUADS, vertices);
                    stringRenderers[descriptor].prepare(m_vbo);
                }
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::TextShader);
            shader.set("Color", prefs.get(Preferences::BrowserTextColor));
            shader.set("Texture", 0);
            
            StringRendererMap::iterator it, end;
            for (it = stringRenderers.begin(), end = stringRenderers.end(); it != end; ++it) {
                const Renderer::FontDescriptor& descriptor = it->first;
                Renderer::VertexArray& vertexArray = it->second;
                
                Renderer::TextureFont& font = m_resources.fontManager().font(descriptor);
                font.activate();
                vertexArray.render();
                font.deactivate();
            }
        }
        
        EntityBrowserView::StringMap EntityBrowserView::collectStringVertices(Layout& layout, const float y, const float height) {
            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::FontDescriptor defaultDescriptor(prefs.get(Preferences::RendererFontName),
                                                       static_cast<size_t>(prefs.get(Preferences::BrowserFontSize)));
            
            StringMap stringVertices;
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const String& title = group.item();
                    if (!title.empty()) {
                        const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        const Vec2f offset(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());
                        
                        Renderer::TextureFont& font = m_resources.fontManager().font(defaultDescriptor);
                        const Vec2f::List quads = font.quads(title, false, offset);
                        const StringVertex::List titleVertices = StringVertex::fromLists(quads, quads, quads.size() / 2, 0, 2, 1, 2);
                        StringVertex::List& vertices = stringVertices[defaultDescriptor];
                        vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
                    }
                    
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds titleBounds = cell.titleBounds();
                                const Vec2f offset(titleBounds.left(), height - (titleBounds.top() - y) - titleBounds.height());
                                
                                Renderer::TextureFont& font = m_resources.fontManager().font(cell.item().fontDescriptor);
                                const Vec2f::List quads = font.quads(cell.item().entityDefinition->name(), false, offset);
                                const StringVertex::List titleVertices = StringVertex::fromLists(quads, quads, quads.size() / 2, 0, 2, 1, 2);
                                StringVertex::List& vertices = stringVertices[cell.item().fontDescriptor];
                                vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
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
        
    }
}
