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

#ifndef TrenchBroom_TextureManager
#define TrenchBroom_TextureManager

#include "Assets/AssetTypes.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class TextureLoader;
    }
    
    namespace Assets {
        class TextureManager {
        public:
            typedef enum {
                SortOrder_Name,
                SortOrder_Usage
            } SortOrder;
            
            typedef std::pair<TextureCollection*, TextureList> Group;
            typedef std::vector<Group> GroupList;
        private:
            typedef std::map<IO::Path, TextureCollection*> TextureCollectionMap;
            typedef std::pair<IO::Path, TextureCollection*> TextureCollectionMapEntry;
            typedef std::map<String, Texture*> TextureMap;
            
            Logger* m_logger;
            
            TextureCollectionList m_collections;
            
            TextureCollectionList m_toPrepare;
            TextureCollectionList m_toRemove;
            
            TextureList m_sortedTextures[2];
            GroupList m_sortedGroups[2];
            
            TextureMap m_texturesByName;
            
            int m_minFilter;
            int m_magFilter;
            bool m_resetTextureMode;
        public:
            TextureManager(Logger* logger, int minFilter, int magFilter);
            ~TextureManager();

            void setTextureCollections(const IO::Path::List& paths, IO::TextureLoader& loader);
        private:
            TextureCollectionMap collectionMap() const;
            void addTextureCollection(Assets::TextureCollection* collection);
        public:
            void clear();
            
            void setTextureMode(int minFilter, int magFilter);
            void commitChanges();
            
            Texture* texture(const String& name) const;
            const TextureList& textures(const SortOrder sortOrder) const;
            const GroupList& groups(const SortOrder sortOrder) const;
            const TextureCollectionList& collections() const;
            const StringList collectionNames() const;
        private:
            void resetTextureMode();
            void prepare();

            void updateTextures();
            TextureList textureList() const;
        };
    }
}

#endif /* defined(TrenchBroom_TextureManager) */
