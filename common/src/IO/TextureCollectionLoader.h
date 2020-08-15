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

#include <memory>
#include <string>
#include <vector>
#include <string>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class TextureCollection;
    }

    namespace IO {
        class File;
        class FileSystem;
        class Path;
        class TextureReader;

        class TextureCollectionLoader {
        protected:
            using FileList = std::vector<std::shared_ptr<File>>;
        protected:
            Logger& m_logger;
            const std::vector<std::string> m_textureExclusions;
        protected:
            explicit TextureCollectionLoader(Logger& logger, const std::vector<std::string>& exclusions);
        public:
            virtual ~TextureCollectionLoader();
        public:
            virtual std::unique_ptr<Assets::TextureCollection> loadTextureCollection(const Path& path, const std::vector<std::string>& textureExtensions, const TextureReader& textureReader) = 0;
        protected:
            bool shouldExclude(const std::string& textureName);
        };

        class FileTextureCollectionLoader : public TextureCollectionLoader {
        private:
            const std::vector<Path> m_searchPaths;
        public:
            FileTextureCollectionLoader(Logger& logger, const std::vector<Path>& searchPaths, const std::vector<std::string>& exclusions);
        private:
            std::unique_ptr<Assets::TextureCollection> loadTextureCollection(const Path& path, const std::vector<std::string>& textureExtensions, const TextureReader& textureReader);
        };

        class DirectoryTextureCollectionLoader : public TextureCollectionLoader {
        private:
            const FileSystem& m_gameFS;
        public:
            DirectoryTextureCollectionLoader(Logger& logger, const FileSystem& gameFS, const std::vector<std::string>& exclusions);
        private:
            std::unique_ptr<Assets::TextureCollection> loadTextureCollection(const Path& path, const std::vector<std::string>& textureExtensions, const TextureReader& textureReader);
        };
    }
}

#endif /* TextureCollectionLoader_h */
