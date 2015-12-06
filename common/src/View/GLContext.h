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

#ifndef TrenchBroom_GLContext
#define TrenchBroom_GLContext

#include "SharedPointer.h"

#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Renderer {
        class FontManager;
        class ShaderManager;
        class Vbo;
    }
    
    namespace View {
        class GLContextManager;
        
        class GLContext : public wxGLContext {
        public:
            typedef std::tr1::shared_ptr<GLContext> Ptr;
        private:
            GLContextManager* m_contextManager;
        public:
            GLContext(wxGLCanvas* canvas, GLContextManager* contextManager);

            Renderer::Vbo& vertexVbo();
            Renderer::Vbo& indexVbo();
            Renderer::FontManager& fontManager();
            Renderer::ShaderManager& shaderManager();
            
            bool initialize();
            bool SetCurrent(const wxGLCanvas* canvas) const;
            bool SetCurrent(const wxGLCanvas& canvas) const;
        };
    }
}

#endif /* defined(TrenchBroom_GLContext) */
