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

#include "ShaderManager.h"

#include "IO/FileManager.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Console.h"

namespace TrenchBroom {
    namespace Renderer {
        namespace Shaders {
            const ShaderConfig ColoredEdgeShader = ShaderConfig("Colored Edge Shader Program", "ColoredEdge.vertsh", "Edge.fragsh");
            const ShaderConfig EdgeShader = ShaderConfig("Edge Shader Program", "Edge.vertsh", "Edge.fragsh");
            const ShaderConfig EntityModelShader = ShaderConfig("Entity Model Shader Program", "EntityModel.vertsh", "EntityModel.fragsh");
            const ShaderConfig FaceShader = ShaderConfig("Face Shader Program", "Face.vertsh", "Face.fragsh");
            const ShaderConfig TextShader = ShaderConfig("Text Shader Program", "Text.vertsh", "Text.fragsh");
            const ShaderConfig TextBackgroundShader = ShaderConfig("Text Background Shader Program", "TextBackground.vertsh", "TextBackground.fragsh");
            const ShaderConfig TextureBrowserShader = ShaderConfig("Texture Browser Shader Program", "TextureBrowser.vertsh", "TextureBrowser.fragsh");
            const ShaderConfig TextureBrowserBorderShader = ShaderConfig("Texture Browser Border Shader Program", "TextureBrowserBorder.vertsh", "TextureBrowserBorder.fragsh");
            const ShaderConfig HandleShader = ShaderConfig("Handle Shader Program", "Handle.vertsh", "Handle.fragsh");
            const ShaderConfig InstancedHandleShader = ShaderConfig("Instanced Handle Shader Program", "InstancedHandle.vertsh", "Handle.fragsh");
            const ShaderConfig ColoredHandleShader = ShaderConfig("Colored Handle Shader Program", "ColoredHandle.vertsh", "Handle.fragsh");
        }

        Shader& ShaderManager::loadShader(const String& path, GLenum type) {
            ShaderCache::iterator it = m_shaders.find(path);
            if (it != m_shaders.end())
                return *it->second;
            
            IO::FileManager fileManager;
            String resourceDirectory = fileManager.resourceDirectory();
            Shader* shader = new Shader(fileManager.appendPath(resourceDirectory, path), type, m_console);
            m_shaders.insert(ShaderCacheEntry(path, shader));
            return *shader;
        }
        
        ShaderManager::ShaderManager(Utility::Console& console) :
        m_console(console) {}

        ShaderManager::~ShaderManager() {
            ShaderProgramCache::iterator programIt, programEnd;
            for (programIt = m_programs.begin(), programEnd = m_programs.end(); programIt != programEnd; ++programIt)
                delete programIt->second;
            m_programs.clear();
            
            ShaderCache::iterator shaderIt, shaderEnd;
            for (shaderIt = m_shaders.begin(), shaderEnd = m_shaders.end(); shaderIt != shaderEnd; ++shaderIt)
                delete shaderIt->second;
            m_shaders.clear();
        }
        
        ShaderProgram& ShaderManager::shaderProgram(const ShaderConfig& config) {
            ShaderProgramCache::iterator it = m_programs.find(&config);
            if (it != m_programs.end())
                return *it->second;
            
            ShaderProgram* program = new ShaderProgram(config.name(), m_console);
            
            const StringList& vertexShaders = config.vertexShaders();
            const StringList& fragmentShaders = config.fragmentShaders();
            StringList::const_iterator stringIt, stringEnd;

            for (stringIt = vertexShaders.begin(), stringEnd = vertexShaders.end(); stringIt != stringEnd; ++stringIt) {
                const String& path = *stringIt;
                Shader& shader = loadShader(path, GL_VERTEX_SHADER);
                program->attachShader(shader);
            }

            for (stringIt = fragmentShaders.begin(), stringEnd = fragmentShaders.end(); stringIt != stringEnd; ++stringIt) {
                const String& path = *stringIt;
                Shader& shader = loadShader(path, GL_FRAGMENT_SHADER);
                program->attachShader(shader);
            }
            
            m_programs.insert(ShaderProgramCacheEntry(&config, program));
            return *program;
        }
        
        ActivateShader::ActivateShader(ShaderManager& shaderManager, const ShaderConfig& shaderConfig) :
        m_shaderProgram(shaderManager.shaderProgram(shaderConfig)) {
            m_shaderProgram.activate();
        }
        
        ActivateShader::~ActivateShader() {
            m_shaderProgram.deactivate();
        }

    }
}
