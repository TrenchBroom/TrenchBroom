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

#include "IO/Path.h"

namespace TrenchBroom {
    namespace Assets {
        class Quake3Shader {
        public:
            IO::Path texturePath;
            IO::Path qerImagePath;

            Quake3Shader() = default;
            Quake3Shader(const IO::Path& i_texturePath, const IO::Path& i_qerImagePath);

            bool operator==(const Quake3Shader& other) const;
        };
    }
}


#endif //TRENCHBROOM_QUAKE3SHADER_H
