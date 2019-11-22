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

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/util.h>

#include <functional>
#include <list>
#include <memory>
#include <vector>

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
        class PortalFile;
        class TagManager;
        class SmartTag;
    }

    namespace View {
        class Action;
        class Command;
        class Grid;
        class MapViewConfig;
        class Selection;
        class UndoableCommand;
        class ViewEffectsService;

        class MapDocument : public Model::MapFacade, public CachingLogger {
        public:
            static const vm::bbox3 DefaultWorldBounds;
            static const String DefaultDocumentName;
        protected:
            vm::bbox3 m_worldBounds;
            Model::GameSPtr m_game;
            std::unique_ptr<Model::World> m_world;

            std::unique_ptr<Model::PointFile> m_pointFile;
            std::unique_ptr<Model::PortalFile> m_portalFile;
            IO::Path m_pointFilePath;
            IO::Path m_portalFilePath;

            std::unique_ptr<Assets::EntityDefinitionManager> m_entityDefinitionManager;
            std::unique_ptr<Assets::EntityModelManager> m_entityModelManager;
            std::unique_ptr<Assets::TextureManager> m_textureManager;
            std::unique_ptr<Model::TagManager> m_tagManager;

            std::unique_ptr<Model::EditorContext> m_editorContext;
            std::unique_ptr<MapViewConfig> m_mapViewConfig;
            std::unique_ptr<Grid> m_grid;

            using ActionList = std::vector<std::unique_ptr<Action>>;
            ActionList m_tagActions;
            ActionList m_entityDefinitionActions;

            IO::Path m_path;
            size_t m_lastSaveModificationCount;
            size_t m_modificationCount;

            Model::NodeCollection m_partiallySelectedNodes;
            Model::NodeCollection m_selectedNodes;
            Model::BrushFaceList m_selectedBrushFaces;

            Model::Layer* m_currentLayer;
            String m_currentTextureName;
            vm::bbox3 m_lastSelectionBounds;
            mutable vm::bbox3 m_selectionBounds;
            mutable bool m_selectionBoundsValid;

            ViewEffectsService* m_viewEffectsService;
        public: // notification
            Notifier<Command::Ptr> commandDoNotifier;
            Notifier<Command::Ptr> commandDoneNotifier;
            Notifier<Command::Ptr> commandDoFailedNotifier;
            Notifier<UndoableCommand::Ptr> commandUndoNotifier;
            Notifier<UndoableCommand::Ptr> commandUndoneNotifier;
            Notifier<UndoableCommand::Ptr> commandUndoFailedNotifier;
            Notifier<const String&> transactionDoneNotifier;
            Notifier<const String&> transactionUndoneNotifier;

            Notifier<MapDocument*> documentWillBeClearedNotifier;
            Notifier<MapDocument*> documentWasClearedNotifier;
            Notifier<MapDocument*> documentWasNewedNotifier;
            Notifier<MapDocument*> documentWasLoadedNotifier;
            Notifier<MapDocument*> documentWasSavedNotifier;
            Notifier<> documentModificationStateDidChangeNotifier;

            Notifier<> editorContextDidChangeNotifier;
            Notifier<> mapViewConfigDidChangeNotifier;
            Notifier<const Model::Layer*> currentLayerDidChangeNotifier;
            Notifier<const String&> currentTextureNameDidChangeNotifier;

            Notifier<> selectionWillChangeNotifier;
            Notifier<const Selection&> selectionDidChangeNotifier;

            Notifier<const Model::NodeList&> nodesWereAddedNotifier;
            Notifier<const Model::NodeList&> nodesWillBeRemovedNotifier;
            Notifier<const Model::NodeList&> nodesWereRemovedNotifier;
            Notifier<const Model::NodeList&> nodesWillChangeNotifier;
            Notifier<const Model::NodeList&> nodesDidChangeNotifier;

            Notifier<const Model::NodeList&> nodeVisibilityDidChangeNotifier;
            Notifier<const Model::NodeList&> nodeLockingDidChangeNotifier;

            Notifier<Model::Group*> groupWasOpenedNotifier;
            Notifier<Model::Group*> groupWasClosedNotifier;

            Notifier<const Model::BrushFaceList&> brushFacesDidChangeNotifier;

            Notifier<> textureCollectionsWillChangeNotifier;
            Notifier<> textureCollectionsDidChangeNotifier;

            Notifier<> entityDefinitionsDidChangeNotifier;
            Notifier<> modsDidChangeNotifier;

            Notifier<> pointFileWasLoadedNotifier;
            Notifier<> pointFileWasUnloadedNotifier;

            Notifier<> portalFileWasLoadedNotifier;
            Notifier<> portalFileWasUnloadedNotifier;
        protected:
            MapDocument();
        public:
            ~MapDocument() override;
        public: // accessors and such
            Logger& logger();

            Model::GameSPtr game() const override;
            const vm::bbox3& worldBounds() const;
            Model::World* world() const;

            bool isGamePathPreference(const IO::Path& path) const;

            Model::Layer* currentLayer() const override;
            void setCurrentLayer(Model::Layer* currentLayer);

            Model::Group* currentGroup() const override;
            Model::Node* currentParent() const override;

            Model::EditorContext& editorContext() const;

            Assets::EntityDefinitionManager& entityDefinitionManager() override;
            Assets::EntityModelManager& entityModelManager() override;
            Assets::TextureManager& textureManager() override;

            MapViewConfig& mapViewConfig() const;
            Grid& grid() const;

            Model::PointFile* pointFile() const;
            Model::PortalFile* portalFile() const;

            void setViewEffectsService(ViewEffectsService* viewEffectsService);
        public: // tag and entity definition actions
            using ActionVisitor = std::function<void(const Action&)>;
            void visitTagActions(const ActionVisitor& visitor) const;
            void visitEntityDefinitionActions(const ActionVisitor& visitor) const;
        private: // tag and entity definition actions
            void createTagActions();
            void createEntityDefinitionActions();
            void visitActions(const ActionVisitor& visitor, const ActionList& actions) const;
        public: // new, load, save document
            void newDocument(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, Model::GameSPtr game);
            void loadDocument(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, Model::GameSPtr game, const IO::Path& path);
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
            // cppcheck-suppress passedByValue
            void loadPointFile(const IO::Path path);
            bool isPointFileLoaded() const;
            bool canReloadPointFile() const;
            void reloadPointFile();
            void unloadPointFile();
        public: // portal file management
            // cppcheck-suppress passedByValue
            void loadPortalFile(const IO::Path path);
            bool isPortalFileLoaded() const;
            bool canReloadPortalFile() const;
            void reloadPortalFile();
            void unloadPortalFile();
        public: // selection
            bool hasSelection() const override;
            bool hasSelectedNodes() const override;
            bool hasSelectedBrushFaces() const override;

            const Model::AttributableNodeList allSelectedAttributableNodes() const override;
            const Model::NodeCollection& selectedNodes() const override;
            const Model::BrushFaceList allSelectedBrushFaces() const override;
            const Model::BrushFaceList& selectedBrushFaces() const override;

            const vm::bbox3& referenceBounds() const override;
            const vm::bbox3& lastSelectionBounds() const override;
            const vm::bbox3& selectionBounds() const override;
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
            void selectFacesWithTexture(const Assets::Texture* texture);

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
        public: // entity management
            Model::Entity* createPointEntity(const Assets::PointEntityDefinition* definition, const vm::vec3& delta) override;
            Model::Entity* createBrushEntity(const Assets::BrushEntityDefinition* definition) override;
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
            void hide(Model::NodeList nodes) override; // Don't take the nodes by reference!
            void hideSelection();
            void show(const Model::NodeList& nodes) override;
            void showAll();
            void ensureVisible(const Model::NodeList& nodes);
            void resetVisibility(const Model::NodeList& nodes) override;

            void lock(const Model::NodeList& nodes) override;
            void unlock(const Model::NodeList& nodes) override;
            void resetLock(const Model::NodeList& nodes) override;
        public: // modifying objects, declared in MapFacade interface
            bool translateObjects(const vm::vec3& delta) override;
            bool rotateObjects(const vm::vec3& center, const vm::vec3& axis, FloatType angle) override;
            bool scaleObjects(const vm::bbox3& oldBBox, const vm::bbox3& newBBox) override;
            bool scaleObjects(const vm::vec3& center, const vm::vec3& scaleFactors) override;
            bool shearObjects(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta) override;
            bool flipObjects(const vm::vec3& center, vm::axis::type axis) override;
        public: // CSG operations, declared in MapFacade interface
            bool createBrush(const std::vector<vm::vec3>& points);
            bool csgConvexMerge();
            bool csgSubtract();
            bool csgIntersect();
            bool csgHollow();
        public: // Clipping operations, declared in MapFacade interface
            bool clipBrushes(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3);
        public: // modifying entity attributes, declared in MapFacade interface
            bool setAttribute(const Model::AttributeName& name, const Model::AttributeValue& value) override;
            bool renameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName) override;
            bool removeAttribute(const Model::AttributeName& name) override;

            bool convertEntityColorRange(const Model::AttributeName& name, Assets::ColorRange::Type range) override;
            bool updateSpawnflag(const Model::AttributeName& name, const size_t flagIndex, const bool setFlag) override;
        public: // brush resizing, declared in MapFacade interface
            bool resizeBrushes(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) override;
        public: // modifying face attributes, declared in MapFacade interface
            void setTexture(Assets::Texture* texture) override;
        private:
            bool hasTexture(const Model::BrushFaceList& faces, Assets::Texture* texture) const;
        public:
            bool setFaceAttributes(const Model::BrushFaceAttributes& attributes) override;
            bool setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) override;
            bool copyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
            bool moveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) override;
            bool rotateTextures(float angle) override;
            bool shearTextures(const vm::vec2f& factors) override;
        public: // modifying vertices, declared in MapFacade interface
            void rebuildBrushGeometry(const Model::BrushList& brushes) override;

            bool snapVertices(FloatType snapTo) override;
            bool findPlanePoints() override;

            MoveVerticesResult moveVertices(const Model::VertexToBrushesMap& vertices, const vm::vec3& delta) override;
            bool moveEdges(const Model::EdgeToBrushesMap& edges, const vm::vec3& delta) override;
            bool moveFaces(const Model::FaceToBrushesMap& faces, const vm::vec3& delta) override;

            bool addVertices(const Model::VertexToBrushesMap& vertices);
            bool removeVertices(const Model::VertexToBrushesMap& vertices);
            bool removeEdges(const Model::EdgeToBrushesMap& edges);
            bool removeFaces(const Model::FaceToBrushesMap& faces);
        private: // subclassing interface for certain operations which are available from this class, but can only be implemented in a subclass
            virtual void performRebuildBrushGeometry(const Model::BrushList& brushes) = 0;
        public: // debug commands
            void printVertices();
            bool throwExceptionDuringCommand();
        public: // command processing
            bool canUndoLastCommand() const;
            bool canRedoNextCommand() const;
            const String& lastCommandName() const;
            const String& nextCommandName() const;
            void undoLastCommand();
            void redoNextCommand();
            bool hasRepeatableCommands() const;
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
            virtual bool doHasRepeatableCommands() const = 0;
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
            void pick(const vm::ray3& pickRay, Model::PickResult& pickResult) const;
            Model::NodeList findNodesContaining(const vm::vec3& point) const;
        private: // world management
            void createWorld(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, Model::GameSPtr game);
            void loadWorld(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, Model::GameSPtr game, const IO::Path& path);
            void clearWorld();
        public: // asset management
            Assets::EntityDefinitionFileSpec entityDefinitionFile() const;
            Assets::EntityDefinitionFileSpec::List allEntityDefinitionFiles() const;
            void setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec);

            // For testing
            void setEntityDefinitions(const Assets::EntityDefinitionList& definitions);

            IO::Path::List enabledTextureCollections() const;
            IO::Path::List availableTextureCollections() const;
            void setEnabledTextureCollections(const IO::Path::List& paths);
            void reloadTextureCollections();

            void reloadEntityDefinitions();
        private:
            void loadAssets();
            void unloadAssets();

            void loadEntityDefinitions();
            void unloadEntityDefinitions();

            void loadEntityModels();
            void unloadEntityModels();
        protected:
            void reloadTextures();
            void loadTextures();
            void unloadTextures();

            class SetTextures;
            class UnsetTextures;
            void setTextures();
            void setTextures(const Model::NodeList& nodes);
            void setTextures(const Model::BrushFaceList& faces);
            void unsetTextures();
            void unsetTextures(const Model::NodeList& nodes);

            class SetEntityDefinitions;
            class UnsetEntityDefinitions;
            void setEntityDefinitions();
            void setEntityDefinitions(const Model::NodeList& nodes);
            void unsetEntityDefinitions();
            void unsetEntityDefinitions(const Model::NodeList& nodes);
            void reloadEntityDefinitionsInternal();

            void clearEntityModels();

            class SetEntityModels;
            class UnsetEntityModels;
            void setEntityModels();
            void setEntityModels(const Model::NodeList& nodes);
            void unsetEntityModels();
            void unsetEntityModels(const Model::NodeList& nodes);
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
        public: // tag management
            void registerSmartTags(); // public for testing
            const std::list<Model::SmartTag>& smartTags() const;
            bool isRegisteredSmartTag(const String& name) const;
            const Model::SmartTag& smartTag(const String& name) const;
            bool isRegisteredSmartTag(size_t index) const;
            const Model::SmartTag& smartTag(size_t index) const;
        private:
            class InitializeNodeTagsVisitor;
            class ClearNodeTagsVisitor;
            void initializeNodeTags(MapDocument* document);
            void initializeNodeTags(const Model::NodeList& nodes);
            void clearNodeTags(const Model::NodeList& nodes);
            void updateNodeTags(const Model::NodeList& nodes);

            class InitializeFaceTagsVisitor;
            void updateFaceTags(const Model::BrushFaceList& faces);
            void updateAllFaceTags();
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
            explicit Transaction(MapDocumentWPtr document, const String& name = "");
            explicit Transaction(MapDocumentSPtr document, const String& name = "");
            explicit Transaction(MapDocument* document, const String& name = "");
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
