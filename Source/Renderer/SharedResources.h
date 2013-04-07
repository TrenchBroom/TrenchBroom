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

#ifndef __TrenchBroom__SharedResources__
#define __TrenchBroom__SharedResources__

#include "Utility/String.h"

#include <wx/frame.h>

#include <cassert>

class wxGLCanvas;
class wxGLContext;

namespace TrenchBroom {
    namespace Model {
        class TextureManager;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        namespace Text {
            class FontManager;
        }
        
        class EntityModelRendererManager;
        class Palette;
        class ShaderManager;
        class TextureRendererManager;
        
        class SharedResources : public wxFrame {
        protected:
            Palette* m_palette;
            EntityModelRendererManager* m_modelRendererManager;
            ShaderManager* m_shaderManager;
            TextureRendererManager* m_textureRendererManager;
            Text::FontManager* m_fontManager;
            
            int* m_attribs;
            wxGLContext* m_sharedContext;
            wxGLCanvas* m_glCanvas;
            
            unsigned int m_retainCount;
        public:
            SharedResources(Model::TextureManager& textureManager, Utility::Console& console);
            ~SharedResources();
            
            inline const Palette& palette() const {
                assert(m_palette != NULL);
                return *m_palette;
            }
            
            void loadPalette(const String& palettePath);
            
            inline EntityModelRendererManager& modelRendererManager() const {
                return *m_modelRendererManager;
            }
            
            inline ShaderManager& shaderManager() const {
                return *m_shaderManager;
            }
            
            inline TextureRendererManager& textureRendererManager() const {
                return *m_textureRendererManager;
            }
            
            inline Text::FontManager& fontManager() const {
                return *m_fontManager;
            }
            
            inline const int* attribs() const {
                return m_attribs;
            }
            
            inline wxGLContext* sharedContext() const {
                return m_sharedContext;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__SharedResources__) */
