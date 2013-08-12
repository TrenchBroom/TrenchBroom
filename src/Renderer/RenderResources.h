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

#ifndef __TrenchBroom__RenderResources__
#define __TrenchBroom__RenderResources__

#include "Renderer/FontManager.h"
#include "Renderer/ShaderManager.h"

#include <vector>

class wxGLContext;

namespace TrenchBroom {
    namespace Renderer {
        class RenderResources {
        public:
            typedef std::vector<int> GLAttribs;
        private:
            GLAttribs m_glAttribs;
            wxGLContext* m_sharedContext;
            FontManager m_fontManager;
            ShaderManager m_shaderManager;
        public:
            RenderResources(const GLAttribs& glAttribs, wxGLContext* sharedContext);
            
            const GLAttribs& glAttribs() const;
            wxGLContext* sharedContext() const;
            FontManager& fontManager();
            ShaderManager& shaderManager();
        private:
            RenderResources(const RenderResources& other);
            RenderResources& operator= (const RenderResources& other);
        };
    }
}

#endif /* defined(__TrenchBroom__RenderResources__) */
