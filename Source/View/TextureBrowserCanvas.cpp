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

#include "TextureBrowserCanvas.h"

#include "IO/FileManager.h"
#include "Model/MapDocument.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/SharedResources.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TextureRenderer.h"
#include "Renderer/TextureRendererManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Text/PathRenderer.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "View/TextureSelectedCommand.h"

#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        void TextureBrowserCanvas::addTextureToLayout(Layout& layout, Model::Texture* texture, const Renderer::Text::FontDescriptor& font) {
            if ((!m_hideUnused || texture->usageCount() > 0) && (m_filterText.empty() || Utility::containsString(texture->name(), m_filterText, false))) {
                Renderer::Text::FontDescriptor actualFont(font);
                Vec2f actualSize;
                
                Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();
                Renderer::Text::StringRendererPtr stringRenderer(NULL);
                StringRendererCache::iterator it = m_stringRendererCache.find(texture);
                if (it != m_stringRendererCache.end()) {
                    stringRenderer = it->second;
                    actualSize = Vec2f(stringRenderer->width(), stringRenderer->height());
                } else {
                    float cellSize = layout.fixedCellSize();
                    if  (cellSize > 0.0f)
                        actualSize = stringManager.selectFontSize(font, texture->name(), Vec2f(cellSize, static_cast<float>(font.size())), 5, actualFont);
                    else
                        actualSize = stringManager.measureString(font, texture->name());
                    stringRenderer = stringManager.stringRenderer(actualFont, texture->name());
                    m_stringRendererCache.insert(StringRendererCacheEntry(texture, stringRenderer));
                }
                
                Renderer::TextureRendererManager& textureRendererManager = m_documentViewHolder.document().sharedResources().textureRendererManager();
                Renderer::TextureRenderer& textureRenderer = textureRendererManager.renderer(texture);
                layout.addItem(TextureCellData(texture, &textureRenderer, stringRenderer), texture->width(), texture->height(), actualSize.x, font.size() + 2.0f);
            }
        }
        
        void TextureBrowserCanvas::doInitLayout(Layout& layout) {
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setFixedCellSize(CRBoth, 64.0f);
        }
        
        void TextureBrowserCanvas::doReloadLayout(Layout& layout) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Model::TextureManager& textureManager = m_documentViewHolder.document().textureManager();
            Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();

            String fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::TextureBrowserFontSize);
            
            assert(fontSize >= 0);
            Renderer::Text::FontDescriptor font(fontName, static_cast<unsigned int>(fontSize));
            IO::FileManager fileManager;
            
            if (m_group) {
                const Model::TextureCollectionList& collections = textureManager.collections();
                for (unsigned int i = 0; i < collections.size(); i++) {
                    Model::TextureCollection* collection = collections[i];
                    if (m_group) {
                        String name = fileManager.pathComponents(collection->name()).back();
                        layout.addGroup(TextureGroupData(collection, stringManager.stringRenderer(font, name)), fontSize + 2.0f);
                    }
                    
                    Model::TextureList textures = collection->textures(m_sortOrder);
                    for (unsigned int j = 0; j < textures.size(); j++)
                        addTextureToLayout(layout, textures[j], font);
                }
            } else {
                Model::TextureList textures = textureManager.textures(m_sortOrder);
                for (unsigned int i = 0; i < textures.size(); i++)
                    addTextureToLayout(layout, textures[i], font);
            }
        }
        
        void TextureBrowserCanvas::doRender(Layout& layout, float y, float height) {
            Renderer::ShaderManager& shaderManager = m_documentViewHolder.document().sharedResources().shaderManager();
            Renderer::ShaderProgram& textureProgram = shaderManager.shaderProgram(Renderer::Shaders::TextureBrowserShader);
            Renderer::ShaderProgram& textureBorderProgram = shaderManager.shaderProgram(Renderer::Shaders::TextureBrowserBorderShader);
            Renderer::ShaderProgram& textProgram = shaderManager.shaderProgram(Renderer::Shaders::TextShader);
            
            float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
            float viewTop       = static_cast<float>(GetClientRect().GetBottom());
            float viewRight     = static_cast<float>(GetClientRect().GetRight());
            float viewBottom    = static_cast<float>(GetClientRect().GetTop());
            
            Mat4f projection;
            projection.setOrtho(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom);
            
            Mat4f view;
            view.setView(Vec3f::NegZ, Vec3f::PosY);
            view.translate(Vec3f(0.0f, 0.0f, 0.1f));
            Renderer::Transformation transformation(projection * view, true);

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::Text::StringManager& stringManager = m_documentViewHolder.document().sharedResources().stringManager();

            // render borders
            textureBorderProgram.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                
                                bool selected = cell.item().texture == m_selectedTexture;
                                bool inUse = cell.item().texture->usageCount() > 0;
                                bool overridden = cell.item().texture->overridden();
                                
                                if (selected || inUse || overridden) {
                                    if (selected)
                                        textureBorderProgram.setUniformVariable("Color", prefs.getColor(Preferences::SelectedTextureColor));
                                    else if (inUse)
                                        textureBorderProgram.setUniformVariable("Color", prefs.getColor(Preferences::UsedTextureColor));
                                    else
                                        textureBorderProgram.setUniformVariable("Color", prefs.getColor(Preferences::OverriddenTextureColor));
                                    
                                    glBegin(GL_QUADS);
                                    glVertex2f(cell.itemBounds().left() - 1.5f, height - (cell.itemBounds().top() - 1.5f - y));
                                    glVertex2f(cell.itemBounds().left() - 1.5f, height - (cell.itemBounds().bottom() + 1.5f - y));
                                    glVertex2f(cell.itemBounds().right() + 1.5f, height - (cell.itemBounds().bottom() + 1.5f - y));
                                    glVertex2f(cell.itemBounds().right() + 1.5f, height - (cell.itemBounds().top() - 1.5f - y));
                                    glEnd();
                                }
                            }
                        }
                    }
                }
            }
            textureBorderProgram.deactivate();
            
            // render textures
            textureProgram.activate();
            textureProgram.setUniformVariable("ApplyTinting", false);
            textureProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                textureProgram.setUniformVariable("GrayScale", cell.item().texture->overridden());
                                textureProgram.setUniformVariable("Texture", 0);
                                cell.item().textureRenderer->activate();
                                glBegin(GL_QUADS);
                                glTexCoord2f(0.0f, 0.0f);
                                glVertex2f(cell.itemBounds().left(), height - (cell.itemBounds().top() - y));
                                glTexCoord2f(0.0f, 1.0f);
                                glVertex2f(cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y));
                                glTexCoord2f(1.0f, 1.0f);
                                glVertex2f(cell.itemBounds().right(), height - (cell.itemBounds().bottom() - y));
                                glTexCoord2f(1.0f, 0.0f);
                                glVertex2f(cell.itemBounds().right(), height - (cell.itemBounds().top() - y));
                                glEnd();
                                cell.item().textureRenderer->deactivate();
                            }
                        }
                    }
                }
            }
            textureProgram.deactivate();

            // render texture captions
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

                                if (cell.item().texture == m_selectedTexture)
                                    textProgram.setUniformVariable("Color", prefs.getColor(Preferences::SelectedTextureColor));
                                else if (cell.item().texture->usageCount() > 0)
                                    textProgram.setUniformVariable("Color", prefs.getColor(Preferences::UsedTextureColor));
                                else if (cell.item().texture->overridden())
                                    textProgram.setUniformVariable("Color", prefs.getColor(Preferences::OverriddenTextureColor));
                                else
                                    textProgram.setUniformVariable("Color", prefs.getColor(Preferences::BrowserTextureColor));

                                Mat4f translation;
                                translation.translate(Vec3f(cell.titleBounds().left(), height - (cell.titleBounds().top() - y) - cell.titleBounds().height() + 2.0f, 0.0f));
                                
                                Renderer::ApplyMatrix applyTranslation(transformation, translation);
                                Renderer::Text::StringRendererPtr stringRenderer = cell.item().stringRenderer;
                                stringRenderer->render();
                            }
                        }
                    }
                }
            }
            stringManager.deactivate();
            textProgram.deactivate();
            
            // render group title background
            textureBorderProgram.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    if (group.item().textureCollection != NULL) {
                        textureBorderProgram.setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupBackgroundColor));
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
            textureBorderProgram.deactivate();
            
            // render group captions
            textProgram.activate();
            stringManager.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    if (group.item().textureCollection != NULL) {
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

        void TextureBrowserCanvas::handleLeftClick(Layout& layout, float x, float y) {
            const Layout::Group::Row::Cell* result = NULL;
            if (layout.cellAt(x, y, &result)) {
                if (!result->item().texture->overridden()) {
                    m_selectedTexture = result->item().texture;
                    Refresh();
                    
                    if (m_documentViewHolder.valid()) {
                        TextureSelectedCommand command;
                        command.setTexture(m_selectedTexture);
                        command.SetEventObject(this);
                        command.SetId(GetId());
                        ProcessEvent(command);
                    }
                }
            }
        }

        TextureBrowserCanvas::TextureBrowserCanvas(wxWindow* parent, wxWindowID windowId, wxScrollBar* scrollBar, DocumentViewHolder& documentViewHolder) :
        CellLayoutGLCanvas(parent, windowId, documentViewHolder.document().sharedResources().attribs(), documentViewHolder.document().sharedResources().sharedContext(), scrollBar),
        m_documentViewHolder(documentViewHolder),
        m_selectedTexture(NULL),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Model::TextureSortOrder::Name) {}

        TextureBrowserCanvas::~TextureBrowserCanvas() {
            clear();
            m_stringRendererCache.clear();
            m_selectedTexture = NULL;
        }
    }
}

