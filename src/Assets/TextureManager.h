/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__TextureManager__
#define __TrenchBroom__TextureManager__

#include "Assets/AssetTypes.h"
#include "Model/ModelTypes.h"
#include "IO/Path.h"

#include <map>

namespace TrenchBroom {
    namespace Assets {
        class TextureManager {
        private:
            typedef std::map<IO::Path, FaceTextureCollection*> TextureCollectionMap;
            typedef std::pair<IO::Path, FaceTextureCollection*> TextureCollectionMapEntry;
            typedef std::map<String, FaceTexture*> TextureMap;
            
            Model::GamePtr m_game;
            
            TextureCollectionList m_collections;
            TextureCollectionMap m_collectionsByPath;
            TextureCollectionMap m_toUpload;
            TextureCollectionMap m_toRemove;
            
            TextureMap m_texturesByName;
        public:
            ~TextureManager();

            void addTextureCollection(const IO::Path& path);
            void addTextureCollections(const IO::Path::List& paths);
            void removeTextureCollection(const size_t index);
            
            void reset(Model::GamePtr game);
            void commitChanges();
            
            FaceTexture* texture(const String& name) const;
        private:
            void doAddTextureCollection(const IO::Path& path, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath, TextureCollectionMap& toUpload, TextureCollectionMap& toRemove);
            void doAddTextureCollection(FaceTextureCollection* collection, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath, TextureCollectionMap& toUpload);
            void updateTextures();
        };
    }
}

#endif /* defined(__TrenchBroom__TextureManager__) */
