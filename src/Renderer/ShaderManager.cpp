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
#include "IO/Path.h"
#include "IO/SystemPaths.h"
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
            const ShaderConfig VaryingPCShader            = ShaderConfig("Varying Position / Color",         "VaryingPC.vertsh",            "VaryingPC.fragsh");
            const ShaderConfig VaryingPUniformCShader     = ShaderConfig("Varying Position / Uniform Color", "VaryingPUniformC.vertsh",     "VaryingPC.fragsh");
            const ShaderConfig EntityModelShader          = ShaderConfig("Entity Model",                     "EntityModel.vertsh",          "EntityModel.fragsh");
            const ShaderConfig FaceShader                 = ShaderConfig("Face",                             "Face.vertsh",                 "Face.fragsh");
            const ShaderConfig ColoredTextShader          = ShaderConfig("Colored Text",                     "ColoredText.vertsh",          "Text.fragsh");
            const ShaderConfig TextShader                 = ShaderConfig("Text",                             "Text.vertsh",                 "Text.fragsh");
            const ShaderConfig TextBackgroundShader       = ShaderConfig("Text Background",                  "TextBackground.vertsh",       "TextBackground.fragsh");
            const ShaderConfig TextureBrowserShader       = ShaderConfig("Texture Browser",              "TextureBrowser.vertsh",       "TextureBrowser.fragsh");
            const ShaderConfig TextureBrowserBorderShader = ShaderConfig("Texture Browser Border",       "TextureBrowserBorder.vertsh", "TextureBrowserBorder.fragsh");
            const ShaderConfig BrowserGroupShader         = ShaderConfig("Browser Group",                    "BrowserGroup.vertsh",         "BrowserGroup.fragsh");
            const ShaderConfig HandleShader               = ShaderConfig("Handle",                           "Handle.vertsh",               "Handle.fragsh");
            const ShaderConfig PointHandleShader          = ShaderConfig("Point Handle",                     "PointHandle.vertsh",          "Handle.fragsh");
            const ShaderConfig InstancedPointHandleShader = ShaderConfig("Instanced Point Handle",           "InstancedPointHandle.vertsh", "Handle.fragsh");
            const ShaderConfig ColoredHandleShader        = ShaderConfig("Colored Handle",                   "ColoredHandle.vertsh",        "Handle.fragsh");
            const ShaderConfig CompassShader              = ShaderConfig("Compass",                          "Compass.vertsh",              "Compass.fragsh");
            const ShaderConfig CompassOutlineShader       = ShaderConfig("Compass Outline",                  "CompassOutline.vertsh",       "Compass.fragsh");
            const ShaderConfig CompassBackgroundShader    = ShaderConfig("Compass Background",               "VaryingPUniformC.vertsh",     "VaryingPC.fragsh");
            const ShaderConfig EntityLinkShader           = ShaderConfig("Entity Link",                      "EntityLink.vertsh",           "EntityLink.fragsh");
            const ShaderConfig TriangleShader             = ShaderConfig("Shaded Triangles",                 "Triangle.vertsh",             "Triangle.fragsh");
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
