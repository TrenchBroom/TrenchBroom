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

#include "Ensure.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderConfig.h"

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace Renderer {
        ShaderManager::ShaderManager() : m_currentProgram(nullptr) {}

        ShaderManager::~ShaderManager() = default;

        ShaderProgram& ShaderManager::program(const ShaderConfig& config) {
            auto it = m_programs.find(&config);
            if (it != std::end(m_programs)) {
                return *it->second;
            }

            auto result = m_programs.emplace(&config, createProgram(config));
            assert(result.second);

            return *(result.first->second);
        }

        ShaderProgram* ShaderManager::currentProgram() {
            return m_currentProgram;
        }

        void ShaderManager::setCurrentProgram(ShaderProgram* program) {
            m_currentProgram = program;
        }

        std::unique_ptr<ShaderProgram> ShaderManager::createProgram(const ShaderConfig& config) {
            auto program = std::make_unique<ShaderProgram>(this, config.name());

            for (const auto& path : config.vertexShaders()) {
                Shader& shader = loadShader(path, GL_VERTEX_SHADER);
                program->attach(shader);
            }

            for (const auto& path : config.fragmentShaders()) {
                Shader& shader = loadShader(path, GL_FRAGMENT_SHADER);
                program->attach(shader);
            }

            return program;
        }

        Shader& ShaderManager::loadShader(const std::string& name, const GLenum type) {
            auto it = m_shaders.find(name);
            if (it != std::end(m_shaders)) {
                return *it->second;
            }

            const auto shaderPath = IO::SystemPaths::findResourceFile(IO::Path("shader") + IO::Path(name));
            auto result = m_shaders.emplace(name, std::make_unique<Shader>(shaderPath, type));
            assert(result.second);

            return *(result.first->second);
        }
    }
}
