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

#ifndef __TrenchBroom__Shader__
#define __TrenchBroom__Shader__

#include "Utility/GLee.h"
#include "Utility/String.h"

namespace TrenchBroom {
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        class Shader {
        private:
            Utility::Console& m_console;
            GLenum m_type;
            String m_source;
            GLuint m_shaderId;
        public:
            Shader(GLenum type, const String& source, Utility::Console& console);
            ~Shader();
            
            bool createShader();
            void attachTo(GLuint programId);
            void detachFrom(GLuint programId);
        };
        
        class ShaderProgram {
        private:
            Utility::Console& m_console;
            GLuint m_programId;
            bool m_needsLinking;
        public:
            ShaderProgram(Utility::Console& console);
            ~ShaderProgram();
            
            bool createProgram();
            
            
            void attachShader(Shader& shader);
            void detachShader(Shader& shader);
            
            void activate();
            void deactivate();
        };
    }
}

#endif /* defined(__TrenchBroom__Shader__) */
