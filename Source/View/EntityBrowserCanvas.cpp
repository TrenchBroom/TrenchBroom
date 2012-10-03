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
#include "Renderer/EntityRenderer.h"
#include "Renderer/EntityRendererManager.h"
#include "Renderer/PushMatrix.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/SharedResources.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Text/PathRenderer.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

namespace TrenchBroom {
    namespace View {
        void EntityBrowserCanvas::createShaders() {
            assert(!m_shadersCreated);
            
            Utility::Console& console = m_documentViewHolder.view().console();
            IO::FileManager fileManager;
            String resourceDirectory = fileManager.resourceDirectory();
            
            
            m_boundsVertexShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "Edge.vertsh"), GL_VERTEX_SHADER, console));
            m_boundsFragmentShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "Edge.fragsh"), GL_FRAGMENT_SHADER, console));
            m_boundsShaderProgram = Renderer::ShaderProgramPtr(new Renderer::ShaderProgram("entity browser bounds shader program", console));
            m_boundsShaderProgram->attachShader(*m_boundsVertexShader.get());
            m_boundsShaderProgram->attachShader(*m_boundsFragmentShader.get());
            
            
            m_modelVertexShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "EntityModel.vertsh"), GL_VERTEX_SHADER, console));
            m_modelFragmentShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "EntityModel.fragsh"), GL_FRAGMENT_SHADER, console));
            m_modelShaderProgram = Renderer::ShaderProgramPtr(new Renderer::ShaderProgram("entity browser model shader program", console));
            m_modelShaderProgram->attachShader(*m_modelVertexShader.get());
            m_modelShaderProgram->attachShader(*m_modelFragmentShader.get());
            
            m_textVertexShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "Text.vertsh"), GL_VERTEX_SHADER, console));
            m_textFragmentShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "Text.fragsh"), GL_FRAGMENT_SHADER, console));
            m_textShaderProgram = Renderer::ShaderProgramPtr(new Renderer::ShaderProgram("entity browser text shader program", console));
            m_textShaderProgram->attachShader(*m_textVertexShader.get());
            m_textShaderProgram->attachShader(*m_textFragmentShader.get());
            
            m_shadersCreated = true;
        }

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
                    String definitionName = definition->name();
                    String shortName;
                    size_t uscoreIndex = definitionName.find_first_of('_');
                    if (uscoreIndex != String::npos)
                        shortName = Utility::capitalize(definitionName.substr(uscoreIndex + 1));
                    else
                        shortName = definitionName;
                    
                    float cellSize = layout.fixedCellSize();
                    if  (cellSize > 0.0f)
                        actualSize = stringManager.selectFontSize(font, shortName, Vec2f(cellSize, static_cast<float>(font.size())), 5, actualFont);
                    else
                        actualSize = stringManager.measureString(font, shortName);
                    stringRenderer = stringManager.stringRenderer(actualFont, shortName);
                    m_stringRendererCache.insert(StringRendererCacheEntry(definition, stringRenderer));
                }
                
                Renderer::EntityRendererManager& entityRendererManager = m_documentViewHolder.document().sharedResources().entityRendererManager();
                const StringList& mods = m_documentViewHolder.document().mods();
                Renderer::EntityRenderer* entityRenderer = entityRendererManager.entityRenderer(*definition, mods);
                
                BBox bounds = entityRenderer != NULL ? entityRenderer->bounds() : definition->bounds();
                bounds = bounds.boundsAfterRotation(m_rotation);
                Vec3f size = bounds.size();
                
                layout.addItem(EntityCellData(definition, entityRenderer, stringRenderer), size.x, size.z, actualSize.x, font.size() + 2.0f);
            }
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
            Renderer::Text::FontDescriptor font(fontName, fontSize);
            IO::FileManager fileManager;
            
            const Model::EntityDefinitionList& definitions = definitionManager.definitions(Model::EntityDefinition::PointEntity, m_sortOrder);
            if (m_group) {
                typedef std::map<String,  Model::EntityDefinitionList> GroupedDefinitions;
                GroupedDefinitions groupedDefinitions;
                
                for (unsigned int i = 0; i < definitions.size(); i++) {
                    Model::EntityDefinition& definition = *definitions[i];
                    String definitionName = definition.name();
                    String groupName;
                    size_t uscoreIndex = definitionName.find_first_of('_');
                    if (uscoreIndex != String::npos)
                        groupName = Utility::capitalize(definitionName.substr(0, uscoreIndex));
                    else
                        groupName = "Misc";
                    groupedDefinitions[groupName].push_back(&definition);
                }

                GroupedDefinitions::iterator it, end;
                for (it = groupedDefinitions.begin(), end = groupedDefinitions.end(); it != end; ++it) {
                    const String& groupName = it->first;
                    const Model::EntityDefinitionList& groupDefinitions = it->second;
                    
                    layout.addGroup(EntityGroupData(groupName, stringManager.stringRenderer(font, groupName)), fontSize + 2.0f);
                    for (unsigned int i = 0; i < groupDefinitions.size(); i++)
                        addEntityToLayout(layout, static_cast<Model::PointEntityDefinition*>(groupDefinitions[i]), font);
                }
            } else {
                for (unsigned int i = 0; i < definitions.size(); i++)
                    addEntityToLayout(layout, static_cast<Model::PointEntityDefinition*>(definitions[i]), font);
            }
        }

        void EntityBrowserCanvas::doRender(Layout& layout, float y, float height) {
            glEnable(GL_DEPTH_TEST);
            
            if (!m_shadersCreated)
                createShaders();

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
            Renderer::EntityRendererManager& entityRendererManager = m_documentViewHolder.document().sharedResources().entityRendererManager();
            Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();

            // render bounds
            m_boundsShaderProgram->activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                Model::PointEntityDefinition* definition = cell.item().entityDefinition;
                                Renderer::EntityRenderer* entityRenderer = cell.item().entityRenderer;
                                if (entityRenderer != NULL)
                                    continue;
                                
                                const BBox& bounds = definition->bounds();
                                BBox rotBounds = bounds.boundsAfterRotation(m_rotation);
                                
                                Renderer::PushMatrix pushMatrix(transformation);
                                Mat4f itemMatrix = pushMatrix.matrix();
                                itemMatrix.translate(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y));
                                itemMatrix.scale(cell.scale());
                                itemMatrix.translate(0.0f, -rotBounds.min.y, -rotBounds.min.z);
                                itemMatrix.translate(bounds.center());
                                itemMatrix.rotate(m_rotation);
                                itemMatrix.translate(-1.0f * bounds.center());
                                pushMatrix.load(itemMatrix);

                                Color entityColor;
                                if (definition != NULL) {
                                    entityColor = definition->color();
                                    entityColor.w = prefs.getColor(Preferences::EntityBoundsColor).w;
                                } else {
                                    entityColor = prefs.getColor(Preferences::EntityBoundsColor);
                                }
                                m_boundsShaderProgram->setUniformVariable("Color", entityColor);
                                
                                Vec3f::List vertices;
                                bounds.vertices(vertices);
                                
                                glBegin(GL_LINES);
                                for (unsigned int i = 0; i < vertices.size(); i++)
                                    Renderer::glVertexV3f(vertices[i]);
                                glEnd();
                            }
                        }
                    }
                }
            }
            m_boundsShaderProgram->deactivate();
            
            // render models
            entityRendererManager.activate();
            m_modelShaderProgram->activate();
            m_modelShaderProgram->setUniformVariable("ApplyTinting", false);
            m_modelShaderProgram->setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                Renderer::EntityRenderer* entityRenderer = cell.item().entityRenderer;
                                if (entityRenderer == NULL)
                                    continue;
                                
                                BBox bounds = entityRenderer->bounds().boundsAfterRotation(m_rotation);

                                Renderer::PushMatrix pushMatrix(transformation);
                                Mat4f itemMatrix = pushMatrix.matrix();
                                itemMatrix.translate(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y));
                                itemMatrix.scale(cell.scale());
                                itemMatrix.translate(0.0f, -bounds.min.y, -bounds.min.z);
                                itemMatrix.translate(bounds.center());
                                itemMatrix.rotate(m_rotation);
                                itemMatrix.translate(-1.0f * bounds.center());
                                pushMatrix.load(itemMatrix);
                                
                                entityRenderer->render(*m_modelShaderProgram);
                            }
                        }
                    }
                }
            }
            m_modelShaderProgram->deactivate();
            entityRendererManager.deactivate();

            glDisable(GL_DEPTH_TEST);
            view.setView(Vec3f::NegZ, Vec3f::PosY);
            view.translate(Vec3f(0.0f, 0.0f, -1.0f));
            transformation = Renderer::Transformation(projection * view, true);

            // render entity captions
            m_textShaderProgram->activate();
            stringManager.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                
                                m_textShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::BrowserTextureColor));
                                
                                Renderer::PushMatrix matrix(transformation);
                                Mat4f translate = matrix.matrix();
                                translate.translate(cell.titleBounds().left(), height - (cell.titleBounds().top() - y) - cell.titleBounds().height() + 2.0f, 0.0f);
                                matrix.load(translate);
                                
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
                        m_textShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupBackgroundColor));
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
                        Renderer::PushMatrix matrix(transformation);
                        Mat4f translate = matrix.matrix();
                        translate.translate(Vec3f(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height() + 4.0f, 0.0f));
                        matrix.load(translate);
                        
                        m_textShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupTextColor));
                        Renderer::Text::StringRendererPtr stringRenderer = group.item().stringRenderer;
                        stringRenderer->render();
                    }
                }
            }
            stringManager.deactivate();
            m_textShaderProgram->deactivate();
        }
        
        void EntityBrowserCanvas::handleLeftClick(Layout& layout, float x, float y) {
        }

        EntityBrowserCanvas::EntityBrowserCanvas(wxWindow* parent, wxWindowID windowId, wxScrollBar* scrollBar, DocumentViewHolder& documentViewHolder) :
        CellLayoutGLCanvas(parent, windowId, documentViewHolder.document().sharedResources().attribs(), documentViewHolder.document().sharedResources().sharedContext(), scrollBar),
        m_documentViewHolder(documentViewHolder),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Model::EntityDefinitionManager::Name),
        m_shadersCreated(false) {
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
