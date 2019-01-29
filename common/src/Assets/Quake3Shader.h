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

#ifndef TRENCHBROOM_QUAKE3SHADER_H
#define TRENCHBROOM_QUAKE3SHADER_H

#include "StringUtils.h"
#include "IO/Path.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Quake3ShaderStage {
        public:
            IO::Path map;
        public:
            bool operator==(const Quake3ShaderStage& other) const;
        };

        class Quake3Shader {
        public:
        public:
            IO::Path shaderPath;
            IO::Path editorImage;
            IO::Path lightImage;
            StringSet surfaceParms;
            std::vector<Quake3ShaderStage> stages;
        public:
            bool operator==(const Quake3Shader& other) const;
            friend bool isEqual(const Quake3Shader& lhs, const Quake3Shader& rhs);

            Quake3ShaderStage& addStage();
        };
    }
}


#endif //TRENCHBROOM_QUAKE3SHADER_H
