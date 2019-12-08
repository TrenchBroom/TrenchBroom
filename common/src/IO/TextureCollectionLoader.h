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

#ifndef TextureCollectionLoader_h
#define TextureCollectionLoader_h

#include "IO/Path.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class TextureCollection;
        class TextureReader;
        class TextureManager;

    }
    namespace IO {
        class File;
        class FileSystem;
        class TextureReader;

        class TextureCollectionLoader {
        protected:
            using FileList = std::vector<std::shared_ptr<File>>;
        protected:
            Logger& m_logger;
        protected:
            TextureCollectionLoader(Logger& logger);
        public:
            virtual ~TextureCollectionLoader();
        public:
            std::unique_ptr<Assets::TextureCollection> loadTextureCollection(const Path& path, const std::vector<std::string>& textureExtensions, const TextureReader& textureReader);
        private:
            virtual FileList doFindTextures(const Path& path, const std::vector<std::string>& extensions) = 0;
        };

        class FileTextureCollectionLoader : public TextureCollectionLoader {
        private:
            const Path::List m_searchPaths;
        public:
            FileTextureCollectionLoader(Logger& logger, const Path::List& searchPaths);
        private:
            FileList doFindTextures(const Path& path, const std::vector<std::string>& extensions) override;
        };

        class DirectoryTextureCollectionLoader : public TextureCollectionLoader {
        private:
            const FileSystem& m_gameFS;
        public:
            DirectoryTextureCollectionLoader(Logger& logger, const FileSystem& gameFS);
        private:
            FileList doFindTextures(const Path& path, const std::vector<std::string>& extensions) override;
        };
    }
}

#endif /* TextureCollectionLoader_h */
