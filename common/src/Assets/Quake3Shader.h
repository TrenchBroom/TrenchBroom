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

namespace TrenchBroom {
    namespace Assets {
        class Quake3Shader {
        private:
            bool m_hasTexturePath;
            IO::Path m_texturePath;

            bool m_hasQerImagePath;
            IO::Path m_qerImagePath;

            StringSet m_surfaceParms;
        public:
            Quake3Shader();

            bool operator==(const Quake3Shader& other) const;

            friend bool isEqual(const Quake3Shader& lhs, const Quake3Shader& rhs);

            bool hasTexturePath() const;
            const IO::Path& texturePath() const;
            void setTexturePath(const IO::Path& texturePath);

            bool hasQerImagePath() const;
            IO::Path qerImagePath(const IO::Path& defaultPath = IO::Path()) const;
            void setQerImagePath(const IO::Path& qerImagePath);
            void clearQerImagePath();

            const StringSet& surfaceParms() const;
            void addSurfaceParm(const String& parm) ;
        };
    }
}


#endif //TRENCHBROOM_QUAKE3SHADER_H
