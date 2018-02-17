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

#include "StringUtils.h"
#include "IO/MappedFile.h"
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
        class FileSystem;
        class TextureReader;

        class TextureCollectionLoader {
        public:
            typedef std::unique_ptr<TextureCollectionLoader> Ptr;
        protected:
            TextureCollectionLoader();
        public:
            virtual ~TextureCollectionLoader();
        public:
            Assets::TextureCollection* loadTextureCollection(const Path& path, const String& textureExtension, const TextureReader& textureReader);
        private:
            virtual MappedFile::List doFindTextures(const Path& path, const String& extension) = 0;
        };
        
        class FileTextureCollectionLoader : public TextureCollectionLoader {
        private:
            const Path::List m_searchPaths;
        public:
            FileTextureCollectionLoader(const Path::List& searchPaths);
        private:
            MappedFile::List doFindTextures(const Path& path, const String& extension) override;
        };
        
        class DirectoryTextureCollectionLoader : public TextureCollectionLoader {
        private:
            const FileSystem& m_gameFS;
        public:
            DirectoryTextureCollectionLoader(const FileSystem& gameFS);
        private:
            MappedFile::List doFindTextures(const Path& path, const String& extension) override;
        };
    }
}

#endif /* TextureCollectionLoader_h */
