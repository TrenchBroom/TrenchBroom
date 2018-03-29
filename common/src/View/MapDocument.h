/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_MapDocument
#define TrenchBroom_MapDocument

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
#include "Model/TexCoordSystem.h"
#include "View/CachingLogger.h"
#include "View/UndoableCommand.h"
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
        class Group;
        class PickResult;
        class PointFile;
    }
    
    namespace View {
        class Command;
        class Grid;
        class MapViewConfig;
        class Selection;
        class UndoableCommand;
        class ViewEffectsService;
        
        class MapDocument : public Model::MapFacade, public CachingLogger {
        public:
            static const BBox3 DefaultWorldBounds;
            static const String DefaultDocumentName;
        protected:
            BBox3 m_worldBounds;
            Model::GameSPtr m_game;
            Model::World* m_world;
            Model::Layer* m_currentLayer;
            Model::PointFile* m_pointFile;
            Model::EditorContext* m_editorContext;
            
            Assets::EntityDefinitionManager* m_entityDefinitionManager;
            Assets::EntityModelManager* m_entityModelManager;
            Assets::TextureManager* m_textureManager;
            
            MapViewConfig* m_mapViewConfig;
            Grid* m_grid;
            
            IO::Path m_path;
            size_t m_lastSaveModificationCount;
            size_t m_modificationCount;

            Model::NodeCollection m_partiallySelectedNodes;
            Model::NodeCollection m_selectedNodes;
            Model::BrushFaceList m_selectedBrushFaces;
            
            String m_currentTextureName;
            BBox3 m_lastSelectionBounds;
            mutable BBox3 m_selectionBounds;
            mutable bool m_selectionBoundsValid;
            
            ViewEffectsService* m_viewEffectsService;
        public: // notification
            Notifier1<Command::Ptr> commandDoNotifier;
            Notifier1<Command::Ptr> commandDoneNotifier;
            Notifier1<Command::Ptr> commandDoFailedNotifier;
            Notifier1<UndoableCommand::Ptr> commandUndoNotifier;
            Notifier1<UndoableCommand::Ptr> commandUndoneNotifier;
            Notifier1<UndoableCommand::Ptr> commandUndoFailedNotifier;
            
            Notifier1<MapDocument*> documentWillBeClearedNotifier;
            Notifier1<MapDocument*> documentWasClearedNotifier;
            Notifier1<MapDocument*> documentWasNewedNotifier;
            Notifier1<MapDocument*> documentWasLoadedNotifier;
            Notifier1<MapDocument*> documentWasSavedNotifier;
            Notifier0 documentModificationStateDidChangeNotifier;
            
            Notifier0 editorContextDidChangeNotifier;
            Notifier0 mapViewConfigDidChangeNotifier;
            Notifier1<const Model::Layer*> currentLayerDidChangeNotifier;
            Notifier1<const String&> currentTextureNameDidChangeNotifier;
            
            Notifier0 selectionWillChangeNotifier;
            Notifier1<const Selection&> selectionDidChangeNotifier;
            
            Notifier1<const Model::NodeList&> nodesWereAddedNotifier;
            Notifier1<const Model::NodeList&> nodesWillBeRemovedNotifier;
            Notifier1<const Model::NodeList&> nodesWereRemovedNotifier;
            Notifier1<const Model::NodeList&> nodesWillChangeNotifier;
            Notifier1<const Model::NodeList&> nodesDidChangeNotifier;
            
            Notifier1<const Model::NodeList&> nodeVisibilityDidChangeNotifier;
            Notifier1<const Model::NodeList&> nodeLockingDidChangeNotifier;
            
            Notifier1<Model::Group*> groupWasOpenedNotifier;
            Notifier1<Model::Group*> groupWasClosedNotifier;
            
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
            Model::GameSPtr game() const;
            const BBox3& worldBounds() const;
            Model::World* world() const;

            bool isGamePathPreference(const IO::Path& path) const;
            
            Model::Layer* currentLayer() const;
            void setCurrentLayer(Model::Layer* currentLayer);
            
            Model::Group* currentGroup() const;
            Model::Node* currentParent() const;
            
            Model::EditorContext& editorContext() const;
            
            Assets::EntityDefinitionManager& entityDefinitionManager();
            Assets::EntityModelManager& entityModelManager();
            Assets::TextureManager& textureManager();
            
            MapViewConfig& mapViewConfig() const;
            Grid& grid() const;
            
            Model::PointFile* pointFile() const;
            
            void setViewEffectsService(ViewEffectsService* viewEffectsService);
        public: // new, load, save document
            void newDocument(Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GameSPtr game);
            void loadDocument(Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GameSPtr game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            void saveDocumentTo(const IO::Path& path);
            void exportDocumentAs(Model::ExportFormat format, const IO::Path& path);
        private:
            void doSaveDocument(const IO::Path& path);
            void clearDocument();
        public: // copy and paste
            String serializeSelectedNodes();
            String serializeSelectedBrushFaces();
            
            PasteType paste(const String& str);
        private:
            bool pasteNodes(const Model::NodeList& nodes);
            bool pasteBrushFaces(const Model::BrushFaceList& faces);
        public: // point file management
            void loadPointFile(const IO::Path& path);
            bool isPointFileLoaded() const;
            void unloadPointFile();
        public: // selection
            bool hasSelection() const override;
            bool hasSelectedNodes() const override;
            bool hasSelectedBrushFaces() const override;

            const Model::AttributableNodeList allSelectedAttributableNodes() const override;
            const Model::NodeCollection& selectedNodes() const override;
            const Model::BrushFaceList allSelectedBrushFaces() const override;
            const Model::BrushFaceList& selectedBrushFaces() const override;

            const BBox3& referenceBounds() const override;
            const BBox3& lastSelectionBounds() const override;
            const BBox3& selectionBounds() const override;
            const String& currentTextureName() const override;
            void setCurrentTextureName(const String& currentTextureName);
            
            void selectAllNodes() override;
            void selectSiblings() override;
            void selectTouching(bool del) override;
            void selectInside(bool del) override;
            void selectNodesWithFilePosition(const std::vector<size_t>& positions) override;
            void select(const Model::NodeList& nodes) override;
            void select(Model::Node* node) override;
            void select(const Model::BrushFaceList& faces) override;
            void select(Model::BrushFace* face) override;
            void convertToFaceSelection() override;
            
            void deselectAll() override;
            void deselect(Model::Node* node) override;
            void deselect(const Model::NodeList& nodes) override;
            void deselect(Model::BrushFace* face) override;
        protected:
            void updateLastSelectionBounds();
            void invalidateSelectionBounds();
        private:
            void validateSelectionBounds() const;
            void clearSelection();
        public: // adding, removing, reparenting, and duplicating nodes, declared in MapFacade interface
            void addNode(Model::Node* node, Model::Node* parent) override;
            void removeNode(Model::Node* node) override;

            Model::NodeList addNodes(const Model::ParentChildrenMap& nodes) override;
            Model::NodeList addNodes(const Model::NodeList& nodes, Model::Node* parent) override;
            
            void removeNodes(const Model::NodeList& nodes) override;
        private:
            Model::ParentChildrenMap collectRemovableParents(const Model::ParentChildrenMap& nodes) const;
            
            struct CompareByAncestry;
            Model::NodeList removeImplicitelyRemovedNodes(Model::NodeList nodes) const;
            
            void closeRemovedGroups(const Model::ParentChildrenMap& toRemove);
        public:
            bool reparentNodes(Model::Node* newParent, const Model::NodeList& children) override;
            bool reparentNodes(const Model::ParentChildrenMap& nodesToAdd) override;
        private:
            bool checkReparenting(const Model::ParentChildrenMap& nodesToAdd) const;
        public:
            bool deleteObjects() override;
            bool duplicateObjects() override;
        public: // group management
            Model::Group* groupSelection(const String& name);
            void mergeSelectedGroupsWithGroup(Model::Group* group);
        private:
            class MatchGroupableNodes;
            Model::NodeList collectGroupableNodes(const Model::NodeList& selectedNodes) const;
        public:
            void ungroupSelection();
            void renameGroups(const String& name);
            
            void openGroup(Model::Group* group);
            void closeGroup();
        public: // modifying transient node attributes, declared in MapFacade interface
            void isolate(const Model::NodeList& nodes);
            void hide(const Model::NodeList nodes) override; // Don't take the nodes by reference!
            void hideSelection();
            void show(const Model::NodeList& nodes) override;
            void showAll();
            void ensureVisible(const Model::NodeList& nodes);
            void resetVisibility(const Model::NodeList& nodes) override;
            
            void lock(const Model::NodeList& nodes) override;
            void unlock(const Model::NodeList& nodes) override;
            void resetLock(const Model::NodeList& nodes) override;
        public: // modifying objects, declared in MapFacade interface
            bool translateObjects(const Vec3& delta) override;
            bool rotateObjects(const Vec3& center, const Vec3& axis, FloatType angle) override;
            bool scaleObjects(const Vec3& center, const Vec3& scaleFactors) override;
            bool scaleObjectsBBox(const BBox3& oldBBox, const BBox3& newBBox) override;
            bool shearObjects(const BBox3& box, const Vec3& sideToShear, const Vec3& delta) override;
            bool flipObjects(const Vec3& center, Math::Axis::Type axis) override;
        public:
            bool createBrush(const Vec3::List& points);
            bool csgConvexMerge();
            bool csgSubtract();
            bool csgIntersect();
            bool csgHollow();
        public:
            bool clipBrushes(const Vec3& p1, const Vec3& p2, const Vec3& p3);
        public: // modifying entity attributes, declared in MapFacade interface
            bool setAttribute(const Model::AttributeName& name, const Model::AttributeValue& value) override;
            bool renameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName) override;
            bool removeAttribute(const Model::AttributeName& name) override;
            
            bool convertEntityColorRange(const Model::AttributeName& name, Assets::ColorRange::Type range) override;
            bool updateSpawnflag(const Model::AttributeName& name, const size_t flagIndex, const bool setFlag) override;
        public: // brush resizing, declared in MapFacade interface
            bool resizeBrushes(const Polygon3::List& faces, const Vec3& delta) override;
        public: // modifying face attributes, declared in MapFacade interface
            void setTexture(Assets::Texture* texture) override;
        private:
            bool hasTexture(const Model::BrushFaceList& faces, Assets::Texture* texture) const;
        public:
            bool setFaceAttributes(const Model::BrushFaceAttributes& attributes) override;
            bool setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) override;
            bool copyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot* coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const Plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
            bool moveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) override;
            bool rotateTextures(float angle) override;
            bool shearTextures(const Vec2f& factors) override;
        public: // modifying vertices, declared in MapFacade interface
            void rebuildBrushGeometry(const Model::BrushList& brushes) override;
            
            bool snapVertices(size_t snapTo) override;
            bool findPlanePoints() override;
            
            MoveVerticesResult moveVertices(const Model::VertexToBrushesMap& vertices, const Vec3& delta) override;
            bool moveEdges(const Model::EdgeToBrushesMap& edges, const Vec3& delta) override;
            bool moveFaces(const Model::FaceToBrushesMap& faces, const Vec3& delta) override;
            
            bool addVertices(const Model::VertexToBrushesMap& vertices);
            bool removeVertices(const Model::VertexToBrushesMap& vertices);
            bool removeEdges(const Model::EdgeToBrushesMap& edges);
            bool removeFaces(const Model::FaceToBrushesMap& faces);
        private: // subclassing interface for certain operations which are available from this class, but can only be implemented in a subclass
            virtual void performRebuildBrushGeometry(const Model::BrushList& brushes) = 0;
        public: // debug commands
            void printVertices();
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
            bool submit(Command::Ptr command);
            bool submitAndStore(UndoableCommand::Ptr command);
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

            virtual bool doSubmit(Command::Ptr command) = 0;
            virtual bool doSubmitAndStore(UndoableCommand::Ptr command) = 0;
        public: // asset state management
            void commitPendingAssets();
        public: // picking
            void pick(const Ray3& pickRay, Model::PickResult& pickResult) const;
            Model::NodeList findNodesContaining(const Vec3& point) const;
        private: // world management
            void createWorld(Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GameSPtr game);
            void loadWorld(Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GameSPtr game, const IO::Path& path);
            void clearWorld();
            void initializeWorld(const BBox3& worldBounds);
        public: // asset management
            Assets::EntityDefinitionFileSpec entityDefinitionFile() const;
            Assets::EntityDefinitionFileSpec::List allEntityDefinitionFiles() const;
            void setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec);
            
            IO::Path::List enabledTextureCollections() const;
            IO::Path::List availableTextureCollections() const;
            void setEnabledTextureCollections(const IO::Path::List& paths);
            void reloadTextureCollections();
        private:
            void loadAssets();
            void unloadAssets();
            
            void loadEntityDefinitions();
            void unloadEntityDefinitions();
            
            void loadEntityModels();
            void unloadEntityModels();
        protected:
            void loadTextures();
            void unloadTextures();
            void reloadTextures();
            
            void setEntityDefinitions();
            void setEntityDefinitions(const Model::NodeList& nodes);
            void unsetEntityDefinitions();
            void unsetEntityDefinitions(const Model::NodeList& nodes);
            void reloadEntityDefinitions();
            
            void clearEntityModels();

            void setTextures();
            void setTextures(const Model::NodeList& nodes);
            void setTextures(const Model::BrushFaceList& faces);
            
            void unsetTextures();
            void unsetTextures(const Model::NodeList& nodes);
        protected: // search paths and mods
            IO::Path::List externalSearchPaths() const;
            void updateGameSearchPaths();
        public:
            StringList mods() const override;
            void setMods(const StringList& mods) override;
            String defaultMod() const;
        private: // issue management
            void registerIssueGenerators();
        public:
            void setIssueHidden(Model::Issue* issue, bool hidden);
        private:
            virtual void doSetIssueHidden(Model::Issue* issue, bool hidden) = 0;
        public: // document path
            bool persistent() const;
            String filename() const;
            const IO::Path& path() const;
        private:
            void setPath(const IO::Path& path);
        public: // modification count
            bool modified() const;
            size_t modificationCount() const;
        private:
            void setLastSaveModificationCount();
            void clearModificationCount();
        private: // observers
            void bindObservers();
            void unbindObservers();
            void preferenceDidChange(const IO::Path& path);
            void commandDone(Command::Ptr command);
            void commandUndone(UndoableCommand::Ptr command);
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

#endif /* defined(TrenchBroom_MapDocument) */
