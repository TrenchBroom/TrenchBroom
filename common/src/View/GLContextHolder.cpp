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

#include "GLContextHolder.h"
#include "Exceptions.h"
#include "Renderer/GL.h"

#include <wx/glcanvas.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        GLContextHolder::~GLContextHolder() {
            delete m_context;
            m_context = NULL;
        }
        
        bool GLContextHolder::setCurrent(wxGLCanvas* canvas) {
            return canvas->SetCurrent(*m_context);
        }
        
        wxGLContext* GLContextHolder::context() const {
            return m_context;
        }

        const GLContextHolder::GLAttribs& GLContextHolder::attribs() const {
            return doGetAttribs();
        }
        
        void GLContextHolder::initialize() {
            doInitialize();
        }

        Renderer::FontManager& GLContextHolder::fontManager() {
            return doGetFontManager();
        }
        
        Renderer::ShaderManager& GLContextHolder::shaderManager() {
            return doGetShaderManager();
        }

        GLContextHolder::GLContextHolder(wxGLCanvas* canvas, wxGLContext* sharedContext) :
        m_context(new wxGLContext(canvas, sharedContext)) {}

        RootGLContextHolder::RootGLContextHolder(wxGLCanvas* canvas, const GLAttribs& attribs) :
        GLContextHolder(canvas),
        m_attribs(attribs) {}

        const GLContextHolder::GLAttribs& RootGLContextHolder::doGetAttribs() const {
            return m_attribs;
        }
        
        void RootGLContextHolder::doInitialize() {
            glewInitialize();
        }

        Renderer::FontManager& RootGLContextHolder::doGetFontManager() {
            return m_fontManager;
        }
    
        Renderer::ShaderManager& RootGLContextHolder::doGetShaderManager() {
            return m_shaderManager;
        }

        SharedGLContextHolder::SharedGLContextHolder(wxGLCanvas* canvas, Ptr parent) :
        GLContextHolder(canvas, parent->context()),
        m_parent(parent) {}

        const GLContextHolder::GLAttribs& SharedGLContextHolder::doGetAttribs() const {
            return m_parent->attribs();
        }
    
        void SharedGLContextHolder::doInitialize() {
            m_parent->initialize();
        }

        Renderer::FontManager& SharedGLContextHolder::doGetFontManager() {
            return m_parent->fontManager();
        }
        
        Renderer::ShaderManager& SharedGLContextHolder::doGetShaderManager() {
            return m_parent->shaderManager();
        }
    }
}
