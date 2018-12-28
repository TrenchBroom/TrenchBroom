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
#include "EL.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "IO/Path.h"
#include "IO/TextureCollectionLoader.h"
#include "IO/TextureReader.h"
#include "Model/GameConfig.h"

#include <memory>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class Palette;
        class TextureManager;
    }
    
    namespace IO {
        class FileSystem;
        class TextureCollectionLoader;
        class TextureReader;
        
        class TextureLoader {
        private:
            StringList m_textureExtensions;
            std::unique_ptr<TextureReader> m_textureReader;
            std::unique_ptr<TextureCollectionLoader> m_textureCollectionLoader;
        public:
            TextureLoader(const FileSystem& gameFS, const IO::Path::List& fileSearchPaths, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger);
        private:
            static StringList getTextureExtensions(const Model::GameConfig::TextureConfig& textureConfig);
            static std::unique_ptr<TextureReader> createTextureReader(const FileSystem& gameFS, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger);
            static Assets::Palette loadPalette(const FileSystem& gameFS, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger);
            static std::unique_ptr<TextureCollectionLoader> createTextureCollectionLoader(const FileSystem& gameFS, const IO::Path::List& fileSearchPaths, const Model::GameConfig::TextureConfig& textureConfig, Logger* logger);
        public:
            std::unique_ptr<Assets::TextureCollection> loadTextureCollection(const Path& path);
            void loadTextures(const Path::List& paths, Assets::TextureManager& textureManager);

            deleteCopyAndAssignment(TextureLoader)
        };
    }
}

#endif /* TextureLoader_h */
