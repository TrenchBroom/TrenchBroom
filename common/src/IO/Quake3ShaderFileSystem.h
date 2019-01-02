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

#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class Quake3Shader;
    }

    namespace IO {
        /**
         * Parses Quake 3 shader scripts found in a file system and makes the shader objects available as virtual files
         * in the file system.
         *
         * Also scans for all textures available at a given prefix path and generates shaders for such textures which
         * do not already have a shader by the same name.
         */
        class Quake3ShaderFileSystem : public ImageFileSystemBase {
        private:
            Path m_texturePrefix;
            Logger* m_logger;
        public:
            /**
             * Creates a new instance at the given base path that uses the given file system to find shaders and shader
             * image resources.
             *
             * @param fs the filesystem to use when searching for shaders and linking image resources
             * @param texturePrefix the path prefix where textures are scanned
             * @param logger the logger to use
             */
            Quake3ShaderFileSystem(std::unique_ptr<FileSystem> fs, const Path& texturePrefix, Logger* logger);
        private:
            void doReadDirectory() override;

            std::vector<Assets::Quake3Shader> loadShaders() const;
            void linkShaders(std::vector<Assets::Quake3Shader>& shaders);
            void linkTextures(const Path::List& textures, std::vector<Assets::Quake3Shader>& shaders);
            void linkStandaloneShaders(std::vector<Assets::Quake3Shader>& shaders);

            Assets::Quake3Shader fixImagePath(Assets::Quake3Shader shader) const;
        };
    }
}


#endif //TRENCHBROOM_QUAKE3SHADERFILESYSTEM_H
