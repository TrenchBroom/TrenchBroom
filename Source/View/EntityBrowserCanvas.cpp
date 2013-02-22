/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityBrowserCanvas.h"

#include "IO/FileManager.h"
#include "Model/MapDocument.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/SharedResources.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Text/PathRenderer.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

namespace TrenchBroom {
    namespace View {
        void EntityBrowserCanvas::addEntityToLayout(Layout& layout, Model::PointEntityDefinition* definition, const Renderer::Text::FontDescriptor& font) {
            if ((!m_hideUnused || definition->usageCount() > 0) && (m_filterText.empty() || Utility::containsString(definition->name(), m_filterText, false))) {
                Renderer::Text::FontDescriptor actualFont(font);
                Vec2f actualSize;
                
                Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();
                Renderer::Text::StringRendererPtr stringRenderer(NULL);
                StringRendererCache::iterator it = m_stringRendererCache.find(definition);
                if (it != m_stringRendererCache.end()) {
                    stringRenderer = it->second;
                    actualSize = Vec2f(stringRenderer->width(), stringRenderer->height());
                } else {
                    String shortName = definition->shortName();
                    
                    float cellSize = layout.fixedCellSize();
                    if  (cellSize > 0.0f)
                        actualSize = stringManager.selectFontSizeWithEllipses(font, shortName, Vec2f(cellSize, static_cast<float>(font.size())), 9, actualFont, shortName);
                    else
                        actualSize = stringManager.measureString(font, shortName);
                    stringRenderer = stringManager.stringRenderer(actualFont, shortName);
                    m_stringRendererCache.insert(StringRendererCacheEntry(definition, stringRenderer));
                }
                
                Renderer::EntityModelRendererManager& modelRendererManager = m_documentViewHolder.document().sharedResources().modelRendererManager();
                const StringList& searchPaths = m_documentViewHolder.document().searchPaths();
                Renderer::EntityModelRenderer* modelRenderer = modelRendererManager.modelRenderer(*definition, searchPaths);

                BBox rotatedBounds;
                if (modelRenderer != NULL) {
                    const Vec3f& center = modelRenderer->center();
                    Mat4f transformation;
                    transformation.setIdentity();
                    transformation.translate(center);
                    transformation.rotate(m_rotation);
                    transformation.translate(-1.0f * center);
                    rotatedBounds = modelRenderer->boundsAfterTransformation(transformation);
                } else {
                    rotatedBounds = definition->bounds();
                    const Vec3f center = rotatedBounds.center();
                    rotatedBounds.rotate(m_rotation, center);
                }
                
                Vec3f size = rotatedBounds.size();
                layout.addItem(EntityCellData(definition, modelRenderer, stringRenderer, rotatedBounds), size.y, size.z, actualSize.x, font.size() + 2.0f);
            }
        }
        
        void EntityBrowserCanvas::renderEntityBounds(Renderer::Transformation& transformation, Renderer::ShaderProgram& boundsProgram, const Model::PointEntityDefinition& definition, const BBox& rotatedBounds, const Vec3f& offset, float scale) {
            const BBox& bounds = definition.bounds();
            
            Mat4f itemMatrix;
            itemMatrix.translate(offset.x, offset.y, offset.z);
            itemMatrix.scale(scale);
            itemMatrix.translate(0.0f, -rotatedBounds.min.y, -rotatedBounds.min.z);
            itemMatrix.translate(bounds.center());
            itemMatrix.rotate(m_rotation);
            itemMatrix.translate(-1.0f * bounds.center());
            Renderer::ApplyMatrix applyItemMatrix(transformation, itemMatrix);
            
            boundsProgram.setUniformVariable("Color", definition.color());
            
            Vec3f::List vertices;
            bounds.vertices(vertices);
            
            glBegin(GL_LINES);
            for (unsigned int i = 0; i < vertices.size(); i++)
                Renderer::glVertexV3f(vertices[i]);
            glEnd();
        }
        
        void EntityBrowserCanvas::renderEntityModel(Renderer::Transformation& transformation, Renderer::ShaderProgram& entityModelProgram, Renderer::EntityModelRenderer& renderer, const BBox& rotatedBounds, const Vec3f& offset, float scale) {
            const Vec3f& rotationCenter = renderer.center();
            
            Mat4f itemMatrix;
            itemMatrix.translate(offset.x, offset.y, offset.z);
            itemMatrix.scale(scale);
            itemMatrix.translate(0.0f, -rotatedBounds.min.y, -rotatedBounds.min.z);
            itemMatrix.translate(rotationCenter);
            itemMatrix.rotate(m_rotation);
            itemMatrix.translate(-1.0f * rotationCenter);
            Renderer::ApplyMatrix applyItemMatrix(transformation, itemMatrix);
            
            renderer.render(entityModelProgram);
        }

        void EntityBrowserCanvas::doInitLayout(Layout& layout) {
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setFixedCellSize(CRBoth, 64.0f);
            layout.setScaleCellsUp(true, 1.5f);
        }
        
        void EntityBrowserCanvas::doReloadLayout(Layout& layout) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Model::EntityDefinitionManager& definitionManager = m_documentViewHolder.document().definitionManager();
            Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();
            
            String fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::EntityBrowserFontSize);
            assert(fontSize >= 0);
            
            Renderer::Text::FontDescriptor font(fontName, static_cast<unsigned int>(fontSize));
            IO::FileManager fileManager;
            
            if (m_group) {
                Model::EntityDefinitionManager::EntityDefinitionGroups groups = definitionManager.groups(Model::EntityDefinition::PointEntity, m_sortOrder);
                Model::EntityDefinitionManager::EntityDefinitionGroups::const_iterator groupIt, groupEnd;
                
                for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                    const String& groupName = groupIt->first;
                    const Model::EntityDefinitionList& definitions = groupIt->second;
                    
                    layout.addGroup(EntityGroupData(groupName, stringManager.stringRenderer(font, groupName)), fontSize + 2.0f);
                    
                    Model::EntityDefinitionList::const_iterator defIt, defEnd;
                    for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                        Model::PointEntityDefinition* definition = static_cast<Model::PointEntityDefinition*>(*defIt);
                        addEntityToLayout(layout, definition, font);
                    }
                }
            } else {
                const Model::EntityDefinitionList& definitions = definitionManager.definitions(Model::EntityDefinition::PointEntity, m_sortOrder);
                for (unsigned int i = 0; i < definitions.size(); i++)
                    addEntityToLayout(layout, static_cast<Model::PointEntityDefinition*>(definitions[i]), font);
            }
        }

        void EntityBrowserCanvas::doClear() {
            m_stringRendererCache.clear();
        }

        void EntityBrowserCanvas::doRender(Layout& layout, float y, float height) {
            Renderer::ShaderManager& shaderManager = m_documentViewHolder.document().sharedResources().shaderManager();
            Renderer::ShaderProgram& boundsProgram = shaderManager.shaderProgram(Renderer::Shaders::EdgeShader);
            Renderer::ShaderProgram& entityModelProgram = shaderManager.shaderProgram(Renderer::Shaders::EntityModelShader);
            Renderer::ShaderProgram& textProgram = shaderManager.shaderProgram(Renderer::Shaders::TextShader);
            
            glEnable(GL_DEPTH_TEST);

            float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
            float viewTop       = static_cast<float>(GetClientRect().GetBottom());
            float viewRight     = static_cast<float>(GetClientRect().GetRight());
            float viewBottom    = static_cast<float>(GetClientRect().GetTop());
            
            Mat4f projection;
            projection.setOrtho(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
            
            Mat4f view;
            view.setView(Vec3f::NegX, Vec3f::PosZ);
            view.translate(Vec3f(256.0f, 0.0f, 0.0f));
            Renderer::Transformation transformation(projection * view, true);

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::EntityModelRendererManager& modelRendererManager = m_documentViewHolder.document().sharedResources().modelRendererManager();
            Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();

            // render bounds
            boundsProgram.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                Model::PointEntityDefinition* definition = cell.item().entityDefinition;
                                Renderer::EntityModelRenderer* modelRenderer = cell.item().modelRenderer;
                                if (modelRenderer == NULL)
                                    renderEntityBounds(transformation, boundsProgram, *definition, cell.item().bounds, Vec3f(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y)), cell.scale());
                            }
                        }
                    }
                }
            }
            boundsProgram.deactivate();
            
            // render models
            modelRendererManager.activate();
            entityModelProgram.activate();
            entityModelProgram.setUniformVariable("ApplyTinting", false);
            entityModelProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
            entityModelProgram.setUniformVariable("GrayScale", false);
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                Renderer::EntityModelRenderer* modelRenderer = cell.item().modelRenderer;
                                if (modelRenderer != NULL)
                                    renderEntityModel(transformation, entityModelProgram, *modelRenderer, cell.item().bounds, Vec3f(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y)), cell.scale());
                            }
                        }
                    }
                }
            }
            entityModelProgram.deactivate();
            modelRendererManager.deactivate();

            glDisable(GL_DEPTH_TEST);
            view.setView(Vec3f::NegZ, Vec3f::PosY);
            view.translate(Vec3f(0.0f, 0.0f, -1.0f));
            transformation = Renderer::Transformation(projection * view, true);

            // render entity captions
            textProgram.activate();
            stringManager.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                
                                textProgram.setUniformVariable("Color", prefs.getColor(Preferences::BrowserTextureColor));
                                
                                Mat4f translation;
                                translation.translate(cell.titleBounds().left(), height - (cell.titleBounds().top() - y) - cell.titleBounds().height() + 2.0f, 0.0f);
                                Renderer::ApplyMatrix applyTranslation(transformation, translation);
                                
                                Renderer::Text::StringRendererPtr stringRenderer = cell.item().stringRenderer;
                                stringRenderer->render();
                            }
                        }
                    }
                }
            }
            stringManager.deactivate();
            
            // render group title background
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    if (!group.item().groupName.empty()) {
                        textProgram.setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupBackgroundColor));
                        LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        glBegin(GL_QUADS);
                        glVertex2f(titleBounds.left(), height - (titleBounds.top() - y));
                        glVertex2f(titleBounds.left(), height - (titleBounds.bottom() - y));
                        glVertex2f(titleBounds.right(), height - (titleBounds.bottom() - y));
                        glVertex2f(titleBounds.right(), height - (titleBounds.top() - y));
                        glEnd();
                    }
                }
            }
            
            // render group captions
            stringManager.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    if (!group.item().groupName.empty()) {
                        LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        Mat4f translation;
                        translation.translate(Vec3f(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height() + 4.0f, 0.0f));
                        Renderer::ApplyMatrix applyTranslation(transformation, translation);
                        
                        textProgram.setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupTextColor));
                        Renderer::Text::StringRendererPtr stringRenderer = group.item().stringRenderer;
                        stringRenderer->render();
                    }
                }
            }
            stringManager.deactivate();
            textProgram.deactivate();
        }
        
        bool EntityBrowserCanvas::dndEnabled() {
            return true;
        }
        
        wxImage* EntityBrowserCanvas::dndImage(const Layout::Group::Row::Cell& cell) {
            if (!SetCurrent(*glContext()))
                return NULL;

            const LayoutBounds& bounds = cell.itemBounds();
            
            unsigned int width = static_cast<unsigned int>(bounds.width());
            unsigned int height = static_cast<unsigned int>(bounds.height());
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::EntityModelRendererManager& modelRendererManager = m_documentViewHolder.document().sharedResources().modelRendererManager();
            
            Renderer::ShaderManager& shaderManager = m_documentViewHolder.document().sharedResources().shaderManager();
            Renderer::ShaderProgram& boundsProgram = shaderManager.shaderProgram(Renderer::Shaders::EdgeShader);
            Renderer::ShaderProgram& entityModelProgram = shaderManager.shaderProgram(Renderer::Shaders::EntityModelShader);

            m_offscreenRenderer.setDimensions(width, height);
            m_offscreenRenderer.preRender();
            
            float viewLeft      = 0.0f;
            float viewTop       = 0.0f;
            float viewRight     = bounds.width();
            float viewBottom    = bounds.height();
            
            Mat4f projection;
            projection.setOrtho(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
            
            Mat4f view;
            view.setView(Vec3f::NegX, Vec3f::PosZ);
            view.translate(Vec3f(256.0f, 0.0f, 0.0f));
            Renderer::Transformation transformation(projection * view, true);
            
            glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            
            Renderer::EntityModelRenderer* modelRenderer = cell.item().modelRenderer;
            if (modelRenderer == NULL) {
                boundsProgram.activate();
                Model::PointEntityDefinition* definition = cell.item().entityDefinition;
                renderEntityBounds(transformation, boundsProgram, *definition, cell.item().bounds, Vec3f::Null, cell.scale());
                boundsProgram.deactivate();
            } else {
                modelRendererManager.activate();
                entityModelProgram.activate();
                entityModelProgram.setUniformVariable("ApplyTinting", false);
                entityModelProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                renderEntityModel(transformation, entityModelProgram, *modelRenderer, cell.item().bounds, Vec3f::Null, cell.scale());
                entityModelProgram.deactivate();
                modelRendererManager.deactivate();
            }
            
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            
            wxImage* image = m_offscreenRenderer.getImage();
            m_offscreenRenderer.postRender();
            
            return image;
        }
        
        wxDataObject* EntityBrowserCanvas::dndData(const Layout::Group::Row::Cell& cell) {
            return new wxTextDataObject("entity:" + cell.item().entityDefinition->name());
        }
        
        EntityBrowserCanvas::EntityBrowserCanvas(wxWindow* parent, wxWindowID windowId, wxScrollBar* scrollBar, DocumentViewHolder& documentViewHolder) :
        CellLayoutGLCanvas(parent, windowId, documentViewHolder.document().sharedResources().attribs(), documentViewHolder.document().sharedResources().sharedContext(), scrollBar),
        m_documentViewHolder(documentViewHolder),
        m_offscreenRenderer(GL::glCapabilities()),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Model::EntityDefinitionManager::Name) {
            Quat hRotation = Quat(radians(-30.0f), Vec3f::PosZ);
            Quat vRotation = Quat(radians(20.0f), Vec3f::PosY);
            m_rotation = vRotation * hRotation;
        }
        
        EntityBrowserCanvas::~EntityBrowserCanvas() {
            clear();
            m_stringRendererCache.clear();
        }
    }
}
