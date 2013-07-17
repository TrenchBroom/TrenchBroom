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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/Map.h"
#include "Model/Picker.h"
#include "Model/TextureManager.h"
#include "View/CachingLogger.h"

namespace TrenchBroom {
    namespace View {
        class Logger;

        class MapDocument : public CachingLogger {
        public:
            typedef std::tr1::shared_ptr<MapDocument> Ptr;
            static const BBox3 DefaultWorldBounds;
        private:
            typedef std::tr1::weak_ptr<MapDocument> WkPtr;
            
            BBox3 m_worldBounds;
            IO::Path m_path;
            Model::Game::Ptr m_game;
            Model::Map::Ptr m_map;
            Model::TextureManager m_textureManager;
            Model::Picker m_picker;
            
            size_t m_modificationCount;
        public:
            static MapDocument::Ptr newMapDocument();
            ~MapDocument();
            
            const IO::Path& path() const;
            String filename() const;

            Model::Game::Ptr game() const;
            Model::Map::Ptr map() const;
            
            bool modified() const;
            void incModificationCount();
            void decModificationCount();
            void clearModificationCount();
            
            void newDocument(const BBox3& worldBounds, Model::Game::Ptr game);
            void openDocument(const BBox3& worldBounds, Model::Game::Ptr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            
            void commitPendingRenderStateChanges();

            void commandDo(Controller::Command::Ptr command);
            void commandDone(Controller::Command::Ptr command);
            void commandDoFailed(Controller::Command::Ptr command);
            void commandUndo(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);
            void commandUndoFailed(Controller::Command::Ptr command);

            Model::PickResult pick(const Ray3& ray);
        private:
            MapDocument();
            
            void loadAndUpdateTextures();
            void loadTextures();
            void updateTextures();
            void doSaveDocument(const IO::Path& path);
            void setDocumentPath(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
