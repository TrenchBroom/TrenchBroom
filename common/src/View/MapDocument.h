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
#include "Hit.h"
#include "Assets/AssetTypes.h"
#include "IO/Path.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
#include "Model/NodeCollection.h"
#include "View/CachingLogger.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinitionManager;
        class EntityModelManager;
        class TextureManager;
    }
    
    namespace Model {
        class EditorContext;
        class PointFile;
    }
    
    namespace View {
        class Command;
        class Grid;
        class MapViewConfig;
        class Selection;
        class UndoableCommand;
        
        class MapDocument : public CachingLogger {
        public:
            static const BBox3 DefaultWorldBounds;
            static const String DefaultDocumentName;
        protected:
            BBox3 m_worldBounds;
            Model::GamePtr m_game;
            Model::World* m_world;
            Model::PointFile* m_pointFile;
            Model::EditorContext* m_editorContext;
            
            Assets::EntityDefinitionManager* m_entityDefinitionManager;
            Assets::EntityModelManager* m_entityModelManager;
            Assets::TextureManager* m_textureManager;
            
            MapViewConfig* m_mapViewConfig;
            Grid* m_grid;
            bool m_textureLock;
            
            IO::Path m_path;
            size_t m_modificationCount;

            Model::NodeCollection m_partiallySelectedNodes;
            Model::NodeCollection m_selectedNodes;
            Model::BrushFaceList m_selectedBrushFaces;
            
            mutable BBox3 m_selectionBounds;
            mutable bool m_selectionBoundsValid;
        public: // notification
            Notifier1<MapDocument*> documentWillBeClearedNotifier;
            Notifier1<MapDocument*> documentWasClearedNotifier;
            Notifier1<MapDocument*> documentWasNewedNotifier;
            Notifier1<MapDocument*> documentWasLoadedNotifier;
            Notifier1<MapDocument*> documentWasSavedNotifier;
            Notifier0 documentModificationStateDidChangeNotifier;
            
            Notifier1<Command*> commandProcessedNotifier;

            Notifier0 selectionWillChangeNotifier;
            Notifier1<const Selection&> selectionDidChangeNotifier;
            
            Notifier1<const Model::NodeList&> nodesWereAddedNotifier;
            Notifier1<const Model::NodeList&> nodesWillBeRemovedNotifier;
            Notifier1<const Model::NodeList&> nodesWereRemovedNotifier;
            Notifier1<const Model::NodeList&> nodesWillChangeNotifier;
            Notifier1<const Model::NodeList&> nodesDidChangeNotifier;
            
            Notifier1<const Model::BrushFaceList&> brushFacesDidChangeNotifier;
            
            Notifier0 pointFileWasLoadedNotifier;
            Notifier0 pointFileWasUnloadedNotifier;
        protected:
            MapDocument();
        public:
            virtual ~MapDocument();
        public: // accessors and such
            const BBox3& worldBounds() const;
            Model::World* world() const;
            const Model::EditorContext& editorContext() const;
            
            Assets::EntityModelManager& entityModelManager();
            
            const MapViewConfig& mapViewConfig() const;
            const Grid& grid() const;
        public: // new, load, save document
            void newDocument(const BBox3& worldBounds, Model::GamePtr game, Model::MapFormat::Type mapFormat);
            void loadDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            void saveDocumentTo(const IO::Path& path);
        private:
            void doSaveDocument(const IO::Path& path);
            void clearDocument();
        public: // point file management
            bool canLoadPointFile() const;
            void loadPointFile();
            bool isPointFileLoaded() const;
            void unloadPointFile();
        public: // selection
            bool hasSelection() const;
            bool hasSelectedNodes() const;
            bool hasSelectedBrushFaces() const;

            const Model::NodeCollection& selectedNodes() const;
            const Model::BrushFaceList& selectedBrushFaces() const;

            const BBox3& selectionBounds() const;
            
            void selectAllNodes();
            void selectTouching(bool del);
            void selectInside(bool del);
            void select(const Model::NodeList& nodes);
            void select(Model::Node* node);
            void select(const Model::BrushFaceList& faces);
            void select(Model::BrushFace* face);
            void convertToFaceSelection();
            
            void deselectAll();
            void deselect(Model::Node* node);
            void deselect(Model::BrushFace* face);
        protected:
            void invalidateSelectionBounds();
        private:
            void validateSelectionBounds() const;
            void clearSelection();
        public: // adding, removing, and duplicating objects
            bool deleteObjects();
            bool duplicateObjects();
        public: // modifying objects
            bool translateObjects(const Vec3& delta);
            bool rotateObjects(const Vec3& center, const Vec3& axis, FloatType angle);
            bool flipObjects(const Vec3& center, Math::Axis::Type axis);
        public: // modifying face attributes
            bool moveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta);
            bool rotateTextures(float angle);
        public: // command processing
            bool canUndoLastCommand() const;
            bool canRedoNextCommand() const;
            const String& lastCommandName() const;
            const String& nextCommandName() const;
            void undoLastCommand();
            void redoNextCommand();
            bool repeatLastCommands();
            void clearRepeatableCommands();
        public: // transactions
            void beginTransaction(const String& name);
            void endTransaction();
            void rollbackTransaction();
            void cancelTransaction();
        private:
            bool submit(UndoableCommand* command);
        private: // subclassing interface for command processing
            virtual bool doCanUndoLastCommand() const = 0;
            virtual bool doCanRedoNextCommand() const = 0;
            virtual const String& doGetLastCommandName() const = 0;
            virtual const String& doGetNextCommandName() const = 0;
            virtual void doUndoLastCommand() = 0;
            virtual void doRedoNextCommand() = 0;
            virtual bool doRepeatLastCommands() = 0;
            virtual void doClearRepeatableCommands() = 0;
            
            virtual void doBeginTransaction(const String& name) = 0;
            virtual void doEndTransaction() = 0;
            virtual void doRollbackTransaction() = 0;

            virtual bool doSubmit(UndoableCommand* command) = 0;
        public: // asset state management
            void commitPendingAssets();
        public: // picking
            Hits pick(const Ray3& pickRay) const;
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

        class Transaction {
        private:
            MapDocument* m_document;
            bool m_cancelled;
        public:
            Transaction(MapDocumentWPtr document, const String& name = "");
            Transaction(MapDocumentSPtr document, const String& name = "");
            Transaction(MapDocument* document, const String& name = "");
            ~Transaction();
            
            void rollback();
            void cancel();
        private:
            void begin(const String& name);
            void end();
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
