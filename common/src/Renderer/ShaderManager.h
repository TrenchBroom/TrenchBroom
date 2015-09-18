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

#ifndef TrenchBroom_ShaderManager
#define TrenchBroom_ShaderManager

#include "StringUtils.h"
#include "Renderer/GL.h"
#include "Renderer/ShaderProgram.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Renderer {
        class Shader;
        class ShaderConfig;
        
        class ShaderManager {
        private:
            typedef std::map<String, Shader*> ShaderCache;
            typedef std::pair<String, Shader*> ShaderCacheEntry;
            typedef std::map<const ShaderConfig*, ShaderProgram*> ShaderProgramCache;
            typedef std::pair<const ShaderConfig*, ShaderProgram*> ShaderProgramCacheEntry;
            
            ShaderCache m_shaders;
            ShaderProgramCache m_programs;
        public:
            ~ShaderManager();
        
            ShaderProgram& program(const ShaderConfig& config);
        private:
            ShaderProgram* createProgram(const ShaderConfig& config);
            Shader& loadShader(const String& name, const GLenum type);
        };

        class ActiveShader {
        private:
            ShaderProgram& m_program;
        public:
            ActiveShader(ShaderManager& shaderManager, const ShaderConfig& shaderConfig);
            ~ActiveShader();
            
            template <class T>
            void set(const String& name, const T& value) {
                m_program.set(name, value);
            }
        };
    }
}

#endif /* defined(TrenchBroom_ShaderManager) */
