/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include "IO/FileSystem.h"
#include "IO/Path.h"
#include "Renderer/Shader.h"

namespace TrenchBroom {
    namespace Renderer {
        ShaderConfig::ShaderConfig(const String& name, const String& vertexShader, const String& fragmentShader) :
        m_name(name) {
            m_vertexShaders.push_back(vertexShader);
            m_fragmentShaders.push_back(fragmentShader);
        }
        
        const String& ShaderConfig::name() const {
            return m_name;
        }
        
        const StringList& ShaderConfig::vertexShaders() const {
            return m_vertexShaders;
        }
        
        const StringList& ShaderConfig::fragmentShaders() const {
            return m_fragmentShaders;
        }

        namespace Shaders {
            const ShaderConfig ColoredEdgeShader = ShaderConfig("Colored Edge Shader Program", "ColoredEdge.vertsh", "Edge.fragsh");
            const ShaderConfig EdgeShader = ShaderConfig("Edge Shader Program", "Edge.vertsh", "Edge.fragsh");
            const ShaderConfig EntityModelShader = ShaderConfig("Entity Model Shader Program", "EntityModel.vertsh", "EntityModel.fragsh");
            const ShaderConfig FaceShader = ShaderConfig("Face Shader Program", "Face.vertsh", "Face.fragsh");
            const ShaderConfig ColoredTextShader = ShaderConfig("Colored Text Shader Program", "ColoredText.vertsh", "Text.fragsh");
            const ShaderConfig TextShader = ShaderConfig("Text Shader Program", "Text.vertsh", "Text.fragsh");
            const ShaderConfig TextBackgroundShader = ShaderConfig("Text Background Shader Program", "TextBackground.vertsh", "TextBackground.fragsh");
            const ShaderConfig TextureBrowserShader = ShaderConfig("FaceTexture Browser Shader Program", "TextureBrowser.vertsh", "TextureBrowser.fragsh");
            const ShaderConfig TextureBrowserBorderShader = ShaderConfig("FaceTexture Browser Border Shader Program", "TextureBrowserBorder.vertsh", "TextureBrowserBorder.fragsh");
            const ShaderConfig BrowserGroupShader = ShaderConfig("Browser Group Shader Program", "BrowserGroup.vertsh", "BrowserGroup.fragsh");
            const ShaderConfig HandleShader = ShaderConfig("Handle Shader Program", "Handle.vertsh", "Handle.fragsh");
            const ShaderConfig PointHandleShader = ShaderConfig("Point Handle Shader Program", "PointHandle.vertsh", "Handle.fragsh");
            const ShaderConfig InstancedPointHandleShader = ShaderConfig("Instanced Point Handle Shader Program", "InstancedPointHandle.vertsh", "Handle.fragsh");
            const ShaderConfig ColoredHandleShader = ShaderConfig("Colored Handle Shader Program", "ColoredHandle.vertsh", "Handle.fragsh");
            const ShaderConfig CompassShader = ShaderConfig("Compass Shader Program", "Compass.vertsh", "Compass.fragsh");
            const ShaderConfig CompassOutlineShader = ShaderConfig("Compass Outline Shader Program", "CompassOutline.vertsh", "Compass.fragsh");
            const ShaderConfig EntityLinkShader = ShaderConfig("Entity Link Shader Program", "EntityLink.vertsh", "EntityLink.fragsh");
        }

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
            
            IO::FileSystem fs;
            const IO::Path resourceDirectory = fs.resourceDirectory();
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
