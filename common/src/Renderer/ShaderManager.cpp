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
            if (it != std::end(m_programs))
                return *it->second;

            ShaderProgram* program = createProgram(config);
            m_programs.insert(ShaderProgramCacheEntry(&config, program));
            return *program;
        }

        ShaderProgram* ShaderManager::createProgram(const ShaderConfig& config) {
            ShaderProgram* program = new ShaderProgram(config.name());
            try {
                for (const String& path : config.vertexShaders()) {
                    Shader& shader = loadShader(path, GL_VERTEX_SHADER);
                    program->attach(shader);
                }

                for (const String& path : config.fragmentShaders()) {
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
            if (it != std::end(m_shaders))
                return *it->second;

            const IO::Path shaderPath = IO::SystemPaths::findResourceFile(IO::Path("shader") + IO::Path(name));

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
