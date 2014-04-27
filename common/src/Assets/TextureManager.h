/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "IO/Path.h"
#include "Model/ModelTypes.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    class Logger;
    
    namespace Assets {
        class TextureCollectionSpec;
        
        class TextureManager {
        public:
            typedef enum {
                SortOrder_Name,
                SortOrder_Usage
            } SortOrder;
            
            typedef std::pair<TextureCollection*, TextureList> Group;
            typedef std::vector<Group> GroupList;
        private:
            typedef std::map<String, TextureCollection*> TextureCollectionMap;
            typedef std::pair<String, TextureCollection*> TextureCollectionMapEntry;
            typedef std::map<String, Texture*> TextureMap;
            
            Logger* m_logger;
            Model::GamePtr m_game;
            
            TextureCollectionList m_builtinCollections;
            TextureCollectionMap m_builtinCollectionsByName;
            
            TextureCollectionList m_externalCollections;
            TextureCollectionMap m_externalCollectionsByName;
            TextureCollectionMap m_toRemove;
            
            TextureCollectionList m_allCollections;
            TextureList m_sortedTextures[2];
            GroupList m_sortedGroups[2];
            
            TextureMap m_texturesByName;
        public:
            TextureManager(Logger* logger);
            ~TextureManager();

            void setBuiltinTextureCollections(const IO::Path::List& paths);
            
            bool addExternalTextureCollection(const TextureCollectionSpec& spec);
            void removeExternalTextureCollection(const String& name);
            void moveExternalTextureCollectionUp(const String& name);
            void moveExternalTextureCollectionDown(const String& name);
            
            void reset(Model::GamePtr game);
            void commitChanges();
            
            Texture* texture(const String& name) const;
            const TextureList& textures(const SortOrder sortOrder) const;
            const GroupList& groups(const SortOrder sortOrder) const;
            const TextureCollectionList& collections() const;
            const StringList externalCollectionNames() const;
        private:
            void doAddTextureCollection(const TextureCollectionSpec& spec, TextureCollectionList& collections, TextureCollectionMap& collectionsByName);
            
            void doRemoveTextureCollection(const String& name, TextureCollectionList& collections, TextureCollectionMap& collectionsByName, TextureCollectionMap& toRemove);
            
            void clear();
            void clearBuiltinTextureCollections();
            void clearExternalTextureCollections();
            void updateTextures();
            TextureList textureList() const;
        };
    }
}

#endif /* defined(__TrenchBroom__TextureManager__) */
