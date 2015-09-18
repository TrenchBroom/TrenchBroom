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

#ifndef TrenchBroom_Shader
#define TrenchBroom_Shader

#include "StringUtils.h"
#include "Renderer/GL.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace Renderer {
        class Shader {
        private:
            String m_name;
            GLenum m_type;
            GLuint m_shaderId;
        public:
            Shader(const IO::Path& path, const GLenum type);
            ~Shader();
            
            void attach(const GLuint programId);
            void detach(const GLuint programId);
        private:
            static StringList loadSource(const IO::Path& path);
        };
    }
}

#endif /* defined(TrenchBroom_Shader) */
