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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__TextureBrowserView__
#define __TrenchBroom__TextureBrowserView__

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
    
    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        typedef String TextureGroupData;
        
        class TextureCellData {
        public:
            Assets::Texture* texture;
            Renderer::FontDescriptor fontDescriptor;
            
            TextureCellData(Assets::Texture* i_texture, const Renderer::FontDescriptor& i_fontDescriptor);
        };

        class TextureBrowserView : public CellView<TextureCellData, TextureGroupData> {
        private:
            typedef Renderer::VertexSpecs::P2T2::Vertex StringVertex;
            typedef std::map<Renderer::FontDescriptor, StringVertex::List> StringMap;

            Renderer::RenderResources& m_resources;
            Assets::TextureManager& m_textureManager;

            bool m_group;
            bool m_hideUnused;
            Assets::TextureManager::SortOrder m_sortOrder;
            String m_filterText;
            
            Renderer::Vbo m_vbo;
            Assets::Texture* m_selectedTexture;
        public:
            TextureBrowserView(wxWindow* parent, wxWindowID windowId,
                               wxScrollBar* scrollBar,
                               Renderer::RenderResources& resources,
                               Assets::TextureManager& textureManager);
            ~TextureBrowserView();

            void setSortOrder(const Assets::TextureManager::SortOrder sortOrder);
            void setGroup(const bool group);
            void setHideUnused(const bool hideUnused);
            void setFilterText(const String& filterText);

            Assets::Texture* selectedTexture() const;
            void setSelectedTexture(Assets::Texture* selectedTexture);
        private:
            void doInitLayout(Layout& layout);
            void doReloadLayout(Layout& layout);
            void addTextureToLayout(Layout& layout, Assets::Texture* texture, const Renderer::FontDescriptor& font);
            
            void doClear();
            void doRender(Layout& layout, const float y, const float height);
            void renderBounds(Layout& layout, const float y, const float height);
            const Color& textureColor(const Assets::Texture& texture) const;
            void renderTextures(Layout& layout, const float y, const float height);
            void renderNames(Layout& layout, const float y, const float height);
            void renderGroupTitleBackgrounds(Layout& layout, const float y, const float height);
            void renderStrings(Layout& layout, const float y, const float height);
            StringMap collectStringVertices(Layout& layout, const float y, const float height);
            
            void doLeftClick(Layout& layout, const float x, const float y);
            wxString tooltip(const Layout::Group::Row::Cell& cell);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureBrowserView__) */
