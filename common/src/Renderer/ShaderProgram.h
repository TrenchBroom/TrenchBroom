/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_ShaderProgram
#define TrenchBroom_ShaderProgram

#include "StringUtils.h"
#include "Renderer/GL.h"

#include <vecmath/forward.h>

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class Shader;
        
        class ShaderProgram {
        private:
            using UniformVariableCache = std::map<String, GLint>;
            
            String m_name;
            GLuint m_programId;
            bool m_needsLinking;
            mutable UniformVariableCache m_variableCache;
        public:
            explicit ShaderProgram(const String& name);
            ~ShaderProgram();
            
            void attach(Shader& shader);
            void detach(Shader& shader);
            
            void activate();
            void deactivate();

            void set(const String& name, bool value);
            void set(const String& name, int value);
            void set(const String& name, size_t value);
            void set(const String& name, float value);
            void set(const String& name, double value);
            void set(const String& name, const vm::vec2f& value);
            void set(const String& name, const vm::vec3f& value);
            void set(const String& name, const vm::vec4f& value);
            void set(const String& name, const vm::mat2x2f& value);
            void set(const String& name, const vm::mat3x3f& value);
            void set(const String& name, const vm::mat4x4f& value);
        private:
            void link();
            GLint findUniformLocation(const String& name) const;
            bool checkActive() const;
        };
    }
}

#endif /* defined(TrenchBroom_ShaderProgram) */
