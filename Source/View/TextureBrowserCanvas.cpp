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
#include "Renderer/PushMatrix.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Text/PathRenderer.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

#include <cassert>

using namespace TrenchBroom::Math;

DEFINE_EVENT_TYPE(EVT_TEXTURE_SELECTED_EVENT)

namespace TrenchBroom {
    namespace View {
        void TextureBrowserCanvas::createShaders() {
            assert(!m_shadersCreated);
            
            Utility::Console& console = m_documentViewHolder.view().console();
            IO::FileManager fileManager;
            String resourceDirectory = fileManager.resourceDirectory();
            
            
            m_textureBorderVertexShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "TextureBrowserBorder.vertsh"), GL_VERTEX_SHADER, console));
            m_textureBorderFragmentShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "TextureBrowserBorder.fragsh"), GL_FRAGMENT_SHADER, console));
            m_textureBorderShaderProgram = Renderer::ShaderProgramPtr(new Renderer::ShaderProgram("texture browser border shader program", console));
            m_textureBorderShaderProgram->attachShader(*m_textureBorderVertexShader.get());
            m_textureBorderShaderProgram->attachShader(*m_textureBorderFragmentShader.get());

            
            m_textureVertexShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "TextureBrowser.vertsh"), GL_VERTEX_SHADER, console));
            m_textureFragmentShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "TextureBrowser.fragsh"), GL_FRAGMENT_SHADER, console));
            m_textureShaderProgram = Renderer::ShaderProgramPtr(new Renderer::ShaderProgram("texture browser shader program", console));
            m_textureShaderProgram->attachShader(*m_textureVertexShader.get());
            m_textureShaderProgram->attachShader(*m_textureFragmentShader.get());
            
            m_textVertexShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "Text.vertsh"), GL_VERTEX_SHADER, console));
            m_textFragmentShader = Renderer::ShaderPtr(new Renderer::Shader(fileManager.appendPath(resourceDirectory, "Text.fragsh"), GL_FRAGMENT_SHADER, console));
            m_textShaderProgram = Renderer::ShaderProgramPtr(new Renderer::ShaderProgram("text shader program", console));
            m_textShaderProgram->attachShader(*m_textVertexShader.get());
            m_textShaderProgram->attachShader(*m_textFragmentShader.get());
            
            m_shadersCreated = true;
        }

        void TextureBrowserCanvas::addTextureToLayout(Layout& layout, Model::Texture* texture, const Renderer::Text::FontDescriptor& font) {
            if ((!m_hideUnused || texture->usageCount() > 0) && (m_filterText.empty() || Utility::containsString(texture->name(), m_filterText, false))) {
                Renderer::Text::FontDescriptor actualFont(font);
                Vec2f actualSize;
                
                Renderer::Text::StringRendererPtr stringRenderer(NULL);
                StringRendererCache::iterator it = m_stringRendererCache.find(texture);
                if (it != m_stringRendererCache.end()) {
                    stringRenderer = it->second;
                    actualSize = Vec2f(stringRenderer->width(), stringRenderer->height());
                } else {
                    float cellSize = layout.fixedCellSize();
                    if  (cellSize > 0.0f)
                        actualSize = m_stringManager.selectFontSize(font, texture->name(), Vec2f(cellSize, static_cast<float>(font.size())), 5, actualFont);
                    else
                        actualSize = m_stringManager.measureString(font, texture->name());
                    stringRenderer = m_stringManager.stringRenderer(actualFont, texture->name());
                    m_stringRendererCache.insert(std::pair<Model::Texture*, Renderer::Text::StringRendererPtr>(texture, stringRenderer));
                }
                layout.addItem(TextureCellData(texture, stringRenderer), texture->width(), texture->height(), actualSize.x, font.size() + 2.0f);
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
            String fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Renderer::Text::FontDescriptor font(fontName, fontSize);
            IO::FileManager fileManager;
            
            if (m_group) {
                const Model::TextureCollectionList& collections = textureManager.collections();
                for (unsigned int i = 0; i < collections.size(); i++) {
                    Model::TextureCollection* collection = collections[i];
                    if (m_group) {
                        String name = fileManager.pathComponents(collection->name()).back();
                        layout.addGroup(TextureGroupData(collection, m_stringManager.stringRenderer(font, name)), fontSize + 2.0f);
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
        
        void TextureBrowserCanvas::doRender(Layout& layout, Renderer::Transformation& transformation, float y, float height) {
            if (!m_shadersCreated)
                createShaders();

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            // render borders
            m_textureBorderShaderProgram->activate();
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
                                        m_textureBorderShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::SelectedTextureColor));
                                    else if (inUse)
                                        m_textureBorderShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::UsedTextureColor));
                                    else
                                        m_textureBorderShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::OverriddenTextureColor));
                                    
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
            m_textureBorderShaderProgram->deactivate();
            
            // render textures
            m_textureShaderProgram->activate();
            m_textureShaderProgram->setUniformVariable("ApplyTinting", false);
            m_textureShaderProgram->setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                m_textureShaderProgram->setUniformVariable("GrayScale", cell.item().texture->overridden());
                                m_textureShaderProgram->setUniformVariable("Texture", 0);
                                cell.item().texture->activate();
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
                                cell.item().texture->deactivate();
                            }
                        }
                    }
                }
            }
            m_textureShaderProgram->deactivate();

            // render texture captions
            m_textShaderProgram->activate();
            m_stringManager.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];

                                if (cell.item().texture == m_selectedTexture)
                                    m_textShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::SelectedTextureColor));
                                else if (cell.item().texture->usageCount() > 0)
                                    m_textShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::UsedTextureColor));
                                else if (cell.item().texture->overridden())
                                    m_textShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::OverriddenTextureColor));
                                else
                                    m_textShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::BrowserTextureColor));

                                Renderer::PushMatrix matrix(transformation);
                                Mat4f translate = matrix.matrix();
                                translate.translate(Vec3f(cell.titleBounds().left(), height - (cell.titleBounds().top() - y) - cell.titleBounds().height() + 2.0f, 0.0f));
                                matrix.load(translate);
                                
                                Renderer::Text::StringRendererPtr stringRenderer = cell.item().stringRenderer;
                                stringRenderer->render();
                            }
                        }
                    }
                }
            }
            m_stringManager.deactivate();
            m_textShaderProgram->deactivate();
            
            // render group title background
            m_textureBorderShaderProgram->activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    if (group.item().textureCollection != NULL) {
                        m_textureBorderShaderProgram->setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupBackgroundColor));
                        LayoutBounds titleBounds = group.titleBoundsForVisibleRect(y, height);
                        glBegin(GL_QUADS);
                        glVertex2f(titleBounds.left(), height - (titleBounds.top() - y));
                        glVertex2f(titleBounds.left(), height - (titleBounds.bottom() - y));
                        glVertex2f(titleBounds.right(), height - (titleBounds.bottom() - y));
                        glVertex2f(titleBounds.right(), height - (titleBounds.top() - y));
                        glEnd();
                    }
                }
            }
            m_textureBorderShaderProgram->deactivate();
            
            // render group captions
            m_textShaderProgram->activate();
            m_stringManager.activate();
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    if (group.item().textureCollection != NULL) {
                        LayoutBounds titleBounds = group.titleBoundsForVisibleRect(y, height);
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
            m_stringManager.deactivate();
            m_textShaderProgram->deactivate();
            
        }

        void TextureBrowserCanvas::handleLeftClick(Layout& layout, float x, float y) {
            const Layout::Group::Row::Cell* result = NULL;
            if (layout.cellAt(x, y, &result)) {
                m_selectedTexture = result->item().texture;
                Refresh();

                if (m_documentViewHolder.valid()) {
                    TextureSelectedCommand command(m_selectedTexture);
                    command.SetEventObject(this);
                    command.SetId(GetId());
                    ProcessEvent(command);
                }
            }
        }

        TextureBrowserCanvas::TextureBrowserCanvas(wxWindow* parent, wxWindowID windowId, wxGLContext* sharedContext, wxScrollBar* scrollBar, DocumentViewHolder& documentViewHolder) :
        CellLayoutGLCanvas(parent, windowId, sharedContext, scrollBar),
        m_documentViewHolder(documentViewHolder),
        m_selectedTexture(NULL),
        m_stringManager(m_documentViewHolder.view().console()),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Model::TextureSortOrder::Name),
        m_shadersCreated(false) {}

        TextureBrowserCanvas::~TextureBrowserCanvas() {
            clear();
            m_stringRendererCache.clear();
            m_selectedTexture = NULL;
        }
    }
}

