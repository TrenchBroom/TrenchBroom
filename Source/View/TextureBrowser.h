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

#ifndef __TrenchBroom__TextureBrowser__
#define __TrenchBroom__TextureBrowser__

#include "Model/TextureManager.h"
#include "Renderer/Text/StringManager.h"
#include "View/CellLayoutGLCanvas.h"

namespace TrenchBroom {
    namespace Model {
        class Texture;
        class TextureCollection;
        class TextureManager;
    }
    
    namespace Renderer {
        namespace Text {
            class FontDescriptor;
        }
        
        class Vbo;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class TextureCellData {
        public:
            Model::Texture* texture;
            Renderer::Text::FontDescriptor fontDescriptor;
            
            TextureCellData(Model::Texture* texture, const Renderer::Text::FontDescriptor& fontDescriptor) :
            texture(texture),
            fontDescriptor(fontDescriptor) {}
        };
        
        class TextureBrowser : public CellLayoutGLCanvas<TextureCellData, Model::TextureCollection*> {
        protected:
            Utility::Console& m_console;
            Model::TextureManager& m_textureManager;
            Renderer::Text::StringManager m_stringManager;
            Renderer::Vbo* m_vbo;
            
            bool m_group;
            bool m_hideUnused;
            Model::TextureSortOrder::Type m_sortOrder;
            String m_filterText;
            
            virtual void doInitLayout(Layout& layout);
            void addTextureToLayout(Layout& layout, Model::Texture* texture, const Renderer::Text::FontDescriptor& font);
            virtual void doReloadLayout(Layout& layout);
            virtual void doRender(Layout& layout, const wxRect& rect);
        public:
            TextureBrowser(wxWindow* parent, wxGLContext* sharedContext, Utility::Console& console, Model::TextureManager& textureManager);
            ~TextureBrowser();
        };
    }
}

#endif /* defined(__TrenchBroom__TextureBrowser__) */
