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

#include "Quake3Shader.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        bool Quake3ShaderStage::operator==(const Quake3ShaderStage& other) const {
            return map == other.map;
        }

        bool Quake3Shader::operator==(const Quake3Shader& other) const {
            return shaderPath == other.shaderPath;
        }

        bool isEqual(const Quake3Shader& lhs, const Quake3Shader& rhs) {
            return (
                lhs.shaderPath == rhs.shaderPath &&
                lhs.editorImage == rhs.editorImage &&
                lhs.lightImage == rhs.lightImage &&
                lhs.surfaceParms == rhs.surfaceParms &&
                lhs.stages == rhs.stages
            );
        }

        Quake3ShaderStage& Quake3Shader::addStage() {
            stages.emplace_back();
            return stages.back();
        }
    }
}

