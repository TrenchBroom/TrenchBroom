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

#ifndef __TrenchBroom__ShaderManager__
#define __TrenchBroom__ShaderManager__

#include "GL/glew.h"
#include "Utility/String.h"

#include <map>

namespace TrenchBroom {
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        class ShaderConfig {
        private:
            String m_name;
            StringList m_vertexShaders;
            StringList m_fragmentShaders;
        public:
            ShaderConfig(const String name, const String& vertexShader, const String& fragmentShader) :
            m_name(name) {
                m_vertexShaders.push_back(vertexShader);
                m_fragmentShaders.push_back(fragmentShader);
            }
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const StringList& vertexShaders() const {
                return m_vertexShaders;
            }
            
            inline const StringList& fragmentShaders() const {
                return m_fragmentShaders;
            }
        };
        
        namespace Shaders {
            extern const ShaderConfig ColoredEdgeShader;
            extern const ShaderConfig EdgeShader;
            extern const ShaderConfig EntityModelShader;
            extern const ShaderConfig FaceShader;
            extern const ShaderConfig TextShader;
            extern const ShaderConfig TextBackgroundShader;
            extern const ShaderConfig TextureBrowserShader;
            extern const ShaderConfig TextureBrowserBorderShader;
            extern const ShaderConfig HandleShader;
            extern const ShaderConfig PointHandleShader;
            extern const ShaderConfig InstancedPointHandleShader;
            extern const ShaderConfig ColoredHandleShader;
            extern const ShaderConfig EntityLinkShader;
        }

        class Shader;
        class ShaderProgram;
        
        class ShaderManager {
        private:
            typedef std::map<String, Shader*> ShaderCache;
            typedef std::pair<String, Shader*> ShaderCacheEntry;
            typedef std::map<const ShaderConfig*, ShaderProgram*> ShaderProgramCache;
            typedef std::pair<const ShaderConfig*, ShaderProgram*> ShaderProgramCacheEntry;
            
            Utility::Console& m_console;
            ShaderCache m_shaders;
            ShaderProgramCache m_programs;
            
            Shader& loadShader(const String& path, GLenum type);
        public:
            ShaderManager(Utility::Console& console);
            ~ShaderManager();
            
            ShaderProgram& shaderProgram(const ShaderConfig& config);
        };
        
        class ActivateShader {
        private:
            ShaderProgram& m_shaderProgram;
        public:
            ActivateShader(ShaderManager& shaderManager, const ShaderConfig& shaderConfig);
            ~ActivateShader();
            
            inline ShaderProgram& currentShader() {
                return m_shaderProgram;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__ShaderManager__) */
