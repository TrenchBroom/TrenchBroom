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

#ifndef TRENCHBROOM_QUAKE3SHADERFILESYSTEM_H
#define TRENCHBROOM_QUAKE3SHADERFILESYSTEM_H

#include "IO/ImageFileSystem.h"

namespace TrenchBroom {
    namespace Assets {
        class Quake3Shader;
    }

    namespace IO {
        class Quake3ShaderFileSystem : public ImageFileSystemBase {
        private:
            const FileSystem& m_fs;
        public:
            Quake3ShaderFileSystem(const Path& path, const FileSystem& fs);
        private:
            void doReadDirectory() override;

            void processScript(const MappedFile::Ptr& file);
            void processShader(const Assets::Quake3Shader& shader);
        };
    }
}


#endif //TRENCHBROOM_QUAKE3SHADERFILESYSTEM_H
