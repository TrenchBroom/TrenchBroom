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
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/TextureManager.h"
#include "Controller/Command.h"
#include "IO/Path.h"
#include "Model/Filter.h"
#include "Model/ModelTypes.h"
#include "Model/Picker.h"
#include "Model/Selection.h"
#include "View/CachingLogger.h"
#include "View/Grid.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Model {
        class SelectionResult;
    }
    
    namespace View {
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
            Assets::EntityDefinitionManager m_entityDefinitionManager;
            Assets::EntityModelManager m_entityModelManager;
            Assets::TextureManager m_textureManager;
            Model::Picker m_picker;
            Model::Filter m_filter;
            View::Grid m_grid;
            
            size_t m_modificationCount;
        public:
            static MapDocumentPtr newMapDocument();
            ~MapDocument();
            
            const BBox3& worldBounds() const;
            const IO::Path& path() const;
            String filename() const;

            Model::GamePtr game() const;
            Model::Map* map() const;
            const Model::Filter& filter() const;
            Assets::EntityDefinitionManager& entityDefinitionManager();
            Assets::EntityModelManager& entityModelManager();
            Assets::TextureManager& textureManager();
            View::Grid& grid();
            
            bool modified() const;
            void incModificationCount();
            void decModificationCount();
            void clearModificationCount();

            void newDocument(const BBox3& worldBounds, Model::GamePtr game);
            void openDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            
            bool hasSelectedObjects() const;
            bool hasSelectedFaces() const;
            bool hasSelection() const;
            Model::ObjectList selectedObjects() const;
            Model::EntityList selectedEntities() const;
            Model::EntityList allSelectedEntities() const;
            Model::EntityList unselectedEntities() const;
            Model::BrushList selectedBrushes() const;
            Model::BrushList unselectedBrushes() const;
            Model::BrushFaceList selectedFaces() const;
            Model::SelectionResult selectObjects(const Model::ObjectList& objects);
            Model::SelectionResult deselectObjects(const Model::ObjectList& objects);
            Model::SelectionResult selectAllObjects();
            Model::SelectionResult selectFaces(const Model::BrushFaceList& faces);
            Model::SelectionResult deselectFaces(const Model::BrushFaceList& faces);
            Model::SelectionResult deselectAll();
            Assets::FaceTexture* currentTexture() const;
            String currentTextureName() const;
            
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
            void updateEntityModels();

            void loadAndUpdateTextures();
            void loadTextures();
            void updateTextures();
            
            void doSaveDocument(const IO::Path& path);
            void setDocumentPath(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
