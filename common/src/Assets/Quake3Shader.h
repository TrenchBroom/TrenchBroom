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
        private:
            bool m_hasTexturePath;
            IO::Path m_texturePath;

            bool m_hasQerImagePath;
            IO::Path m_qerImagePath;

            bool m_hasQerTransparency;
            float m_qerTransparency;
        public:
            Quake3Shader();

            bool operator==(const Quake3Shader& other) const;

            bool hasTexturePath() const;
            const IO::Path& texturePath() const;
            void setTexturePath(const IO::Path& texturePath);

            bool hasQerImagePath() const;
            const IO::Path& qerImagePath() const;
            void setQerImagePath(const IO::Path& qerImagePath);

            bool hasQerTransparency() const;
            float qerTransparency() const;
            void setQerTransparency(float qerTransparency);
        };
    }
}


#endif //TRENCHBROOM_QUAKE3SHADER_H
