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
#include "Renderer/SharedResources.h"
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
                    float cellSize = layout.fixedCellSize();
                    if  (cellSize > 0.0f)
                        actualSize = stringManager.selectFontSize(font, definition->name(), Vec2f(cellSize, static_cast<float>(font.size())), 5, actualFont);
                    else
                        actualSize = stringManager.measureString(font, definition->name());
                    stringRenderer = stringManager.stringRenderer(actualFont, definition->name());
                    m_stringRendererCache.insert(StringRendererCacheEntry(definition, stringRenderer));
                }
                
                Renderer::EntityRendererManager& entityRendererManager = m_documentViewHolder.document().sharedResources().entityRendererManager();
                const StringList& mods = m_documentViewHolder.document().mods();
                Renderer::EntityRenderer* entityRenderer = entityRendererManager.entityRenderer(*definition, mods);
                
                BBox bounds = definition->bounds();
                if (entityRenderer != NULL)
                    bounds.mergeWith(entityRenderer->bounds());
                bounds = bounds.boundsAfterRotation(m_rotation);
                Vec3f size = bounds.size();
                
                layout.addItem(EntityCellData(definition, entityRenderer, stringRenderer), size.y, size.z, actualSize.x, font.size() + 2.0f);
            }
        }
        
        void EntityBrowserCanvas::doInitLayout(Layout& layout) {
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setFixedCellSize(CRBoth, 128.0f);
            layout.setScaleCellsUp(true);
        }
        
        void EntityBrowserCanvas::doReloadLayout(Layout& layout) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Model::EntityDefinitionManager& definitionManager = m_documentViewHolder.document().definitionManager();
            Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();
            
            String fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
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
            view.translate(Vec3f(-256.0f, 0.0f, 0.0f));
            Renderer::Transformation transformation(projection * view, true);

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::EntityRendererManager& entityRendererManager = m_documentViewHolder.document().sharedResources().entityRendererManager();
            Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();

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
                                Model::PointEntityDefinition* definition = cell.item().entityDefinition;
                                Renderer::EntityRenderer* entityRenderer = cell.item().entityRenderer;
                                
                                BBox bounds = definition->bounds();
                                if (entityRenderer != NULL)
                                    bounds.mergeWith(entityRenderer->bounds());
                                bounds = bounds.boundsAfterRotation(m_rotation);

                                Renderer::PushMatrix pushMatrix(transformation);
                                Mat4f itemMatrix = pushMatrix.matrix();
//                                itemMatrix.rotate(m_rotation);
                                itemMatrix.translate(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().top() - y));
                                itemMatrix.scale(cell.scale());
                                itemMatrix.translate(0.0f, -bounds.min.y, -bounds.min.z);
                                pushMatrix.load(itemMatrix);
                                
                                if (entityRenderer != NULL)
                                    entityRenderer->render(*m_modelShaderProgram);
                            }
                        }
                    }
                }
            }
            m_modelShaderProgram->deactivate();
            entityRendererManager.deactivate();
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
            m_rotation = Quat(radians(115.0f), Vec3f::PosZ);
        }
        
        EntityBrowserCanvas::~EntityBrowserCanvas() {
            clear();
            m_stringRendererCache.clear();
        }
    }
}
