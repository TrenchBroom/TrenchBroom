/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__TextureCollectionCommand__
#define __TrenchBroom__TextureCollectionCommand__

#include "Controller/Command.h"

#include "Utility/String.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace Controller {
        class TextureCollectionCommand : public DocumentCommand {
        public:
            typedef std::vector<size_t> IndexList;
        protected:
            StringList m_paths;
            IndexList m_indices;
            String m_mruTextureName;
            
            void addTextureCollectionsByPaths();
            void removeTextureCollectionsByPaths();
            void updateWadKey();
            
            TextureCollectionCommand(Type type, Model::MapDocument& document, const String& name, const String& path);
            TextureCollectionCommand(Type type, Model::MapDocument& document, const String& name, const IndexList& indices);
            
            bool performDo();
            bool performUndo();
        public:
            static TextureCollectionCommand* addTextureWad(Model::MapDocument& document, const String& path);
            static TextureCollectionCommand* removeTextureWads(Model::MapDocument& document, const IndexList& indices);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureCollectionCommand__) */
