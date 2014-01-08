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

#ifndef __TrenchBroom__ShaderManager__
#define __TrenchBroom__ShaderManager__

#include "StringUtils.h"
#include "Renderer/GL.h"
#include "Renderer/ShaderProgram.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Renderer {
        class ShaderConfig {
        private:
            String m_name;
            StringList m_vertexShaders;
            StringList m_fragmentShaders;
        public:
            ShaderConfig(const String& name, const String& vertexShader, const String& fragmentShader);
            
            const String& name() const;
            const StringList& vertexShaders() const;
            const StringList& fragmentShaders() const;
        };
        
        namespace Shaders {
            extern const ShaderConfig VaryingPCShader;
            extern const ShaderConfig VaryingPUniformCShader;
            extern const ShaderConfig EntityModelShader;
            extern const ShaderConfig FaceShader;
            extern const ShaderConfig ColoredTextShader;
            extern const ShaderConfig TextShader;
            extern const ShaderConfig TextBackgroundShader;
            extern const ShaderConfig TextureBrowserShader;
            extern const ShaderConfig TextureBrowserBorderShader;
            extern const ShaderConfig BrowserGroupShader;
            extern const ShaderConfig HandleShader;
            extern const ShaderConfig PointHandleShader;
            extern const ShaderConfig InstancedPointHandleShader;
            extern const ShaderConfig ColoredHandleShader;
            extern const ShaderConfig CompassShader;
            extern const ShaderConfig CompassOutlineShader;
            extern const ShaderConfig CompassBackgroundShader;
            extern const ShaderConfig EntityLinkShader;
            extern const ShaderConfig TriangleShader;
        }
        
        class Shader;
        
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

#endif /* defined(__TrenchBroom__ShaderManager__) */
