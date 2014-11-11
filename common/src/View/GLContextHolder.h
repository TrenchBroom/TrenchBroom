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

#ifndef __TrenchBroom__GLContextHolder__
#define __TrenchBroom__GLContextHolder__

#include "SharedPointer.h"
#include "Renderer/FontManager.h"
#include "Renderer/ShaderManager.h"

#include <vector>

class wxGLCanvas;
class wxGLContext;

namespace TrenchBroom {
    namespace View {
        class GLContextHolder {
        public:
            typedef std::vector<int> GLAttribs;
            typedef std::tr1::shared_ptr<GLContextHolder> Ptr;
        private:
            wxGLContext* m_context;
        public:
            virtual ~GLContextHolder();

            bool setCurrent(wxGLCanvas* canvas);

            wxGLContext* context() const;
            const GLAttribs& attribs() const;
            
            bool initialize();
            
            Renderer::FontManager& fontManager();
            Renderer::ShaderManager& shaderManager();
        protected:
            GLContextHolder(wxGLCanvas* canvas, wxGLContext* sharedContext = NULL);
        private:
            GLContextHolder(const GLContextHolder& other);
            GLContextHolder& operator= (const GLContextHolder& other);

            virtual const GLAttribs& doGetAttribs() const = 0;
            virtual bool doInitialize() = 0;
            virtual Renderer::FontManager& doGetFontManager() = 0;
            virtual Renderer::ShaderManager& doGetShaderManager() = 0;
        };
        
        class RootGLContextHolder : public GLContextHolder {
        private:
            GLAttribs m_attribs;
            bool m_initialized;
            Renderer::FontManager m_fontManager;
            Renderer::ShaderManager m_shaderManager;
        public:
            RootGLContextHolder(wxGLCanvas* canvas, const GLAttribs& attribs);
        private:
            RootGLContextHolder(const GLContextHolder& other);
            RootGLContextHolder& operator= (const GLContextHolder& other);

            const GLAttribs& doGetAttribs() const;
            bool doInitialize();
            Renderer::FontManager& doGetFontManager();
            Renderer::ShaderManager& doGetShaderManager();
        };
        
        class SharedGLContextHolder : public GLContextHolder {
        private:
            Ptr m_parent;
        public:
            SharedGLContextHolder(wxGLCanvas* canvas, Ptr parent);
        private:
            SharedGLContextHolder(const GLContextHolder& other);
            SharedGLContextHolder& operator= (const GLContextHolder& other);

            const GLAttribs& doGetAttribs() const;
            bool doInitialize();
            Renderer::FontManager& doGetFontManager();
            Renderer::ShaderManager& doGetShaderManager();
        };
    }
}

#endif /* defined(__TrenchBroom__GLContextHolder__) */
