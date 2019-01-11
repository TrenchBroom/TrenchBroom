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

#include "TextureCollectionLoader.h"

#include "Logger.h"
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
        TextureCollectionLoader::TextureCollectionLoader(Logger* logger) :
        m_logger(logger) {}

        TextureCollectionLoader::~TextureCollectionLoader() = default;

        std::unique_ptr<Assets::TextureCollection> TextureCollectionLoader::loadTextureCollection(const Path& path, const StringList& textureExtensions, const TextureReader& textureReader) {
            auto collection = std::make_unique<Assets::TextureCollection>(path);

            for (auto file : doFindTextures(path, textureExtensions)) {
                auto* texture = textureReader.readTexture(file);
                collection->addTexture(texture);
            }
            
            return collection;
        }

        FileTextureCollectionLoader::FileTextureCollectionLoader(Logger* logger, const IO::Path::List& searchPaths) :
        TextureCollectionLoader(logger),
        m_searchPaths(searchPaths) {}

        MappedFile::List FileTextureCollectionLoader::doFindTextures(const Path& path, const StringList& extensions) {
            const auto wadPath = Disk::resolvePath(m_searchPaths, path);
            WadFileSystem wadFS(wadPath);
            const auto texturePaths = wadFS.findItems(Path(""), FileExtensionMatcher(extensions));
            
            MappedFile::List result;
            result.reserve(texturePaths.size());

            for (const auto& texturePath : texturePaths)  {
                try {
                    result.push_back(wadFS.openFile(texturePath));
                } catch (const std::exception& e) {
                    m_logger->warn() << e.what();
                }
            }

            return result;
        }

        DirectoryTextureCollectionLoader::DirectoryTextureCollectionLoader(Logger* logger, const FileSystem& gameFS) :
        TextureCollectionLoader(logger),
        m_gameFS(gameFS) {}

        MappedFile::List DirectoryTextureCollectionLoader::doFindTextures(const Path& path, const StringList& extensions) {
            const auto texturePaths = m_gameFS.findItems(path, FileExtensionMatcher(extensions));
            
            MappedFile::List result;
            result.reserve(texturePaths.size());

            for (const auto& texturePath : texturePaths) {
                try {
                    result.push_back(m_gameFS.openFile(texturePath));
                } catch (const std::exception& e) {
                    m_logger->warn() << e.what();
                }
            }

            return result;
        }
    }
}
