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

#ifndef __TrenchBroom__TextureView__
#define __TrenchBroom__TextureView__

#include "Renderer/GL.h"
#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        class TextureView : public wxGLCanvas {
        private:
            Renderer::RenderResources& m_resources;
            wxGLContext* m_glContext;
            Assets::Texture* m_texture;
        public:
            TextureView(wxWindow* parent, wxWindowID windowId, Renderer::RenderResources& resources);
            ~TextureView();
            
            void setTexture(Assets::Texture* texture);
            
            void OnPaint(wxPaintEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureView__) */
