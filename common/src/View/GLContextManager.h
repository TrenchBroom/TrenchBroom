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

#ifndef TrenchBroom_GLContextManager
#define TrenchBroom_GLContextManager

#include "View/GLContext.h"

class wxGLCanvas;

namespace TrenchBroom {
    namespace Renderer {
        class FontManager;
        class ShaderManager;
        class Vbo;
    }
    
    namespace View {
        class GLContextManager {
        private:
            GLContext::Ptr m_mainContext;
            bool m_initialized;
            
            Renderer::Vbo* m_vertexVbo;
            Renderer::Vbo* m_indexVbo;
            Renderer::FontManager* m_fontManager;
            Renderer::ShaderManager* m_shaderManager;
        public:
            GLContextManager();
            ~GLContextManager();
            
            GLContext::Ptr createContext(wxGLCanvas* canvas);
            wxGLContext* mainContext() const;
            
            bool initialized() const;
            bool initialize();
            
            Renderer::Vbo& vertexVbo();
            Renderer::Vbo& indexVbo();
            Renderer::FontManager& fontManager();
            Renderer::ShaderManager& shaderManager();
        private:
            GLContextManager(const GLContextManager& other);
            GLContextManager& operator=(const GLContextManager& other);
        };
    }
}

#endif /* defined(TrenchBroom_GLContextManager) */
