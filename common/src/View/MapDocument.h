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
#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/Path.h"
#include "Model/EntityColor.h"
#include "Model/MapFacade.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
#include "Model/NodeCollection.h"
#include "View/CachingLogger.h"
#include "View/ViewTypes.h"

class Color;
namespace TrenchBroom {
    namespace Assets {
        class EntityDefinitionManager;
        class EntityModelManager;
        class TextureManager;
    }
    
    namespace Model {
        class BrushFaceAttributes;
        class ChangeBrushFaceAttributesRequest;
        class EditorContext;
        class PickResult;
        class PointFile;
    }
    
    namespace View {
        class Command;
        class Grid;
        class MapViewConfig;
        class Selection;
        class UndoableCommand;
        class VertexHandleManager;
        
        class MapDocument : public Model::MapFacade, public CachingLogger {
        public:
            static const BBox3 DefaultWorldBounds;
            static const String DefaultDocumentName;
        protected:
            BBox3 m_worldBounds;
            Model::GamePtr m_game;
            Model::World* m_world;
            Model::Layer* m_currentLayer;
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
            
            String m_currentTextureName;
            BBox3 m_lastSelectionBounds;
            mutable BBox3 m_selectionBounds;
            mutable bool m_selectionBoundsValid;
        public: // notification
            Notifier1<Command*> commandDoNotifier;
            Notifier1<Command*> commandDoneNotifier;
            Notifier1<Command*> commandDoFailedNotifier;
            Notifier1<Command*> commandUndoNotifier;
            Notifier1<Command*> commandUndoneNotifier;
            Notifier1<Command*> commandUndoFailedNotifier;
            
            Notifier1<MapDocument*> documentWillBeClearedNotifier;
            Notifier1<MapDocument*> documentWasClearedNotifier;
            Notifier1<MapDocument*> documentWasNewedNotifier;
            Notifier1<MapDocument*> documentWasLoadedNotifier;
            Notifier1<MapDocument*> documentWasSavedNotifier;
            Notifier0 documentModificationStateDidChangeNotifier;
            
            Notifier0 selectionWillChangeNotifier;
            Notifier1<const Selection&> selectionDidChangeNotifier;
            
            Notifier1<const Model::NodeList&> nodesWereAddedNotifier;
            Notifier1<const Model::NodeList&> nodesWillBeRemovedNotifier;
            Notifier1<const Model::NodeList&> nodesWereRemovedNotifier;
            Notifier1<const Model::NodeList&> nodesWillChangeNotifier;
            Notifier1<const Model::NodeList&> nodesDidChangeNotifier;
            
            Notifier1<const Model::BrushFaceList&> brushFacesDidChangeNotifier;
            
            Notifier0 textureCollectionsDidChangeNotifier;
            Notifier0 entityDefinitionsDidChangeNotifier;
            Notifier0 modsDidChangeNotifier;
            
            Notifier0 pointFileWasLoadedNotifier;
            Notifier0 pointFileWasUnloadedNotifier;
        protected:
            MapDocument();
        public:
            virtual ~MapDocument();
        public: // accessors and such
            Model::GamePtr game() const;
            const BBox3& worldBounds() const;
            Model::World* world() const;

            bool isGamePathPreference(const IO::Path& path) const;
            
            Model::Layer* currentLayer() const;
            void setCurrentLayer(Model::Layer* currentLayer);
            
            const Model::EditorContext& editorContext() const;
            bool textureLock();
            void setTextureLock(bool textureLock);
            
            Assets::EntityDefinitionManager& entityDefinitionManager();
            Assets::EntityModelManager& entityModelManager();
            Assets::TextureManager& textureManager();
            
            const MapViewConfig& mapViewConfig() const;
            Grid& grid() const;
            
            Model::PointFile* pointFile() const;
        public: // new, load, save document
            void newDocument(const BBox3& worldBounds, Model::GamePtr game, Model::MapFormat::Type mapFormat);
            void loadDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            void saveDocumentTo(const IO::Path& path);
        private:
            void doSaveDocument(const IO::Path& path);
            void clearDocument();
        public: // copy and paste
            String serializeSelectedNodes();
            String serializeSelectedBrushFaces();
            bool paste(const String& str);
        private:
            bool pasteNodes(const Model::NodeList& nodes);
            bool pasteBrushFaces(const Model::BrushFaceList& faces);
        public: // point file management
            bool canLoadPointFile() const;
            void loadPointFile();
            bool isPointFileLoaded() const;
            void unloadPointFile();
        public: // selection
            bool hasSelection() const;
            bool hasSelectedNodes() const;
            bool hasSelectedBrushFaces() const;

            const Model::AttributableNodeList allSelectedAttributableNodes() const;
            const Model::NodeCollection& selectedNodes() const;
            const Model::BrushFaceList allSelectedBrushFaces() const;
            const Model::BrushFaceList& selectedBrushFaces() const;

            const BBox3& referenceBounds() const;
            const BBox3& lastSelectionBounds() const;
            const BBox3& selectionBounds() const;
            const String& currentTextureName() const;
            
            void selectAllNodes();
            void selectSiblings();
            void selectTouching(bool del);
            void selectInside(bool del);
            void selectNodesWithFilePosition(const std::vector<size_t>& positions);
            void select(const Model::NodeList& nodes);
            void select(Model::Node* node);
            void select(const Model::BrushFaceList& faces);
            void select(Model::BrushFace* face);
            void convertToFaceSelection();
            
            void deselectAll();
            void deselect(Model::Node* node);
            void deselect(const Model::NodeList& nodes);
            void deselect(Model::BrushFace* face);
        protected:
            void invalidateSelectionBounds();
        private:
            void validateSelectionBounds() const;
            void clearSelection();
        public: // adding, removing, reparenting, and duplicating nodes, declared in MapFacade interface
            void addNode(Model::Node* node, Model::Node* parent);
            void removeNode(Model::Node* node);
            
            Model::NodeList addNodes(const Model::ParentChildrenMap& nodes);
            void removeNodes(const Model::NodeList& nodes);

            void reparentNodes(Model::Node* newParent, const Model::NodeList& children);
            void reparentNodes(const Model::ParentChildrenMap& nodes);
            bool deleteObjects();
            bool duplicateObjects();
        public: // modifying transient layer attributes, declared in MapFacade interface
            void setLayerHidden(Model::Layer* layer, bool hidden);
            void setLayerLocked(Model::Layer* layer, bool locked);
        public: // modifying objects, declared in MapFacade interface
            bool translateObjects(const Vec3& delta);
            bool rotateObjects(const Vec3& center, const Vec3& axis, FloatType angle);
            bool flipObjects(const Vec3& center, Math::Axis::Type axis);
        public: // modifying entity attributes, declared in MapFacade interface
            bool setAttribute(const Model::AttributeName& name, const Model::AttributeValue& value);
            bool renameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName);
            bool removeAttribute(const Model::AttributeName& name);
            
            bool convertEntityColorRange(const Model::AttributeName& name, Model::ColorRange::Type range);
        public: // brush resizing, declared in MapFacade interface
            bool resizeBrushes(const Model::BrushFaceList& faces, const Vec3& delta);
        public: // modifying face attributes, declared in MapFacade interface
            bool setTexture(Assets::Texture* texture);
            bool setFaceAttributes(const Model::BrushFaceAttributes& attributes);
            bool setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request);
            bool moveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta);
            bool rotateTextures(float angle);
            bool shearTextures(const Vec2f& factors);
        public: // modifying vertices, declared in MapFacade interface
            void rebuildBrushGeometry(const Model::BrushList& brushes);
            bool snapVertices(const Model::VertexToBrushesMap& vertices, size_t snapTo);
            bool findPlanePoints();
            
            MoveVerticesResult moveVertices(const Model::VertexToBrushesMap& vertices, const Vec3& delta);
            bool moveEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta);
            bool moveFaces(const Model::VertexToFacesMap& faces, const Vec3& delta);
            bool splitEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta);
            bool splitFaces(const Model::VertexToFacesMap& faces, const Vec3& delta);
        private: // subclassing interface for certain operations which are available from this class, but can only be implemented in a subclass
            virtual void performRebuildBrushGeometry(const Model::BrushList& brushes) = 0;
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
            void beginTransaction(const String& name = "");
            void rollbackTransaction();
            void commitTransaction();
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
            void pick(const Ray3& pickRay, Model::PickResult& pickResult) const;
        private: // world management
            void createWorld(const BBox3& worldBounds, Model::GamePtr game, Model::MapFormat::Type mapFormat);
            void loadWorld(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            void clearWorld();
        public: // asset management
            Assets::EntityDefinitionFileSpec entityDefinitionFile() const;
            Assets::EntityDefinitionFileSpec::List allEntityDefinitionFiles() const;
            void setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec);
            
            const StringList externalTextureCollectionNames() const;
            void addTextureCollection(const String& name);
            void moveTextureCollectionUp(const String& name);
            void moveTextureCollectionDown(const String& name);
            void removeTextureCollections(const StringList& names);
        private:
            void loadAssets();
            void unloadAssets();
            
            void loadEntityDefinitions();
            void unloadEntityDefinitions();
            
            
            void loadEntityModels();
            void unloadEntityModels();
            void clearEntityModels();
            
            void loadTextures();
            void loadBuiltinTextures();
            void loadExternalTextures();
            void unloadTextures();
        protected:
            void addExternalTextureCollections(const StringList& names);
            void updateExternalTextureCollectionProperty();
            
            void setEntityDefinitions();
            void setEntityDefinitions(const Model::NodeList& nodes);
            void unsetEntityDefinitions();
            
            void setEntityModels();
            void setEntityModels(const Model::NodeList& nodes);
            void unsetEntityModels();

            void setTextures();
            void setTextures(const Model::NodeList& nodes);
            void setTextures(const Model::BrushFaceList& faces);
            
            void unsetTextures();
        private: // search paths and mods
            IO::Path::List externalSearchPaths() const;
            void updateGameSearchPaths();
        public:
            StringList mods() const;
            void setMods(const StringList& mods);
        private: // issue management
            void registerIssueGenerators();
        public:
            void setIssueHidden(Model::Issue* issue, bool hidden);
        private:
            virtual void doSetIssueHidden(Model::Issue* issue, bool hidden) = 0;
        public: // document path
            const String filename() const;
            const IO::Path& path() const;
        private:
            void setPath(const IO::Path& path);
        public: // modification count
            bool modified() const;
        private:
            void clearModificationCount();
        private: // observers
            void bindObservers();
            void unbindObservers();
            void preferenceDidChange(const IO::Path& path);
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
            void commit();
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
