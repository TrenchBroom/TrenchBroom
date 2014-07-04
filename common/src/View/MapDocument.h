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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Notifier.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "Hit.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/TextureManager.h"
#include "Controller/Command.h"
#include "IO/Path.h"
#include "Model/EntityDefinitionFileSpec.h"
#include "Model/IssueManager.h"
#include "Model/ModelFilter.h"
#include "Model/ModelTypes.h"
#include "Model/Picker.h"
#include "Model/PointFile.h"
#include "Model/Selection.h"
#include "View/CachingLogger.h"
#include "View/Grid.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Model {
        class BrushFace;
        class Object;
        class Selection;
        class SelectionResult;
    }
    
    namespace View {
        class MapDocument : public CachingLogger {
        public:
            static const BBox3 DefaultWorldBounds;
        private:
            typedef TrenchBroom::weak_ptr<MapDocument> WkPtr;
            
            BBox3 m_worldBounds;
            IO::Path m_path;
            Model::GamePtr m_game;
            Model::Map* m_map;
            Assets::EntityDefinitionManager m_entityDefinitionManager;
            Assets::EntityModelManager m_entityModelManager;
            Assets::TextureManager m_textureManager;
            Model::Picker m_picker;
            Model::ModelFilter m_filter;
            Model::Selection m_selection;
            Model::IssueManager m_issueManager;
            Model::PointFile m_pointFile;
            View::Grid m_grid;
            
            bool m_textureLock;
            size_t m_modificationCount;
        public:
            Notifier0 documentWasClearedNotifier;
            Notifier0 documentWasNewedNotifier;
            Notifier0 documentWasLoadedNotifier;
            Notifier0 documentWasSavedNotifier;
            
            Notifier0 pointFileWasLoadedNotifier;
            Notifier0 pointFileWasUnloadedNotifier;

            Notifier1<Model::Object*> objectWasAddedNotifier;
            Notifier1<Model::Object*> objectWillBeRemovedNotifier;
            Notifier1<Model::Object*> objectWasRemovedNotifier;
            Notifier1<Model::Object*> objectWillChangeNotifier;
            Notifier1<Model::Object*> objectDidChangeNotifier;
            
            Notifier3<Model::Entity*, const Model::EntityProperty&, const Model::EntityProperty&> entityPropertyDidChangeNotifier;
            
            Notifier1<Model::BrushFace*> faceWillChangeNotifier;
            Notifier1<Model::BrushFace*> faceDidChangeNotifier;
            
            Notifier0 modsDidChangeNotifier;
            Notifier0 entityDefinitionsDidChangeNotifier;
            Notifier0 textureCollectionsDidChangeNotifier;
            
            Notifier1<const Model::SelectionResult&> selectionDidChangeNotifier;
        public:
            static MapDocumentSPtr newMapDocument();
            ~MapDocument();
            
            const BBox3& worldBounds() const;
            const IO::Path& path() const;
            String filename() const;

            Model::GamePtr game() const;
            Model::Map* map() const;
            const Model::ModelFilter& filter() const;
            Model::ModelFilter& filter();
            Assets::EntityDefinitionManager& entityDefinitionManager();
            Assets::EntityModelManager& entityModelManager();
            Assets::TextureManager& textureManager();
            Model::IssueManager& issueManager();
            Model::PointFile& pointFile();
            View::Grid& grid();
            
            bool isGamePathPreference(const IO::Path& path) const;
            
            bool modified() const;
            void incModificationCount();
            void decModificationCount();
            void clearModificationCount();

            void newDocument(const BBox3& worldBounds, Model::GamePtr game, Model::MapFormat::Type mapFormat);
            void openDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            
            bool canLoadPointFile() const;
            void loadPointFile();
            bool isPointFileLoaded() const;
            void unloadPointFile();
            
            Model::EntityList parseEntities(const String& str) const;
            Model::BrushList parseBrushes(const String& str) const;
            Model::BrushFaceList parseFaces(const String& str) const;
            void writeObjectsToStream(const Model::ObjectList& objects, std::ostream& stream) const;
            void writeFacesToStream(const Model::BrushFaceList& faces, std::ostream& stream) const;
            
            Model::Entity* worldspawn();
            StringList mods() const;
            
            Model::EntityDefinitionFileSpec entityDefinitionFile() const;
            Model::EntityDefinitionFileSpec::List entityDefinitionFiles() const;
            
            void addExternalTextureCollections(const StringList& names);
            void removeExternalTextureCollections(const StringList& names);
            void moveExternalTextureCollectionUp(const String& name);
            void moveExternalTextureCollectionDown(const String& name);
            void updateExternalTextureCollectionProperty();

            void addObject(Model::Object* object, Model::Object* parent = NULL);
            void removeObject(Model::Object* object);

            bool hasSelectedObjects() const;
            bool hasSelectedEntities() const;
            bool hasSelectedBrushes() const;
            bool hasSelectedFaces() const;
            bool hasSelection() const;
            BBox3 selectionBounds() const;
            
            const Model::ObjectList& selectedObjects() const;
            const Model::EntityList& selectedEntities() const;
            const Model::BrushList& selectedBrushes() const;
            const Model::BrushFaceList& selectedFaces() const;
            
            Model::EntityList allSelectedEntities() const;
            Model::BrushList allSelectedBrushes() const;
            const Model::BrushFaceList& allSelectedFaces() const;

            Model::EntityList unselectedEntities() const;
            Model::BrushList unselectedBrushes() const;
            
            Model::SelectionResult selectObjects(const Model::ObjectList& objects);
            Model::SelectionResult deselectObjects(const Model::ObjectList& objects);
            Model::SelectionResult selectAllObjects();
            Model::SelectionResult selectAllFaces();
            Model::SelectionResult selectFaces(const Model::BrushFaceList& faces, bool keepBrushSelection);
            Model::SelectionResult deselectFaces(const Model::BrushFaceList& faces);
            Model::SelectionResult deselectAll();
            Assets::Texture* currentTexture() const;
            String currentTextureName() const;
            
            bool textureLock() const;
            void setTextureLock(const bool textureLock);
            
            void commitPendingRenderStateChanges();

            Hits pick(const Ray3& ray);
            
            void saveBackup(const IO::Path& path);
        private:
            MapDocument();
            
            void bindObservers();
            void unbindObservers();
            
            void registerIssueGenerators();
            void reloadIssues();
            
            void objectWasAdded(Model::Object* object);
            void objectWillBeRemoved(Model::Object* object);
            void objectWasRemoved(Model::Object* object);
            void objectWillChange(Model::Object* object);
            void objectDidChange(Model::Object* object);
            void updateLinkSourcesInIssueManager(Model::Entity* entity);
            
            void entityPropertyDidChange(Model::Entity* entity, const Model::EntityProperty& before, const Model::EntityProperty& after);
            
            void faceDidChange(Model::BrushFace* face);
            
            void modsDidChange();
            void entityDefinitionsDidChange();
            void textureCollectionsDidChange();
            void preferenceDidChange(const IO::Path& path);
            
            void addEntity(Model::Entity* entity);
            void addBrush(Model::Brush* brush, Model::Entity* entity);
            void removeEntity(Model::Entity* entity);
            void removeBrush(Model::Brush* brush, Model::Entity* entity);
            
            void clearMap();
            
            void updateGameSearchPaths();
            void loadAndUpdateEntityDefinitions();
            void loadEntityDefinitions();
            void clearEntityModels();
            void updateEntityDefinitions(const Model::EntityList& entities);
            void updateEntityDefinition(Model::Entity* entity);
            void updateEntityModels(const Model::EntityList& entities);
            void updateEntityModel(Model::Entity* entity);

            void loadAndUpdateTextures();
            void loadTextures();
            void loadBuiltinTextures();
            void loadExternalTextures();
            void updateTextures();
            void doAddExternalTextureCollections(const StringList& names);
            IO::Path::List externalSearchPaths() const;
            
            void doSaveDocument(const IO::Path& path);
            void setDocumentPath(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
