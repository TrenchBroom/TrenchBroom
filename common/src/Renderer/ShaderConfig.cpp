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

#include "ShaderConfig.h"

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        ShaderConfig::ShaderConfig(const String& name, const String& vertexShader, const String& fragmentShader) :
        m_name(name),
        m_vertexShaders(1, vertexShader),
        m_fragmentShaders(1, fragmentShader) {}

        ShaderConfig::ShaderConfig(const String& name, const String& vertexShader, const StringList& fragmentShaders) :
        m_name(name),
        m_vertexShaders(1, vertexShader),
        m_fragmentShaders(fragmentShaders) {}
        
        ShaderConfig::ShaderConfig(const String& name, const StringList& vertexShaders, const String& fragmentShader) :
        m_name(name),
        m_vertexShaders(vertexShaders),
        m_fragmentShaders(1, fragmentShader) {}
        
        ShaderConfig::ShaderConfig(const String& name, const StringList& vertexShaders, const StringList& fragmentShaders) :
        m_name(name),
        m_vertexShaders(vertexShaders),
        m_fragmentShaders(fragmentShaders) {}

        const String& ShaderConfig::name() const {
            return m_name;
        }
        
        const StringList& ShaderConfig::vertexShaders() const {
            return m_vertexShaders;
        }
        
        const StringList& ShaderConfig::fragmentShaders() const {
            return m_fragmentShaders;
        }
    }
}
