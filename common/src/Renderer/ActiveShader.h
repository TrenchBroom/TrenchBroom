/*
 Copyright (C) 2010-2019 Kristian Duske

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

#pragma once

#include "Renderer/ShaderProgram.h"

#include <string>

namespace TrenchBroom {
    namespace Renderer {
        class ShaderConfig;
        class ShaderManager;

        class ActiveShader {
        private:
            ShaderProgram& m_program;
        public:
            ActiveShader(ShaderManager& shaderManager, const ShaderConfig& shaderConfig);
            ~ActiveShader();

            template <class T>
            void set(const std::string& name, const T& value) {
                m_program.set(name, value);
            }
        };
    }
}

#endif //TRENCHBROOM_ACTIVESHADER_H
