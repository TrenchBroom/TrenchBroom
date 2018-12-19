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
         * Parses Quake 3 shader scripts found in a file system and creates links from the shader names to the actual
         * image files to be loaded.
         *
         * This file system can be used to override the textures using their shaders. This is particularly useful when
         * a shader provides a special editor image.
         */
        class Quake3ShaderFileSystem : public ImageFileSystemBase {
        private:
            const FileSystem& m_fs;
            StringList m_extensions;
            Logger* m_logger;
        public:
            /**
             * Creates a new instance at the given base path that uses the given file system to find shaders and shader
             * image resources.
             *
             * @param fs the filesystem to use when searching for shaders and linking image resources
             * @param extensions the texture extensions to scan
             * @param logger the logger to use
             */
            Quake3ShaderFileSystem(const FileSystem& fs, const StringList& extensions, Logger* logger);
        private:
            void doReadDirectory() override;

            std::vector<Assets::Quake3Shader> loadShaders() const;
            void linkShaders(std::vector<Assets::Quake3Shader>& shaders);
            void linkTextures(const Path::List& textures, std::vector<Assets::Quake3Shader>& shaders);
            void linkStandaloneShaders(std::vector<Assets::Quake3Shader>& shaders);
            void linkShaderToImage(const Path& shaderPath, const Path& imagePath);
        };
    }
}


#endif //TRENCHBROOM_QUAKE3SHADERFILESYSTEM_H
