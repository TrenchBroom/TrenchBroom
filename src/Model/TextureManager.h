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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__TextureManager__
#define __TrenchBroom__TextureManager__

#include "IO/Path.h"
#include "Model/ModelTypes.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class Game;
        
        class TextureManager {
        private:
            typedef std::map<IO::Path, TextureCollection*> TextureCollectionMap;
            typedef std::pair<IO::Path, TextureCollection*> TextureCollectionMapEntry;
            typedef std::map<String, Texture*> TextureMap;
            
            GamePtr m_game;
            
            TextureCollectionList m_collections;
            TextureCollectionMap m_collectionsByPath;
            TextureCollectionMap m_toUpload;
            TextureCollectionMap m_toRemove;
            
            TextureMap m_texturesByName;
        public:
            void addTextureCollection(const IO::Path& path);
            void addTextureCollections(const IO::Path::List& paths);
            void removeTextureCollection(const size_t index);
            
            void reset(GamePtr game);
            void commitChanges();
            
            Texture* texture(const String& name) const;
        private:
            void doAddTextureCollection(const IO::Path& path, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath, TextureCollectionMap& toUpload, TextureCollectionMap& toRemove);
            void doAddTextureCollection(TextureCollection* collection, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath, TextureCollectionMap& toUpload);
            void updateTextures();
        };
    }
}

#endif /* defined(__TrenchBroom__TextureManager__) */
