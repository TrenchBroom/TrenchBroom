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

#ifndef __TrenchBroom__MapDocument__
#define __TrenchBroom__MapDocument__

#include "Notifier.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/TextureManager.h"
#include "IO/Path.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
#include "View/CachingLogger.h"
#include "View/EditorContext.h"
#include "View/MapViewConfig.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace View {
        class MapDocument : public CachingLogger {
        public:
            static const BBox3 DefaultWorldBounds;
            static const String DefaultDocumentName;
        private:
            BBox3 m_worldBounds;
            Model::GamePtr m_game;
            Model::World* m_world;
            
            Assets::EntityDefinitionManager m_entityDefinitionManager;
            Assets::EntityModelManager m_entityModelManager;
            Assets::TextureManager m_textureManager;
            
            View::EditorContext m_editorContext;
            View::MapViewConfig m_mapViewConfig;
            
            IO::Path m_path;
            size_t m_modificationCount;
        public: // notification
            Notifier1<MapDocument*> documentWillBeClearedNotifier;
            Notifier1<MapDocument*> documentWasClearedNotifier;
            Notifier1<MapDocument*> documentWasNewedNotifier;
            Notifier1<MapDocument*> documentWasLoadedNotifier;
            Notifier1<MapDocument*> documentWasSavedNotifier;
        private:
            MapDocument();
        public:
            static MapDocumentSPtr newMapDocument();
            ~MapDocument();
        public: // accessors and such
            Model::World* world() const;
            const View::EditorContext& editorContext() const;
            const View::MapViewConfig& mapViewConfig() const;
        public: // new, load, save document
            void newDocument(const BBox3& worldBounds, Model::GamePtr game, Model::MapFormat::Type mapFormat);
            void loadDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            void saveDocumentTo(const IO::Path& path);
        private:
            void clearDocument();
        public: // asset state management
            void commitPendingAssets();
        private: // selection
            void clearSelection();
        private: // world management
            void createWorld(const BBox3& worldBounds, Model::GamePtr game, Model::MapFormat::Type mapFormat);
            void loadWorld(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void clearWorld();
        private: // asset management
            void loadAssets();
            void unloadAssets();
            
            void loadEntityDefinitions();
            void setEntityDefinitions();
            void unloadEntityDefinitions();
            Assets::EntityDefinitionFileSpec entityDefinitionFile() const;
            
            void loadEntityModels();
            void setEntityModels();
            void unloadEntityModels();
            
            void loadTextures();
            void loadBuiltinTextures();
            void loadExternalTextures();
            void setTextures();
            void unloadTextures();
            
            void addExternalTextureCollections(const StringList& names);
        private: // search paths and mods
            IO::Path::List externalSearchPaths() const;
            void updateGameSearchPaths();
            StringList mods() const;
        private: // issue management
            void registerIssueGenerators();
        public: // document path
            const String filename() const;
            const IO::Path& path() const;
        private:
            void setPath(const IO::Path& path);
        public: // modification count
            bool modified() const;
        private:
            void clearModificationCount();
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
