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

#include "GLContext.h"

#include "View/GLContextManager.h"

namespace TrenchBroom {
    namespace View {
        GLContext::GLContext(wxGLCanvas* canvas, GLContextManager* contextManager) :
        wxGLContext(canvas, contextManager->mainContext()),
        m_contextManager(contextManager) {}
        
        Renderer::Vbo& GLContext::vertexVbo() {
            return m_contextManager->vertexVbo();
        }
        
        Renderer::Vbo& GLContext::indexVbo() {
            return m_contextManager->indexVbo();
        }
        
        Renderer::FontManager& GLContext::fontManager() {
            return m_contextManager->fontManager();
        }
        
        Renderer::ShaderManager& GLContext::shaderManager() {
            return m_contextManager->shaderManager();
        }

        bool GLContext::initialize() {
            return m_contextManager->initialize();
        }
        
        bool GLContext::SetCurrent(const wxGLCanvas* canvas) const {
            return SetCurrent(*canvas);
        }

        bool GLContext::SetCurrent(const wxGLCanvas& canvas) const {
            return wxGLContext::SetCurrent(canvas);
        }
    }
}
