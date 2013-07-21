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
#include "Model/EntityDefinitionManager.h"
#include "Model/ModelTypes.h"
#include "Model/Picker.h"
#include "Model/Selection.h"
#include "Model/TextureManager.h"
#include "View/CachingLogger.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class Logger;

        class MapDocument : public CachingLogger {
        public:
            static const BBox3 DefaultWorldBounds;
        private:
            typedef std::tr1::weak_ptr<MapDocument> WkPtr;
            
            BBox3 m_worldBounds;
            IO::Path m_path;
            Model::GamePtr m_game;
            Model::Map* m_map;
            Model::Selection m_selection;
            Model::EntityDefinitionManager m_entityDefinitionManager;
            Model::TextureManager m_textureManager;
            Model::Picker m_picker;
            
            size_t m_modificationCount;
        public:
            static MapDocumentPtr newMapDocument();
            ~MapDocument();
            
            const IO::Path& path() const;
            String filename() const;

            Model::GamePtr game() const;
            Model::Map* map() const;
            
            bool modified() const;
            void incModificationCount();
            void decModificationCount();
            void clearModificationCount();
            
            void newDocument(const BBox3& worldBounds, Model::GamePtr game);
            void openDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            
            Model::ObjectList selectedObjects() const;
            Model::EntityList selectedEntities() const;
            Model::BrushList selectedBrushes() const;
            Model::BrushFaceList selectedFaces() const;
            void selectObjects(const Model::ObjectList& objects);
            void deselectObjects(const Model::ObjectList& objects);
            void selectAllObjects();
            void selectFaces(const Model::BrushFaceList& faces);
            void deselectFaces(const Model::BrushFaceList& faces);
            void deselectAll();
            
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
            
            void loadAndUpdateEntityDefinitions();
            void loadEntityDefinitions();
            void updateEntityDefinitions();

            void loadAndUpdateTextures();
            void loadTextures();
            void updateTextures();
            
            void doSaveDocument(const IO::Path& path);
            void setDocumentPath(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
