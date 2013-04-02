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
#include "Renderer/Text/FontManager.h"
#include "Renderer/Text/TexturedFont.h"
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
                Renderer::Text::FontManager& fontManager =  m_documentViewHolder.document().sharedResources().fontManager();
                const float maxCellWidth = layout.maxCellWidth();
                const Renderer::Text::FontDescriptor actualFont = fontManager.selectFontSize(font, texture->name(), maxCellWidth, 5);
                const Vec2f actualSize = fontManager.font(actualFont)->measure(texture->name());
                
                Renderer::TextureRendererManager& textureRendererManager = m_documentViewHolder.document().sharedResources().textureRendererManager();
                Renderer::TextureRenderer& textureRenderer = textureRendererManager.renderer(texture);
                layout.addItem(TextureCellData(texture, &textureRenderer, actualFont), texture->width(), texture->height(), actualSize.x, font.size() + 2.0f);
            }
        }
        
        void TextureBrowserCanvas::doInitLayout(Layout& layout) {
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setCellWidth(64.0f, 64.0f);
            layout.setCellHeight(64.0f, 128.0f);
        }
        
        void TextureBrowserCanvas::doReloadLayout(Layout& layout) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Model::TextureManager& textureManager = m_documentViewHolder.document().textureManager();

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
                        layout.addGroup(collection, fontSize + 2.0f);
                    }
                    
                    Model::TextureList textures = collection->textures(m_sortOrder);
                    for (unsigned int j = 0; j < textures.size(); j++)
                        addTextureToLayout(layout, textures[j], font);
                }
            } else {
                layout.addGroup(NULL, fontSize + 2.0f);
                Model::TextureList textures = textureManager.textures(m_sortOrder);
                for (unsigned int i = 0; i < textures.size(); i++)
                    addTextureToLayout(layout, textures[i], font);
            }
        }
        
        void TextureBrowserCanvas::doClear() {
        }

        void TextureBrowserCanvas::doRender(Layout& layout, float y, float height) {
            if (m_vbo == NULL)
                m_vbo = new Renderer::Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            
            Renderer::ShaderManager& shaderManager = m_documentViewHolder.document().sharedResources().shaderManager();
            Renderer::Text::FontManager& fontManager = m_documentViewHolder.document().sharedResources().fontManager();
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::Text::FontDescriptor defaultDescriptor(prefs.getString(Preferences::RendererFontName),
                                                             static_cast<unsigned int>(prefs.getInt(Preferences::TextureBrowserFontSize)));

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

            size_t visibleGroupCount = 0;
            size_t visibleItemCount = 0;

            typedef std::map<Renderer::Text::FontDescriptor, Vec2f::List> StringMap;
            StringMap stringVertices;
            
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    visibleGroupCount++;
                    
                    Model::TextureCollection* collection = group.item();
                    if (collection != NULL && !collection->name().empty()) {
                        const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        const Vec2f offset(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());
                        
                        Renderer::Text::TexturedFont* font = fontManager.font(defaultDescriptor);
                        Vec2f::List titleVertices = font->quads(collection->name(), false, offset);
                        Vec2f::List& vertices = stringVertices[defaultDescriptor];
                        vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
                    }
                    
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                visibleItemCount++;

                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds titleBounds = cell.titleBounds();
                                const Vec2f offset(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());
                                
                                Renderer::Text::TexturedFont* font = fontManager.font(cell.item().fontDescriptor);
                                Vec2f::List titleVertices = font->quads(cell.item().texture->name(), false, offset);
                                Vec2f::List& vertices = stringVertices[cell.item().fontDescriptor];
                                vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
                            }
                        }
                    }
                }
            }
            
            if (visibleItemCount > 0) { // render borders
                unsigned int vertexCount = static_cast<unsigned int>(4 * visibleItemCount);
                Renderer::VertexArray vertexArray(*m_vbo, GL_QUADS, vertexCount,
                                                  Renderer::Attribute::position2f(),
                                                  Renderer::Attribute::color4f());
                
                Renderer::SetVboState mapVbo(*m_vbo, Renderer::Vbo::VboMapped);
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
                                        const Color& color = selected ? prefs.getColor(Preferences::SelectedTextureColor) : (inUse ? prefs.getColor(Preferences::UsedTextureColor) : prefs.getColor(Preferences::OverriddenTextureColor));
                                        
                                        vertexArray.addAttribute(Vec2f(cell.itemBounds().left() - 1.5f, height - (cell.itemBounds().top() - 1.5f - y)));
                                        vertexArray.addAttribute(color);
                                        vertexArray.addAttribute(Vec2f(cell.itemBounds().left() - 1.5f, height - (cell.itemBounds().bottom() + 1.5f - y)));
                                        vertexArray.addAttribute(color);
                                        vertexArray.addAttribute(Vec2f(cell.itemBounds().right() + 1.5f, height - (cell.itemBounds().bottom() + 1.5f - y)));
                                        vertexArray.addAttribute(color);
                                        vertexArray.addAttribute(Vec2f(cell.itemBounds().right() + 1.5f, height - (cell.itemBounds().top() - 1.5f - y)));
                                        vertexArray.addAttribute(color);
                                    }
                                }
                            }
                        }
                    }
                }

                Renderer::SetVboState activateVbo(*m_vbo, Renderer::Vbo::VboActive);
                Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::TextureBrowserBorderShader);
                vertexArray.render();
            }
            
            { // render textures
                Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::TextureBrowserShader);
                shader.currentShader().setUniformVariable("ApplyTinting", false);
                shader.currentShader().setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                for (unsigned int i = 0; i < layout.size(); i++) {
                    const Layout::Group& group = layout[i];
                    if (group.intersectsY(y, height)) {
                        for (unsigned int j = 0; j < group.size(); j++) {
                            const Layout::Group::Row& row = group[j];
                            if (row.intersectsY(y, height)) {
                                for (unsigned int k = 0; k < row.size(); k++) {
                                    const Layout::Group::Row::Cell& cell = row[k];
                                    shader.currentShader().setUniformVariable("GrayScale", cell.item().texture->overridden());
                                    shader.currentShader().setUniformVariable("Texture", 0);
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
            }
            
            if (visibleGroupCount > 0) { // render group title background
                unsigned int vertexCount = static_cast<unsigned int>(4 * visibleGroupCount);
                Renderer::VertexArray vertexArray(*m_vbo, GL_QUADS, vertexCount,
                                                  Renderer::Attribute::position2f());

                Renderer::SetVboState mapVbo(*m_vbo, Renderer::Vbo::VboMapped);
                for (unsigned int i = 0; i < layout.size(); i++) {
                    const Layout::Group& group = layout[i];
                    if (group.intersectsY(y, height)) {
                        if (group.item() != NULL) {
                            LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                            vertexArray.addAttribute(Vec2f(titleBounds.left(), height - (titleBounds.top() - y)));
                            vertexArray.addAttribute(Vec2f(titleBounds.left(), height - (titleBounds.bottom() - y)));
                            vertexArray.addAttribute(Vec2f(titleBounds.right(), height - (titleBounds.bottom() - y)));
                            vertexArray.addAttribute(Vec2f(titleBounds.right(), height - (titleBounds.top() - y)));
                        }
                    }
                }
                
                Renderer::SetVboState activateVbo(*m_vbo, Renderer::Vbo::VboActive);
                Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::BrowserGroupShader);
                shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupBackgroundColor));
                vertexArray.render();
            }
            
            if (!stringVertices.empty()) { // render strings
                StringMap::iterator it, end;
                for (it = stringVertices.begin(), end = stringVertices.end(); it != end; ++it) {
                    const Renderer::Text::FontDescriptor& descriptor = it->first;
                    Renderer::Text::TexturedFont* font = fontManager.font(descriptor);
                    const Vec2f::List& vertices = it->second;
                    
                    unsigned int vertexCount = static_cast<unsigned int>(vertices.size() / 2);
                    Renderer::VertexArray vertexArray(*m_vbo, GL_QUADS, vertexCount,
                                                      Renderer::Attribute::position2f(),
                                                      Renderer::Attribute::texCoord02f(), 0);
                    
                    Renderer::SetVboState mapVbo(*m_vbo, Renderer::Vbo::VboMapped);
                    vertexArray.addAttributes(vertices);
                    
                    Renderer::SetVboState activateVbo(*m_vbo, Renderer::Vbo::VboActive);
                    Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::TextShader);
                    shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::BrowserTextColor));
                    shader.currentShader().setUniformVariable("Texture", 0);
                    
                    font->activate();
                    vertexArray.render();
                    font->deactivate();
                }
            }
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
        m_sortOrder(Model::TextureSortOrder::Name),
        m_vbo() {}

        TextureBrowserCanvas::~TextureBrowserCanvas() {
            clear();
            m_selectedTexture = NULL;
            delete m_vbo;
            m_vbo = NULL;
        }
    }
}

