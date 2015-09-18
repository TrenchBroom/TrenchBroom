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

#ifndef TrenchBroom_TextureBrowserView
#define TrenchBroom_TextureBrowserView

#include "StringUtils.h"
#include "Assets/TextureManager.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/Vbo.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"
#include "View/CellView.h"

#include <map>

class wxScrollBar;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
        class TextureCollection;
    }
    
    namespace View {
        class GLContextManager;
        typedef String TextureGroupData;
        
        class TextureCellData {
        public:
            Assets::Texture* texture;
            Renderer::FontDescriptor fontDescriptor;
            
            TextureCellData(Assets::Texture* i_texture, const Renderer::FontDescriptor& i_fontDescriptor);
        };

        class TextureBrowserView : public CellView<TextureCellData, TextureGroupData> {
        private:
            typedef Renderer::VertexSpecs::P2T2C4::Vertex TextVertex;
            typedef std::map<Renderer::FontDescriptor, TextVertex::List> StringMap;

            Assets::TextureManager& m_textureManager;

            bool m_group;
            bool m_hideUnused;
            Assets::TextureManager::SortOrder m_sortOrder;
            String m_filterText;
            
            Assets::Texture* m_selectedTexture;
        public:
            TextureBrowserView(wxWindow* parent,
                               wxScrollBar* scrollBar,
                               GLContextManager& contextManager,
                               Assets::TextureManager& textureManager);
            ~TextureBrowserView();

            void setSortOrder(Assets::TextureManager::SortOrder sortOrder);
            void setGroup(bool group);
            void setHideUnused(bool hideUnused);
            void setFilterText(const String& filterText);

            Assets::Texture* selectedTexture() const;
            void setSelectedTexture(Assets::Texture* selectedTexture);
        private:
            void doInitLayout(Layout& layout);
            void doReloadLayout(Layout& layout);
            void addTextureToLayout(Layout& layout, Assets::Texture* texture, const Renderer::FontDescriptor& font);
            
            void doClear();
            void doRender(Layout& layout, float y, float height);
            bool doShouldRenderFocusIndicator() const;
            
            void renderBounds(Layout& layout, float y, float height);
            const Color& textureColor(const Assets::Texture& texture) const;
            void renderTextures(Layout& layout, float y, float height);
            void renderNames(Layout& layout, float y, float height);
            void renderGroupTitleBackgrounds(Layout& layout, float y, float height);
            void renderStrings(Layout& layout, float y, float height);
            StringMap collectStringVertices(Layout& layout, float y, float height);
            
            void doLeftClick(Layout& layout, float x, float y);
            wxString tooltip(const Layout::Group::Row::Cell& cell);
        };
    }
}

#endif /* defined(TrenchBroom_TextureBrowserView) */
