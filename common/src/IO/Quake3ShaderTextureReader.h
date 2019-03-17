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

#ifndef Quake3ShaderTextureReader_h
#define Quake3ShaderTextureReader_h

#include "IO/TextureReader.h"

#include <memory>

namespace TrenchBroom {
    namespace Assets {
        class Quake3Shader;
    }

    namespace IO {
        class File;
        class FileSystem;
        class Path;

        /**
         * Loads a texture that represents a Quake 3 shader from the file system. Uses a given file system
         * to locate the actual editor image for the shader. The shader is expected to be readily parsed and
         * available as a virtual object file in the file system.
         */
        class Quake3ShaderTextureReader : public TextureReader {
        private:
            const FileSystem& m_fs;
        public:
            /**
             * Creates a texture reader using the given name strategy and file system to locate the texture image.
             *
             * @param nameStrategy the strategy to determine the texture name
             * @param fs the file system to use when locating the texture image
             */
            Quake3ShaderTextureReader(const NameStrategy& nameStrategy, const FileSystem& fs);
        private:
            Assets::Texture* doReadTexture(std::shared_ptr<File> file) const override;
            Assets::Texture* loadTextureImage(const Path& shaderPath, const Path& imagePath) const;
            Path findTexturePath(const Assets::Quake3Shader& shader) const;
            Path findTexture(const Path& texturePath) const;
        };
    }
}

#endif /* Quake3ShaderTextureReader_h */
