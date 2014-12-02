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

#include "GLContextManager.h"

#include "Renderer/FontManager.h"
#include "Renderer/GL.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace View {
        GLContextManager::GLContextManager() :
        m_initialized(false),
        m_fontManager(new Renderer::FontManager()),
        m_shaderManager(new Renderer::ShaderManager()) {}
        
        GLContextManager::~GLContextManager() {
            delete m_fontManager;
            delete m_shaderManager;
        }

        GLContext::Ptr GLContextManager::createContext(wxGLCanvas* canvas) {
            GLContext::Ptr context(new GLContext(canvas, this));
            if (m_mainContext == NULL)
                m_mainContext = context;
            return context;
        }

        wxGLContext* GLContextManager::mainContext() const {
            return m_mainContext.get();
        }
        
        bool GLContextManager::initialized() const {
            return m_initialized;
        }

        bool GLContextManager::initialize() {
            if (!m_initialized) {
                glewInitialize();
                m_initialized = true;
                return true;
            }
            return false;
        }
        
        Renderer::FontManager& GLContextManager::fontManager() {
            return *m_fontManager;
        }
        
        Renderer::ShaderManager& GLContextManager::shaderManager() {
            return *m_shaderManager;
        }
    }
}
