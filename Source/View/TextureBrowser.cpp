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

#include "TextureBrowser.h"

#include "Renderer/RenderTypes.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        void TextureBrowser::doInitLayout(Layout& layout) {
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setFixedCellWidth(64.0f);
        }
        
        void TextureBrowser::addTextureToLayout(Layout& layout, Model::Texture* texture, const Renderer::Text::FontDescriptor& font) {
            if ((!m_hideUnused || texture->usageCount() > 0) && (m_filterText.empty() || Utility::containsString(texture->name(), m_filterText, false))) {
                Renderer::Text::FontDescriptor actualFont(font);
                Vec2f actualSize;
                
                float cellWidth = layout.fixedCellWidth();
                if  (cellWidth > 0.0f)
                    actualSize = m_stringManager.selectFontSize(font, texture->name(), Vec2f(cellWidth, static_cast<float>(font.size())), 5, actualFont);
                else
                    actualSize = m_stringManager.measureString(font, texture->name());
                
                layout.addItem(TextureCellData(texture, actualFont), texture->width(), texture->height(), actualSize.x, actualSize.y + 2.0f);
            }
        }

        void TextureBrowser::doReloadLayout(Layout& layout) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            String fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Renderer::Text::FontDescriptor font(fontName, fontSize);

            if (m_group) {
                const Model::TextureCollectionList& collections = m_textureManager.collections();
                for (unsigned int i = 0; i < collections.size(); i++) {
                    Model::TextureCollection* collection = collections[i];
                    if (m_group)
                        layout.addGroup(collection, fontSize + 2.0f);
                    
                    Model::TextureList textures = collection->textures(m_sortOrder);
                    for (unsigned int j = 0; j < textures.size(); j++)
                        addTextureToLayout(layout, textures[j], font);
                }
            } else {
                Model::TextureList textures = m_textureManager.textures(m_sortOrder);
                for (unsigned int i = 0; i < textures.size(); i++)
                    addTextureToLayout(layout, textures[i], font);
            }
        }
        
        void TextureBrowser::doRender(Layout& layout, const wxRect& rect) {
            float y = static_cast<float>(rect.GetY());
            float height = static_cast<float>(rect.GetHeight());
            
            unsigned int visibleGroupCount = 0;
            unsigned int visibleCellCount = 0;
            
            // count visible groups and cells
            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(rect.y, height)) {
                    visibleGroupCount++;
                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(rect.y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                if (cell.cellBounds().intersectsY(y, height))
                                    visibleCellCount++;
                            }
                        }
                    }
                }
            }
            
            Renderer::VertexArrayPtr textureVertexArray(new Renderer::VertexArray(*m_vbo, GL_TRIANGLES, visibleCellCount * 6,
                                                                                  Renderer::VertexAttribute(2, GL_FLOAT, Renderer::VertexAttribute::Position),
                                                                                  Renderer::VertexAttribute(2, GL_FLOAT, Renderer::VertexAttribute::TexCoord0)));
            Renderer::VertexArrayPtr groupVertexArray;
            if (m_group && visibleGroupCount > 0)
                groupVertexArray = Renderer::VertexArrayPtr(new Renderer::VertexArray(*m_vbo, GL_TRIANGLES, visibleGroupCount * 6,
                                                                                      Renderer::VertexAttribute(2, GL_FLOAT, Renderer::VertexAttribute::Position)));
            
            
        }

        TextureBrowser::TextureBrowser(wxWindow* parent, wxGLContext* sharedContext, Utility::Console& console, Model::TextureManager& textureManager) :
        CellLayoutGLCanvas(parent, sharedContext),
        m_console(console),
        m_textureManager(textureManager),
        m_stringManager(console),
        m_vbo(NULL),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Model::TextureSortOrder::Name) {
            m_vbo = new Renderer::Vbo(GL_ARRAY_BUFFER, 0xFFFF);
        }

        TextureBrowser::~TextureBrowser() {
            if (m_vbo != NULL) {
                delete m_vbo;
                m_vbo = NULL;
            }
        }
    }
}