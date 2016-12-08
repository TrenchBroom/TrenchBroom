/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "TextureCollectionLoader.h"

#include "Assets/AssetTypes.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "IO/DiskIO.h"
#include "IO/FileMatcher.h"
#include "IO/FileSystem.h"
#include "IO/TextureReader.h"
#include "IO/WadFileSystem.h"

#include <cassert>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        TextureCollectionLoader::TextureCollectionLoader() {}
        TextureCollectionLoader::~TextureCollectionLoader() {}

        Assets::TextureCollection* TextureCollectionLoader::loadTextureCollection(const Path& path, const String& textureExtension, const TextureReader& textureReader) {
            std::auto_ptr<Assets::TextureCollection> collection(new Assets::TextureCollection(path));
            
            const MappedFile::List files = doFindTextures(path, textureExtension);
            
            MappedFile::List::const_iterator it, end;
            for (it = std::begin(files), end = std::end(files); it != end; ++it) {
                const MappedFile::Ptr file = *it;
                Assets::Texture* texture = textureReader.readTexture(file->begin(), file->end(), file->path());
                collection->addTexture(texture);
            }
            
            return collection.release();
        }

        FileTextureCollectionLoader::FileTextureCollectionLoader(const IO::Path::List& searchPaths) :
        m_searchPaths(searchPaths) {}

        MappedFile::List FileTextureCollectionLoader::doFindTextures(const Path& path, const String& extension) {
            const Path wadPath = Disk::resolvePath(m_searchPaths, path);
            
            WadFileSystem wadFS(wadPath);
            const Path::List paths = wadFS.findItems(Path(""), FileExtensionMatcher(extension));
            
            MappedFile::List result;
            Path::List::const_iterator it, end;
            for (it = std::begin(paths), end = std::end(paths); it != end; ++it)
                result.push_back(wadFS.openFile(*it));
            
            return result;
        }

        DirectoryTextureCollectionLoader::DirectoryTextureCollectionLoader(const FileSystem& gameFS) :
        m_gameFS(gameFS) {}

        MappedFile::List DirectoryTextureCollectionLoader::doFindTextures(const Path& path, const String& extension) {
            const Path::List paths = m_gameFS.findItems(path, FileExtensionMatcher(extension));
            
            MappedFile::List result;
            Path::List::const_iterator it, end;
            for (it = std::begin(paths), end = std::end(paths); it != end; ++it)
                result.push_back(m_gameFS.openFile(*it));
            
            return result;
        }
    }
}
