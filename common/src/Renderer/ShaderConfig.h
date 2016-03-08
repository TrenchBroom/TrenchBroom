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

#ifndef TrenchBroom_ShaderConfig
#define TrenchBroom_ShaderConfig

#include "StringUtils.h"
#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        class ShaderConfig {
        private:
            String m_name;
            StringList m_vertexShaders;
            StringList m_fragmentShaders;
        public:
            ShaderConfig(const String& name, const String& vertexShader, const String& fragmentShader);
            ShaderConfig(const String& name, const String& vertexShader, const StringList& fragmentShaders);
            ShaderConfig(const String& name, const StringList& vertexShaders, const String& fragmentShader);
            ShaderConfig(const String& name, const StringList& vertexShaders, const StringList& fragmentShaders);
        public:
            const String& name() const;
            const StringList& vertexShaders() const;
            const StringList& fragmentShaders() const;
        };
    }
}

#endif /* defined(TrenchBroom_ShaderConfig) */
