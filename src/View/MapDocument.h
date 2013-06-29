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

#ifndef __TrenchBroom__MapDocument__
#define __TrenchBroom__MapDocument__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/Map.h"
#include "View/CachingLogger.h"

namespace TrenchBroom {
    namespace View {
        class MapFrame;

        class MapDocument {
        public:
            typedef std::tr1::shared_ptr<MapDocument> Ptr;
        private:
            typedef std::tr1::weak_ptr<MapDocument> WkPtr;
            
            Model::Game::Ptr m_game;
            Model::Map::Ptr m_map;
            IO::Path m_path;
            
            size_t m_modificationCount;
        public:
            static MapDocument::Ptr newMapDocument();
            ~MapDocument();
            
            Model::Game::Ptr game() const;
            const IO::Path& path() const;
            String filename() const;
            
            bool isModified() const;
            void incModificationCount();
            void decModificationCount();
            void clearModificationCount();
            
            bool newDocument(Model::Game::Ptr game);
            bool openDocument(Model::Game::Ptr game, const IO::Path& path);
            bool saveDocument();
            bool saveDocumentAs(const IO::Path& path);
        private:
            MapDocument();
            
            bool doSaveDocument(const IO::Path& path);
            void setDocumentPath(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
