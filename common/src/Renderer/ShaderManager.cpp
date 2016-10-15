/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "ShaderManager.h"

#include "CollectionUtils.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderConfig.h"

namespace TrenchBroom {
    namespace Renderer {
        ShaderManager::~ShaderManager() {
            MapUtils::clearAndDelete(m_programs);
            MapUtils::clearAndDelete(m_shaders);
        }
        
        ShaderProgram& ShaderManager::program(const ShaderConfig& config) {
            ShaderProgramCache::iterator it = m_programs.find(&config);
            if (it != m_programs.end())
                return *it->second;
            
            ShaderProgram* program = createProgram(config);
            m_programs.insert(ShaderProgramCacheEntry(&config, program));
            return *program;
        }
        
        ShaderProgram* ShaderManager::createProgram(const ShaderConfig& config) {
            ShaderProgram* program = new ShaderProgram(config.name());
            try {
                const StringList& vertexShaders = config.vertexShaders();
                const StringList& fragmentShaders = config.fragmentShaders();
                StringList::const_iterator stringIt, stringEnd;
                
                for (stringIt = vertexShaders.begin(), stringEnd = vertexShaders.end(); stringIt != stringEnd; ++stringIt) {
                    const String& path = *stringIt;
                    Shader& shader = loadShader(path, GL_VERTEX_SHADER);
                    program->attach(shader);
                }
                
                for (stringIt = fragmentShaders.begin(), stringEnd = fragmentShaders.end(); stringIt != stringEnd; ++stringIt) {
                    const String& path = *stringIt;
                    Shader& shader = loadShader(path, GL_FRAGMENT_SHADER);
                    program->attach(shader);
                }
            } catch (...) {
                delete program;
                throw;
            }
            return program;
        }

        Shader& ShaderManager::loadShader(const String& name, const GLenum type) {
            ShaderCache::iterator it = m_shaders.find(name);
            if (it != m_shaders.end())
                return *it->second;
            
            const IO::Path resourceDirectory = IO::SystemPaths::resourceDirectory();
            const IO::Path shaderDirectory = resourceDirectory + IO::Path("shader");
            const IO::Path shaderPath = shaderDirectory + IO::Path(name);
            
            Shader* shader = new Shader(shaderPath, type);
            m_shaders.insert(ShaderCacheEntry(name, shader));
            return *shader;
        }

        ActiveShader::ActiveShader(ShaderManager& shaderManager, const ShaderConfig& shaderConfig) :
        m_program(shaderManager.program(shaderConfig)) {
            m_program.activate();
        }
        
        ActiveShader::~ActiveShader() {
            m_program.deactivate();
        }
    }
}
