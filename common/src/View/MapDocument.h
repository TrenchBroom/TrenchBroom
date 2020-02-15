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

#include "FloatType.h"
#include "Notifier.h"
#include "IO/Path.h"
#include "Model/MapFacade.h"
#include "Model/NodeCollection.h"
#include "View/CachingLogger.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/util.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    class Color;

    namespace Assets {
        class EntityDefinition;
        class EntityDefinitionFileSpec;
        class EntityDefinitionManager;
        class EntityModelManager;
        class Texture;
        class TextureManager;
    }

    namespace Model {
        class BrushFaceAttributes;
        class EditorContext;
        enum class ExportFormat;
        class Game;
        class Issue;
        enum class MapFormat;
        class PickResult;
        class PointFile;
        class PortalFile;
        class SmartTag;
        class TagManager;
        class TexCoordSystemSnapshot;
        class World;
        enum class WrapStyle;
    }

    namespace View {
        class Action;
        class Command;
        class CommandResult;
        class Grid;
        class MapViewConfig;
        enum class PasteType;
        class Selection;
        class UndoableCommand;
        class ViewEffectsService;

        class MapDocument : public Model::MapFacade, public CachingLogger {
        public:
            static const vm::bbox3 DefaultWorldBounds;
            static const std::string DefaultDocumentName;
        protected:
            vm::bbox3 m_worldBounds;
            std::shared_ptr<Model::Game> m_game;
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
            std::vector<Model::BrushFace*> m_selectedBrushFaces;

            Model::Layer* m_currentLayer;
            std::string m_currentTextureName;
            vm::bbox3 m_lastSelectionBounds;
            mutable vm::bbox3 m_selectionBounds;
            mutable bool m_selectionBoundsValid;

            ViewEffectsService* m_viewEffectsService;
        public: // notification
            Notifier<Command*> commandDoNotifier;
            Notifier<Command*> commandDoneNotifier;
            Notifier<Command*> commandDoFailedNotifier;
            Notifier<UndoableCommand*> commandUndoNotifier;
            Notifier<UndoableCommand*> commandUndoneNotifier;
            Notifier<UndoableCommand*> commandUndoFailedNotifier;
            Notifier<const std::string&> transactionDoneNotifier;
            Notifier<const std::string&> transactionUndoneNotifier;

            Notifier<MapDocument*> documentWillBeClearedNotifier;
            Notifier<MapDocument*> documentWasClearedNotifier;
            Notifier<MapDocument*> documentWasNewedNotifier;
            Notifier<MapDocument*> documentWasLoadedNotifier;
            Notifier<MapDocument*> documentWasSavedNotifier;
            Notifier<> documentModificationStateDidChangeNotifier;

            Notifier<> editorContextDidChangeNotifier;
            Notifier<> mapViewConfigDidChangeNotifier;
            Notifier<const Model::Layer*> currentLayerDidChangeNotifier;
            Notifier<const std::string&> currentTextureNameDidChangeNotifier;

            Notifier<> selectionWillChangeNotifier;
            Notifier<const Selection&> selectionDidChangeNotifier;

            Notifier<const std::vector<Model::Node*>&> nodesWereAddedNotifier;
            Notifier<const std::vector<Model::Node*>&> nodesWillBeRemovedNotifier;
            Notifier<const std::vector<Model::Node*>&> nodesWereRemovedNotifier;
            Notifier<const std::vector<Model::Node*>&> nodesWillChangeNotifier;
            Notifier<const std::vector<Model::Node*>&> nodesDidChangeNotifier;

            Notifier<const std::vector<Model::Node*>&> nodeVisibilityDidChangeNotifier;
            Notifier<const std::vector<Model::Node*>&> nodeLockingDidChangeNotifier;

            Notifier<Model::Group*> groupWasOpenedNotifier;
            Notifier<Model::Group*> groupWasClosedNotifier;

            Notifier<const std::vector<Model::BrushFace*>&> brushFacesDidChangeNotifier;

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

            std::shared_ptr<Model::Game> game() const override;
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
            template <typename ActionVisitor>
            void visitTagActions(const ActionVisitor& visitor) const {
                visitActions(visitor, m_tagActions);
            }

            template <typename ActionVisitor>
            void visitEntityDefinitionActions(const ActionVisitor& visitor) const {
                visitActions(visitor, m_entityDefinitionActions);
            }
        private: // tag and entity definition actions
            template <typename ActionVisitor>
            void visitActions(const ActionVisitor& visitor, const ActionList& actions) const {
                for (const std::unique_ptr<Action>& action : actions) {
                    visitor(*action);
                }
            }

            void createTagActions();
            void createEntityDefinitionActions();
        public: // new, load, save document
            void newDocument(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game);
            void loadDocument(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game, const IO::Path& path);
            void saveDocument();
            void saveDocumentAs(const IO::Path& path);
            void saveDocumentTo(const IO::Path& path);
            void exportDocumentAs(Model::ExportFormat format, const IO::Path& path);
        private:
            void doSaveDocument(const IO::Path& path);
            void clearDocument();
        public: // copy and paste
            std::string serializeSelectedNodes();
            std::string serializeSelectedBrushFaces();

            PasteType paste(const std::string& str);
        private:
            bool pasteNodes(const std::vector<Model::Node*>& nodes);
            bool pasteBrushFaces(const std::vector<Model::BrushFace*>& faces);
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
            bool hasAnySelectedBrushFaces() const override;

            std::vector<Model::AttributableNode*> allSelectedAttributableNodes() const override;
            const Model::NodeCollection& selectedNodes() const override;
            std::vector<Model::BrushFace*> allSelectedBrushFaces() const override;
            const std::vector<Model::BrushFace*>& selectedBrushFaces() const override;

            const vm::bbox3& referenceBounds() const override;
            const vm::bbox3& lastSelectionBounds() const override;
            const vm::bbox3& selectionBounds() const override;
            const std::string& currentTextureName() const override;
            void setCurrentTextureName(const std::string& currentTextureName);

            void selectAllNodes() override;
            void selectSiblings() override;
            void selectTouching(bool del) override;
            void selectInside(bool del) override;
            void selectNodesWithFilePosition(const std::vector<size_t>& positions) override;
            void select(const std::vector<Model::Node*>& nodes) override;
            void select(Model::Node* node) override;
            void select(const std::vector<Model::BrushFace*>& faces) override;
            void select(Model::BrushFace* face) override;
            void convertToFaceSelection() override;
            void selectFacesWithTexture(const Assets::Texture* texture);

            void deselectAll() override;
            void deselect(Model::Node* node) override;
            void deselect(const std::vector<Model::Node*>& nodes) override;
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

            std::vector<Model::Node*> addNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) override;
            std::vector<Model::Node*> addNodes(const std::vector<Model::Node*>& nodes, Model::Node* parent) override;

            void removeNodes(const std::vector<Model::Node*>& nodes) override;
        private:
            std::map<Model::Node*, std::vector<Model::Node*>> collectRemovableParents(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) const;

            struct CompareByAncestry;
            std::vector<Model::Node*> removeImplicitelyRemovedNodes(std::vector<Model::Node*> nodes) const;

            void closeRemovedGroups(const std::map<Model::Node*, std::vector<Model::Node*>>& toRemove);
        public:
            bool reparentNodes(Model::Node* newParent, const std::vector<Model::Node*>& children) override;
            bool reparentNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) override;
        private:
            bool checkReparenting(const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) const;
        public:
            bool deleteObjects() override;
            bool duplicateObjects() override;
        public: // entity management
            Model::Entity* createPointEntity(const Assets::PointEntityDefinition* definition, const vm::vec3& delta) override;
            Model::Entity* createBrushEntity(const Assets::BrushEntityDefinition* definition) override;
        public: // group management
            Model::Group* groupSelection(const std::string& name);
            void mergeSelectedGroupsWithGroup(Model::Group* group);
        private:
            class MatchGroupableNodes;
            std::vector<Model::Node*> collectGroupableNodes(const std::vector<Model::Node*>& selectedNodes) const;
        public:
            void ungroupSelection();
            void renameGroups(const std::string& name);

            void openGroup(Model::Group* group);
            void closeGroup();
        public: // modifying transient node attributes, declared in MapFacade interface
            void isolate(const std::vector<Model::Node*>& nodes);
            void hide(std::vector<Model::Node*> nodes) override; // Don't take the nodes by reference!
            void hideSelection();
            void show(const std::vector<Model::Node*>& nodes) override;
            void showAll();
            void ensureVisible(const std::vector<Model::Node*>& nodes);
            void resetVisibility(const std::vector<Model::Node*>& nodes) override;

            void lock(const std::vector<Model::Node*>& nodes) override;
            void unlock(const std::vector<Model::Node*>& nodes) override;
            void resetLock(const std::vector<Model::Node*>& nodes) override;
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
            bool setAttribute(const std::string& name, const std::string& value) override;
            bool renameAttribute(const std::string& oldName, const std::string& newName) override;
            bool removeAttribute(const std::string& name) override;

            bool convertEntityColorRange(const std::string& name, Assets::ColorRange::Type range) override;
            bool updateSpawnflag(const std::string& name, const size_t flagIndex, const bool setFlag) override;
        public: // brush resizing, declared in MapFacade interface
            bool resizeBrushes(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) override;
        public: // modifying face attributes, declared in MapFacade interface
            void setTexture(Assets::Texture* texture) override;
        private:
            bool hasTexture(const std::vector<Model::BrushFace*>& faces, Assets::Texture* texture) const;
        public:
            bool setFaceAttributes(const Model::BrushFaceAttributes& attributes) override;
            bool setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) override;
            bool copyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
            bool moveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) override;
            bool rotateTextures(float angle) override;
            bool shearTextures(const vm::vec2f& factors) override;
        public: // modifying vertices, declared in MapFacade interface
            void rebuildBrushGeometry(const std::vector<Model::Brush*>& brushes) override;

            bool snapVertices(FloatType snapTo) override;
            bool findPlanePoints() override;

            MoveVerticesResult moveVertices(const std::map<vm::vec3, std::vector<Model::Brush*>>& vertices, const vm::vec3& delta) override;
            bool moveEdges(const std::map<vm::segment3, std::vector<Model::Brush*>>& edges, const vm::vec3& delta) override;
            bool moveFaces(const std::map<vm::polygon3, std::vector<Model::Brush*>>& faces, const vm::vec3& delta) override;

            bool addVertices(const std::map<vm::vec3, std::vector<Model::Brush*>>& vertices);
            bool removeVertices(const std::map<vm::vec3, std::vector<Model::Brush*>>& vertices);
            bool removeEdges(const std::map<vm::segment3, std::vector<Model::Brush*>>& edges);
            bool removeFaces(const std::map<vm::polygon3, std::vector<Model::Brush*>>& faces);
        private: // subclassing interface for certain operations which are available from this class, but can only be implemented in a subclass
            virtual void performRebuildBrushGeometry(const std::vector<Model::Brush*>& brushes) = 0;
        public: // debug commands
            void printVertices();
            bool throwExceptionDuringCommand();
        public: // command processing
            bool canUndoCommand() const;
            bool canRedoCommand() const;
            const std::string& undoCommandName() const;
            const std::string& redoCommandName() const;
            void undoCommand();
            void redoCommand();
            bool canRepeatCommands() const;
            std::unique_ptr<CommandResult> repeatCommands();
            void clearRepeatableCommands();
        public: // transactions
            void startTransaction(const std::string& name = "");
            void rollbackTransaction();
            void commitTransaction();
            void cancelTransaction();
        private:
            std::unique_ptr<CommandResult> execute(std::unique_ptr<Command>&& command);
            std::unique_ptr<CommandResult> executeAndStore(std::unique_ptr<UndoableCommand>&& command);
        private: // subclassing interface for command processing
            virtual bool doCanUndoCommand() const = 0;
            virtual bool doCanRedoCommand() const = 0;
            virtual const std::string& doGetUndoCommandName() const = 0;
            virtual const std::string& doGetRedoCommandName() const = 0;
            virtual void doUndoCommand() = 0;
            virtual void doRedoCommand() = 0;
            virtual bool doCanRepeatCommands() const = 0;
            virtual std::unique_ptr<CommandResult> doRepeatCommands() = 0;
            virtual void doClearRepeatableCommands() = 0;

            virtual void doStartTransaction(const std::string& name) = 0;
            virtual void doCommitTransaction() = 0;
            virtual void doRollbackTransaction() = 0;

            virtual std::unique_ptr<CommandResult> doExecute(std::unique_ptr<Command>&& command) = 0;
            virtual std::unique_ptr<CommandResult> doExecuteAndStore(std::unique_ptr<UndoableCommand>&& command) = 0;
        public: // asset state management
            void commitPendingAssets();
        public: // picking
            void pick(const vm::ray3& pickRay, Model::PickResult& pickResult) const;
            std::vector<Model::Node*> findNodesContaining(const vm::vec3& point) const;
        private: // world management
            void createWorld(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game);
            void loadWorld(Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game, const IO::Path& path);
            void clearWorld();
        public: // asset management
            Assets::EntityDefinitionFileSpec entityDefinitionFile() const;
            std::vector<Assets::EntityDefinitionFileSpec> allEntityDefinitionFiles() const;
            void setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec);

            // For testing
            void setEntityDefinitions(const std::vector<Assets::EntityDefinition*>& definitions);

            std::vector<IO::Path> enabledTextureCollections() const;
            std::vector<IO::Path> availableTextureCollections() const;
            void setEnabledTextureCollections(const std::vector<IO::Path>& paths);
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
            void setTextures(const std::vector<Model::Node*>& nodes);
            void setTextures(const std::vector<Model::BrushFace*>& faces);
            void unsetTextures();
            void unsetTextures(const std::vector<Model::Node*>& nodes);

            class SetEntityDefinitions;
            class UnsetEntityDefinitions;
            void setEntityDefinitions();
            void setEntityDefinitions(const std::vector<Model::Node*>& nodes);
            void unsetEntityDefinitions();
            void unsetEntityDefinitions(const std::vector<Model::Node*>& nodes);
            void reloadEntityDefinitionsInternal();

            void clearEntityModels();

            class SetEntityModels;
            class UnsetEntityModels;
            void setEntityModels();
            void setEntityModels(const std::vector<Model::Node*>& nodes);
            void unsetEntityModels();
            void unsetEntityModels(const std::vector<Model::Node*>& nodes);
        protected: // search paths and mods
            std::vector<IO::Path> externalSearchPaths() const;
            void updateGameSearchPaths();
        public:
            std::vector<std::string> mods() const override;
            void setMods(const std::vector<std::string>& mods) override;
            std::string defaultMod() const;
        private: // issue management
            void registerIssueGenerators();
        public:
            void setIssueHidden(Model::Issue* issue, bool hidden);
        private:
            virtual void doSetIssueHidden(Model::Issue* issue, bool hidden) = 0;
        public: // tag management
            void registerSmartTags(); // public for testing
            const std::vector<Model::SmartTag>& smartTags() const;
            bool isRegisteredSmartTag(const std::string& name) const;
            const Model::SmartTag& smartTag(const std::string& name) const;
            bool isRegisteredSmartTag(size_t index) const;
            const Model::SmartTag& smartTag(size_t index) const;
        private:
            class InitializeNodeTagsVisitor;
            class ClearNodeTagsVisitor;
            void initializeNodeTags(MapDocument* document);
            void initializeNodeTags(const std::vector<Model::Node*>& nodes);
            void clearNodeTags(const std::vector<Model::Node*>& nodes);
            void updateNodeTags(const std::vector<Model::Node*>& nodes);

            class InitializeFaceTagsVisitor;
            void updateFaceTags(const std::vector<Model::BrushFace*>& faces);
            void updateAllFaceTags();
        public: // document path
            bool persistent() const;
            std::string filename() const;
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
            void commandDone(Command* command);
            void commandUndone(UndoableCommand* command);
        };

        class Transaction {
        private:
            MapDocument* m_document;
            bool m_cancelled;
        public:
            explicit Transaction(std::weak_ptr<MapDocument> document, const std::string& name = "");
            explicit Transaction(std::shared_ptr<MapDocument> document, const std::string& name = "");
            explicit Transaction(MapDocument* document, const std::string& name = "");
            ~Transaction();

            void rollback();
            void cancel();
        private:
            void begin(const std::string& name);
            void commit();
        };
    }
}

#endif /* defined(TrenchBroom_MapDocument) */
