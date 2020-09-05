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

#ifndef TrenchBroom_ShaderConfig
#define TrenchBroom_ShaderConfig

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class ShaderConfig {
        private:
            std::string m_name;
            std::vector<std::string> m_vertexShaders;
            std::vector<std::string> m_fragmentShaders;
        public:
            ShaderConfig(const std::string& name, const std::vector<std::string>& vertexShaders, const std::vector<std::string>& fragmentShaders);
        public:
            const std::string& name() const;
            const std::vector<std::string>& vertexShaders() const;
            const std::vector<std::string>& fragmentShaders() const;
        };
    }
}

#endif /* defined(TrenchBroom_ShaderConfig) */
