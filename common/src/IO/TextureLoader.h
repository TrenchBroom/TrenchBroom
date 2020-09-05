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

#ifndef TextureLoader_h
#define TextureLoader_h

#include "Macros.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class Palette;
        class TextureCollection;
        class TextureManager;
    }

    namespace Model {
        struct TextureConfig;
    }

    namespace IO {
        class FileSystem;
        class Path;
        class TextureCollectionLoader;
        class TextureReader;

        class TextureLoader {
        private:
            std::vector<std::string> m_textureExtensions;
            std::unique_ptr<TextureReader> m_textureReader;
            std::unique_ptr<TextureCollectionLoader> m_textureCollectionLoader;
        public:
            TextureLoader(const FileSystem& gameFS, const std::vector<Path>& fileSearchPaths, const Model::TextureConfig& textureConfig, Logger& logger);
            ~TextureLoader();
        private:
            static std::vector<std::string> getTextureExtensions(const Model::TextureConfig& textureConfig);
            static std::unique_ptr<TextureReader> createTextureReader(const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger);
            static Assets::Palette loadPalette(const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger);
            static std::unique_ptr<TextureCollectionLoader> createTextureCollectionLoader(const FileSystem& gameFS, const std::vector<Path>& fileSearchPaths, const Model::TextureConfig& textureConfig, Logger& logger);
        public:
            Assets::TextureCollection loadTextureCollection(const Path& path);
            void loadTextures(const std::vector<Path>& paths, Assets::TextureManager& textureManager);

            deleteCopyAndMove(TextureLoader)
        };
    }
}

#endif /* TextureLoader_h */
