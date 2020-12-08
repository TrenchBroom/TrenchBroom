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

#include "View/MapDocument.h"

#include "Exceptions.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/AssetUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "EL/ELExceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/GameConfigParser.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SystemPaths.h"
#include "Model/AttributeNameWithDoubleQuotationMarksIssueGenerator.h"
#include "Model/AttributeValueWithDoubleQuotationMarksIssueGenerator.h"
#include "Model/Brush.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EditorContext.h"
#include "Model/EmptyAttributeNameIssueGenerator.h"
#include "Model/EmptyAttributeValueIssueGenerator.h"
#include "Model/EmptyBrushEntityIssueGenerator.h"
#include "Model/EmptyGroupIssueGenerator.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "Model/GroupNode.h"
#include "Model/InvalidTextureScaleIssueGenerator.h"
#include "Model/LayerNode.h"
#include "Model/LinkSourceIssueGenerator.h"
#include "Model/LinkTargetIssueGenerator.h"
#include "Model/LockState.h"
#include "Model/LongAttributeNameIssueGenerator.h"
#include "Model/LongAttributeValueIssueGenerator.h"
#include "Model/MissingClassnameIssueGenerator.h"
#include "Model/MissingDefinitionIssueGenerator.h"
#include "Model/MissingModIssueGenerator.h"
#include "Model/MixedBrushContentsIssueGenerator.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "Model/NonIntegerVerticesIssueGenerator.h"
#include "Model/WorldBoundsIssueGenerator.h"
#include "Model/PointEntityWithBrushesIssueGenerator.h"
#include "Model/PointFile.h"
#include "Model/Polyhedron.h"
#include "Model/Polyhedron3.h"
#include "Model/PortalFile.h"
#include "Model/SoftMapBoundsIssueGenerator.h"
#include "Model/TagManager.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"
#include "View/AddBrushVerticesCommand.h"
#include "View/AddRemoveNodesCommand.h"
#include "View/Actions.h"
#include "View/ChangeBrushFaceAttributesCommand.h"
#include "View/ChangeEntityAttributesCommand.h"
#include "View/UpdateEntitySpawnflagCommand.h"
#include "View/ConvertEntityColorCommand.h"
#include "View/CurrentGroupCommand.h"
#include "View/DuplicateNodesCommand.h"
#include "View/EntityDefinitionFileCommand.h"
#include "View/Grid.h"
#include "View/MapTextEncoding.h"
#include "View/MoveBrushEdgesCommand.h"
#include "View/MoveBrushFacesCommand.h"
#include "View/MoveBrushVerticesCommand.h"
#include "View/MoveTexturesCommand.h"
#include "View/PasteType.h"
#include "View/RemoveBrushEdgesCommand.h"
#include "View/RemoveBrushFacesCommand.h"
#include "View/RemoveBrushVerticesCommand.h"
#include "View/ReparentNodesCommand.h"
#include "View/ResizeBrushesCommand.h"
#include "View/CopyTexCoordSystemFromFaceCommand.h"
#include "View/RepeatStack.h"
#include "View/RotateTexturesCommand.h"
#include "View/SelectionCommand.h"
#include "View/SetLockStateCommand.h"
#include "View/SetCurrentLayerCommand.h"
#include "View/SetModsCommand.h"
#include "View/SetTextureCollectionsCommand.h"
#include "View/SetVisibilityCommand.h"
#include "View/ShearTexturesCommand.h"
#include "View/SnapBrushVerticesCommand.h"
#include "View/SwapNodeContentsCommand.h"
#include "View/TransformObjectsCommand.h"
#include "View/ViewEffectsService.h"

#include <kdl/collection_utils.h>
#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <vecmath/polygon.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <algorithm>
#include <cassert>
#include <cstdlib> // for std::abs
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace TrenchBroom {
    namespace View {
        const vm::bbox3 MapDocument::DefaultWorldBounds(-32768.0, 32768.0);
        const std::string MapDocument::DefaultDocumentName("unnamed.map");

        MapDocument::MapDocument() :
        m_worldBounds(DefaultWorldBounds),
        m_world(nullptr),
        m_pointFile(nullptr),
        m_portalFile(nullptr),
        m_entityDefinitionManager(std::make_unique<Assets::EntityDefinitionManager>()),
        m_entityModelManager(std::make_unique<Assets::EntityModelManager>(
            pref(Preferences::TextureMagFilter),
            pref(Preferences::TextureMinFilter),
            logger())),
        m_textureManager(std::make_unique<Assets::TextureManager>(
            pref(Preferences::TextureMagFilter),
            pref(Preferences::TextureMinFilter), logger())),
        m_tagManager(std::make_unique<Model::TagManager>()),
        m_editorContext(std::make_unique<Model::EditorContext>()),
        m_grid(std::make_unique<Grid>(4)),
        m_path(DefaultDocumentName),
        m_lastSaveModificationCount(0),
        m_modificationCount(0),
        m_currentLayer(nullptr),
        m_currentTextureName(Model::BrushFaceAttributes::NoTextureName),
        m_lastSelectionBounds(0.0, 32.0),
        m_selectionBoundsValid(true),
        m_viewEffectsService(nullptr),
        m_repeatStack(std::make_unique<RepeatStack>()) {
            bindObservers();
        }

        MapDocument::~MapDocument() {
            unbindObservers();

            if (isPointFileLoaded()) {
                unloadPointFile();
            }
            if (isPortalFileLoaded()) {
                unloadPortalFile();
            }
            clearWorld();
        }

        Logger& MapDocument::logger() {
            return *this;
        }

        std::shared_ptr<Model::Game> MapDocument::game() const {
            return m_game;
        }

        const vm::bbox3& MapDocument::worldBounds() const {
            return m_worldBounds;
        }

        Model::WorldNode* MapDocument::world() const {
            return m_world.get();
        }

        bool MapDocument::isGamePathPreference(const IO::Path& path) const {
            return m_game.get() != nullptr && m_game->isGamePathPreference(path);
        }

        Model::LayerNode* MapDocument::currentLayer() const {
            ensure(m_currentLayer != nullptr, "currentLayer is null");
            return m_currentLayer;
        }

        /**
         * Sets the current layer immediately, without adding a Command to the undo stack.
         */
        Model::LayerNode* MapDocument::performSetCurrentLayer(Model::LayerNode* currentLayer) {
            ensure(currentLayer != nullptr, "currentLayer is null");
            
            Model::LayerNode* oldCurrentLayer = m_currentLayer;
            m_currentLayer = currentLayer;
            currentLayerDidChangeNotifier(m_currentLayer);

            return oldCurrentLayer;
        }

        void MapDocument::setCurrentLayer(Model::LayerNode* currentLayer) {
            ensure(m_currentLayer != nullptr, "old currentLayer is null");
            ensure(currentLayer != nullptr, "new currentLayer is null");

            Transaction transaction(this, "Set Current Layer");

            while (currentGroup() != nullptr) {
                closeGroup();
            }

            const auto descendants = Model::collectDescendants({m_currentLayer});
            downgradeShownToInherit(descendants);                
            downgradeUnlockedToInherit(descendants);

            executeAndStore(SetCurrentLayerCommand::set(currentLayer));
        }

        bool MapDocument::canSetCurrentLayer(Model::LayerNode* currentLayer) const {
            return m_currentLayer != currentLayer;
        }

        Model::GroupNode* MapDocument::currentGroup() const {
            return m_editorContext->currentGroup();
        }

        Model::Node* MapDocument::currentGroupOrWorld() const {
            Model::Node* result = currentGroup();
            if (result == nullptr) {
                result = m_world.get();
            }
            return result;
        }

        Model::Node* MapDocument::parentForNodes(const std::vector<Model::Node*>& nodes) const {
            if (nodes.empty()) {
                // No reference nodes, so return either the current group (if open) or current layer 
                Model::Node* result = currentGroup();
                if (result == nullptr) {
                    result = currentLayer();
                }
                return result;
            }

            Model::GroupNode* parentGroup = Model::findContainingGroup(nodes.at(0));
            if (parentGroup != nullptr) {
                return parentGroup;
            }

            Model::LayerNode* parentLayer = Model::findContainingLayer(nodes.at(0));
            ensure(parentLayer != nullptr, "no parent layer");
            return parentLayer;
        }

        Model::EditorContext& MapDocument::editorContext() const {
            return *m_editorContext;
        }

        Assets::EntityDefinitionManager& MapDocument::entityDefinitionManager() {
            return *m_entityDefinitionManager;
        }

        Assets::EntityModelManager& MapDocument::entityModelManager() {
            return *m_entityModelManager;
        }

        Assets::TextureManager& MapDocument::textureManager() {
            return *m_textureManager;
        }

        Grid& MapDocument::grid() const {
            return *m_grid;
        }

        Model::PointFile* MapDocument::pointFile() const {
            return m_pointFile.get();
        }

        Model::PortalFile* MapDocument::portalFile() const {
            return m_portalFile.get();
        }

        void MapDocument::setViewEffectsService(ViewEffectsService* viewEffectsService) {
            m_viewEffectsService = viewEffectsService;
        }

        void MapDocument::createTagActions() {
            const auto& actionManager = ActionManager::instance();
            m_tagActions = actionManager.createTagActions(m_tagManager->smartTags());
        }

        void MapDocument::clearTagActions() {
            m_tagActions.clear();
        }

        void MapDocument::createEntityDefinitionActions() {
            const auto& actionManager = ActionManager::instance();
            m_entityDefinitionActions = actionManager.createEntityDefinitionActions(m_entityDefinitionManager->definitions());
        }

        void MapDocument::newDocument(const Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game) {
            info("Creating new document");

            clearRepeatableCommands();
            clearDocument();
            createWorld(mapFormat, worldBounds, game);

            loadAssets();
            registerIssueGenerators();
            registerSmartTags();
            createTagActions();

            clearModificationCount();

            documentWasNewedNotifier(this);
        }

        void MapDocument::loadDocument(const Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game, const IO::Path& path) {
            info("Loading document from " + path.asString());

            clearRepeatableCommands();
            clearDocument();
            loadWorld(mapFormat, worldBounds, game, path);

            loadAssets();
            registerIssueGenerators();
            registerSmartTags();
            createTagActions();

            documentWasLoadedNotifier(this);
        }

        void MapDocument::saveDocument() {
            doSaveDocument(m_path);
        }

        void MapDocument::saveDocumentAs(const IO::Path& path) {
            doSaveDocument(path);
        }

        void MapDocument::saveDocumentTo(const IO::Path& path) {
            ensure(m_game.get() != nullptr, "game is null");
            ensure(m_world != nullptr, "world is null");
            m_game->writeMap(*m_world, path);
        }

        void MapDocument::exportDocumentAs(const Model::ExportFormat format, const IO::Path& path) {
            m_game->exportMap(*m_world, format, path);
        }

        void MapDocument::doSaveDocument(const IO::Path& path) {
            saveDocumentTo(path);
            setLastSaveModificationCount();
            setPath(path);
            documentWasSavedNotifier(this);
        }

        void MapDocument::clearDocument() {
            if (m_world != nullptr) {
                documentWillBeClearedNotifier(this);

                m_editorContext->reset();
                clearSelection();
                unloadAssets();
                clearTagActions();
                clearWorld();
                clearModificationCount();

                documentWasClearedNotifier(this);
            }
        }

        MapTextEncoding MapDocument::encoding() const {
            return MapTextEncoding::Quake;
        }

        std::string MapDocument::serializeSelectedNodes() {
            std::stringstream stream;
            m_game->writeNodesToStream(*m_world, m_selectedNodes.nodes(), stream);
            return stream.str();
        }

        std::string MapDocument::serializeSelectedBrushFaces() {
            std::stringstream stream;
            const auto faces = kdl::vec_transform(m_selectedBrushFaces, [](const auto& h) { return h.face(); });
            m_game->writeBrushFacesToStream(*m_world, faces, stream);
            return stream.str();
        }

        PasteType MapDocument::paste(const std::string& str) {
            // Try parsing as entities, then as brushes, in all compatible formats
            const std::vector<Model::Node*> nodes = m_game->parseNodes(str, *m_world, m_worldBounds, logger());
            if (!nodes.empty()) {
                if (pasteNodes(nodes)) {
                    return PasteType::Node;
                }
                return PasteType::Failed;
            }

            // Try parsing as brush faces
            try {
                const std::vector<Model::BrushFace> faces = m_game->parseBrushFaces(str, *m_world, m_worldBounds, logger());
                if (!faces.empty() && pasteBrushFaces(faces)) {
                    return PasteType::BrushFace;
                }
            } catch (const ParserException& e) {
                error() << "Could not parse clipboard contents: " << e.what();
            }
            return PasteType::Failed;
        }

        bool MapDocument::pasteNodes(const std::vector<Model::Node*>& nodes) {
            auto nodesToDetach = std::vector<Model::Node*>{};
            auto nodesToDelete = std::vector<Model::Node*>{};
            auto nodesToAdd = std::map<Model::Node*, std::vector<Model::Node*>>{};

            auto* parent = parentForNodes();
            for (auto* node : nodes) {
                node->accept(kdl::overload(
                    [&](auto&& thisLambda, Model::WorldNode* world) {
                        world->visitChildren(thisLambda);
                        nodesToDelete.push_back(world);
                    },
                    [&](auto&& thisLambda, Model::LayerNode* layer) {
                        layer->visitChildren(thisLambda);
                        nodesToDetach.push_back(layer);
                        nodesToDelete.push_back(layer);
                    },
                    [&](Model::GroupNode* group) {
                        nodesToDetach.push_back(group);
                        nodesToAdd[parent].push_back(group);
                    },
                    [&](auto&& thisLambda, Model::EntityNode* entityNode) {
                        if (Model::isWorldspawn(entityNode->entity().classname(), entityNode->entity().attributes())) {
                            entityNode->visitChildren(thisLambda);
                            nodesToDetach.push_back(entityNode);
                            nodesToDelete.push_back(entityNode);
                        } else {
                            nodesToDetach.push_back(entityNode);
                            nodesToAdd[parent].push_back(entityNode);
                        }
                    },
                    [&](Model::BrushNode* brush) {
                        nodesToDetach.push_back(brush);
                        nodesToAdd[parent].push_back(brush);
                    }
                ));
            }

            for (auto* node : nodesToDetach) {
                auto* nodeParent = node->parent();
                if (nodeParent != nullptr) {
                    nodeParent->removeChild(node);
                }
            }
            kdl::vec_clear_and_delete(nodesToDelete);

            const std::vector<Model::Node*> addedNodes = addNodes(nodesToAdd);
            if (addedNodes.empty())
                return false;

            deselectAll();

            const auto nodesToSelect = Model::collectSelectableNodes(addedNodes, editorContext());
            select(nodesToSelect);

            return true;
        }

        bool MapDocument::pasteBrushFaces(const std::vector<Model::BrushFace>& faces) {
            assert(!faces.empty());
            return setFaceAttributesExceptContentFlags(faces.back().attributes());
        }

        void MapDocument::loadPointFile(const IO::Path path) {
            static_assert(!std::is_reference<decltype(path)>::value,
                          "path must be passed by value because reloadPointFile() passes m_pointFilePath");

            if (!Model::PointFile::canLoad(path)) {
                return;
            }

            if (isPointFileLoaded()) {
                unloadPointFile();
            }

            m_pointFilePath = path;
            m_pointFile = std::make_unique<Model::PointFile>(m_pointFilePath);

            info("Loaded point file " + m_pointFilePath.asString());
            pointFileWasLoadedNotifier();
        }

        bool MapDocument::isPointFileLoaded() const {
            return m_pointFile != nullptr;
        }

        bool MapDocument::canReloadPointFile() const {
            return m_pointFile != nullptr && Model::PointFile::canLoad(m_pointFilePath);
        }

        void MapDocument::reloadPointFile() {
            assert(isPointFileLoaded());
            loadPointFile(m_pointFilePath);
        }

        void MapDocument::unloadPointFile() {
            assert(isPointFileLoaded());
            m_pointFile = nullptr;
            m_pointFilePath = IO::Path();

            info("Unloaded point file");
            pointFileWasUnloadedNotifier();
        }

        void MapDocument::loadPortalFile(const IO::Path path) {
            static_assert(!std::is_reference<decltype(path)>::value,
                          "path must be passed by value because reloadPortalFile() passes m_portalFilePath");

            if (!Model::PortalFile::canLoad(path)) {
                return;
            }

            if (isPortalFileLoaded()) {
                unloadPortalFile();
            }

            try {
                m_portalFilePath = path;
                m_portalFile = std::make_unique<Model::PortalFile>(path);
            } catch (const std::exception &exception) {
                info("Couldn't load portal file " + m_portalFilePath.asString() + ": " + exception.what());
            }

            if (isPortalFileLoaded()) {
                info("Loaded portal file " + m_portalFilePath.asString());
                portalFileWasLoadedNotifier();
            }
        }

        bool MapDocument::isPortalFileLoaded() const {
            return m_portalFile != nullptr;
        }

        bool MapDocument::canReloadPortalFile() const {
            return m_portalFile != nullptr && Model::PortalFile::canLoad(m_portalFilePath);
        }

        void MapDocument::reloadPortalFile() {
            assert(isPortalFileLoaded());
            loadPortalFile(m_portalFilePath);
        }

        void MapDocument::unloadPortalFile() {
            assert(isPortalFileLoaded());
            m_portalFile = nullptr;
            m_portalFilePath = IO::Path();

            info("Unloaded portal file");
            portalFileWasUnloadedNotifier();
        }

        bool MapDocument::hasSelection() const {
            return hasSelectedNodes() || hasSelectedBrushFaces();
        }

        bool MapDocument::hasSelectedNodes() const {
            return !m_selectedNodes.empty();
        }

        bool MapDocument::hasSelectedBrushFaces() const {
            return !m_selectedBrushFaces.empty();
        }

        bool MapDocument::hasAnySelectedBrushFaces() const {
            return hasSelectedBrushFaces() || selectedNodes().hasBrushes();
        }

        std::vector<Model::AttributableNode*> MapDocument::allSelectedAttributableNodes() const {
            if (!hasSelection()) {
                return std::vector<Model::AttributableNode*>({ m_world.get() });
            }

            std::vector<Model::AttributableNode*> nodes;
            for (auto* node : m_selectedNodes) {
                node->accept(kdl::overload(
                    [&](auto&& thisLambda, Model::WorldNode* world) { nodes.push_back(world); world->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
                    [&](Model::EntityNode* entity)                  { nodes.push_back(entity); },
                    [&](Model::BrushNode* brush) {
                        auto* entity = brush->entity();
                        ensure(entity != nullptr, "entity is null");
                        nodes.push_back(entity);
                    }
                ));
            }

            return kdl::vec_sort_and_remove_duplicates(std::move(nodes));
        }

        const Model::NodeCollection& MapDocument::selectedNodes() const {
            return m_selectedNodes;
        }

        std::vector<Model::BrushFaceHandle> MapDocument::allSelectedBrushFaces() const {
            if (hasSelectedBrushFaces())
                return selectedBrushFaces();
            return Model::collectBrushFaces(m_selectedNodes.nodes());
        }

        std::vector<Model::BrushFaceHandle> MapDocument::selectedBrushFaces() const {
            return m_selectedBrushFaces;
        }

        const vm::bbox3& MapDocument::referenceBounds() const {
            if (hasSelectedNodes())
                return selectionBounds();
            return lastSelectionBounds();
        }

        const vm::bbox3& MapDocument::lastSelectionBounds() const {
            return m_lastSelectionBounds;
        }

        const vm::bbox3& MapDocument::selectionBounds() const {
            if (!m_selectionBoundsValid)
                validateSelectionBounds();
            return m_selectionBounds;
        }

        const std::string& MapDocument::currentTextureName() const {
            return m_currentTextureName;
        }

        void MapDocument::setCurrentTextureName(const std::string& currentTextureName) {
            if (m_currentTextureName == currentTextureName)
                return;
            m_currentTextureName = currentTextureName;
            currentTextureNameDidChangeNotifier(m_currentTextureName);
        }

        void MapDocument::selectAllNodes() {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::selectAllNodes());
        }

        void MapDocument::selectSiblings() {
            const std::vector<Model::Node*>& nodes = selectedNodes().nodes();
            if (nodes.empty()) {
                return;
            }

            auto visited = std::unordered_set<Model::Node*>{};
            auto nodesToSelect = std::vector<Model::Node*>{};

            for (auto* node : nodes) {
                auto* parent = node->parent();
                if (!visited.insert(parent).second) {
                    nodesToSelect = kdl::vec_concat(std::move(nodesToSelect), Model::collectSelectableNodes(parent->children(), editorContext()));
                }
            }

            Transaction transaction(this, "Select Siblings");
            deselectAll();
            select(nodesToSelect);
        }

        void MapDocument::selectTouching(const bool del) {
            const auto nodes = kdl::vec_filter(
                Model::collectTouchingNodes(std::vector<Model::Node*>{m_world.get()}, m_selectedNodes.brushes()),
                [&](Model::Node* node) { return m_editorContext->selectable(node); });

            Transaction transaction(this, "Select Touching");
            if (del)
                deleteObjects();
            else
                deselectAll();
            select(nodes);
        }

        void MapDocument::selectInside(const bool del) {
            const auto nodes = kdl::vec_filter(
                Model::collectContainedNodes(std::vector<Model::Node*>{m_world.get()}, m_selectedNodes.brushes()),
                [&](Model::Node* node) { return m_editorContext->selectable(node); });

            Transaction transaction(this, "Select Inside");
            if (del)
                deleteObjects();
            else
                deselectAll();
            select(nodes);
        }

        void MapDocument::selectInverse() {
            // This only selects nodes that have no selected children (or parents).
            // This is because if a brush entity only 1 selected child and 1 unselected,
            // we treat it as partially selected and don't want to try to select the entity if the
            // selection is inverted, which would reselect both children.

            auto nodesToSelect = std::vector<Model::Node*>{};
            const auto collectNode = [&](auto* node) {
                if (!node->transitivelySelected() && !node->descendantSelected() && m_editorContext->selectable(node)) {
                    nodesToSelect.push_back(node);
                }
            };

            currentGroupOrWorld()->accept(kdl::overload(
                [] (auto&& thisLambda, Model::WorldNode* world)   { world->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::LayerNode* layer)   { layer->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::GroupNode* group)   { collectNode(group); group->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::EntityNode* entity) { collectNode(entity); entity->visitChildren(thisLambda); },
                [&](Model::BrushNode* brush)                      { collectNode(brush); }
            ));

            Transaction transaction(this, "Select Inverse");
            deselectAll();
            select(nodesToSelect);
        }

        void MapDocument::selectNodesWithFilePosition(const std::vector<size_t>& positions) {
            const auto nodes = kdl::vec_filter(
                Model::collectSelectableNodes(std::vector<Model::Node*>{m_world.get()}, *m_editorContext),
                [&](const auto* node) {
                    for (const size_t position : positions) {
                        if (node->containsLine(position)) {
                            return true;
                        }
                    }
                    return false;
                });

            Transaction transaction(this, "Select by Line Number");
            deselectAll();
            select(nodes);
        }

        void MapDocument::select(const std::vector<Model::Node*>& nodes) {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::select(nodes));
        }

        void MapDocument::select(Model::Node* node) {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::select(std::vector<Model::Node*>(1, node)));
        }

        void MapDocument::select(const std::vector<Model::BrushFaceHandle>& handles) {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::select(handles));
        }

        void MapDocument::select(const Model::BrushFaceHandle& handle) {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::select({ handle }));
            setCurrentTextureName(handle.face().attributes().textureName());
        }

        void MapDocument::convertToFaceSelection() {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::convertToFaces());
        }

        void MapDocument::selectFacesWithTexture(const Assets::Texture* texture) {
            const auto faces = kdl::vec_filter(
                Model::collectSelectableBrushFaces(std::vector<Model::Node*>{m_world.get()}, *m_editorContext), 
                [&](const auto& faceHandle) { return faceHandle.face().texture() == texture; });

            Transaction transaction(this, "Select Faces with Texture");
            deselectAll();
            select(faces);
        }

        void MapDocument::selectTall(const vm::axis::type cameraAxis) {
            const vm::vec3 cameraAbsDirection = vm::vec3::axis(cameraAxis);
            const vm::bbox3 tallBounds = worldBounds().expand(-1.0); // we can't make a brush that is exactly as large as worldBounds

            const FloatType min = vm::dot(tallBounds.min, cameraAbsDirection);
            const FloatType max = vm::dot(tallBounds.max, cameraAbsDirection);

            const vm::plane3 minPlane(min, cameraAbsDirection);
            const vm::plane3 maxPlane(max, cameraAbsDirection);

            const std::vector<Model::BrushNode*>& selectionBrushNodes = selectedNodes().brushes();
            assert(!selectionBrushNodes.empty());

            const Model::BrushBuilder brushBuilder(world(), worldBounds());
            std::vector<Model::BrushNode*> tallBrushes;
            tallBrushes.reserve(selectionBrushNodes.size());

            for (const Model::BrushNode* selectionBrushNode : selectionBrushNodes) {
                const Model::Brush& selectionBrush = selectionBrushNode->brush();
                
                std::vector<vm::vec3> tallVertices;
                tallVertices.reserve(2 * selectionBrush.vertexCount());

                for (const Model::BrushVertex* vertex : selectionBrush.vertices()) {
                    tallVertices.push_back(minPlane.project_point(vertex->position()));
                    tallVertices.push_back(maxPlane.project_point(vertex->position()));
                }

                brushBuilder.createBrush(tallVertices, Model::BrushFaceAttributes::NoTextureName)
                    .visit(kdl::overload(
                        [&](Model::Brush&& b) {
                            tallBrushes.push_back(world()->createBrush(std::move(b)));
                        },
                        [&](const Model::BrushError e) {
                            logger().error() << "Could not create selection brush: " << e;
                        }
                    ));
            }

            // delete the original selection brushes before searching for the objects to select
            Transaction transaction(this, "Select Tall");
            deleteObjects();

            const auto nodesToSelect = kdl::vec_filter(
                Model::collectContainedNodes(std::vector<Model::Node*>{world()}, tallBrushes), 
                [&](const auto* node) { return editorContext().selectable(node); });
            kdl::vec_clear_and_delete(tallBrushes);

            select(nodesToSelect);
        }

        void MapDocument::deselectAll() {
            if (hasSelection()) {
                m_repeatStack->clearOnNextPush();
                executeAndStore(SelectionCommand::deselectAll());
            }
        }

        void MapDocument::deselect(Model::Node* node) {
            deselect(std::vector<Model::Node*>(1, node));
        }

        void MapDocument::deselect(const std::vector<Model::Node*>& nodes) {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::deselect(nodes));
        }

        void MapDocument::deselect(const Model::BrushFaceHandle& handle) {
            m_repeatStack->clearOnNextPush();
            executeAndStore(SelectionCommand::deselect({ handle }));
        }

        void MapDocument::updateLastSelectionBounds() {
            const auto currentSelectionBounds = selectionBounds();
            if (currentSelectionBounds.is_valid() && !currentSelectionBounds.is_empty()) {
                m_lastSelectionBounds = selectionBounds();
            }
        }

        void MapDocument::invalidateSelectionBounds() {
            m_selectionBoundsValid = false;
        }

        void MapDocument::validateSelectionBounds() const {
            m_selectionBounds = computeLogicalBounds(m_selectedNodes.nodes());
            m_selectionBoundsValid = true;
        }

        void MapDocument::clearSelection() {
            m_selectedNodes.clear();
            m_selectedBrushFaces.clear();
        }

        void MapDocument::addNode(Model::Node* node, Model::Node* parent) {
            ensure(node != nullptr, "node is null");
            assert(node->parent() == nullptr);
            ensure(parent != nullptr, "parent is null");
            assert(parent != node);

            std::map<Model::Node*, std::vector<Model::Node*>> map;
            map[parent].push_back(node);
            addNodes(map);
        }

        void MapDocument::removeNode(Model::Node* node) {
            removeNodes(std::vector<Model::Node*>(1, node));
        }

        std::vector<Model::Node*> MapDocument::addNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) {
            Transaction transaction(this, "Add Objects");
            const auto result = executeAndStore(AddRemoveNodesCommand::add(nodes));
            if (!result->success()) {
                return {};
            }

            const std::vector<Model::Node*> addedNodes = collectChildren(nodes);
            ensureVisible(addedNodes);
            ensureUnlocked(addedNodes);
            return addedNodes;
        }

        std::vector<Model::Node*> MapDocument::addNodes(const std::vector<Model::Node*>& nodes, Model::Node* parent) {
            const auto result = executeAndStore(AddRemoveNodesCommand::add(parent, nodes));
            if (!result->success()) {
                return {};
            }

            ensureVisible(nodes);
            return nodes;
        }

        void MapDocument::removeNodes(const std::vector<Model::Node*>& nodes) {
            std::map<Model::Node*, std::vector<Model::Node*>> removableNodes = parentChildrenMap(removeImplicitelyRemovedNodes(nodes));

            Transaction transaction(this);
            while (!removableNodes.empty()) {
                closeRemovedGroups(removableNodes);
                executeAndStore(AddRemoveNodesCommand::remove(removableNodes));

                removableNodes = collectRemovableParents(removableNodes);
            }
        }

        std::map<Model::Node*, std::vector<Model::Node*>> MapDocument::collectRemovableParents(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) const {
            std::map<Model::Node*, std::vector<Model::Node*>> result;
            for (const auto& entry : nodes) {
                Model::Node* node = entry.first;
                if (node->removeIfEmpty() && !node->hasChildren()) {
                    Model::Node* parent = node->parent();
                    ensure(parent != nullptr, "parent is null");
                    result[parent].push_back(node);
                }
            }
            return result;
        }

        struct MapDocument::CompareByAncestry {
            bool operator()(const Model::Node* lhs, const Model::Node* rhs) const {
                return lhs->isAncestorOf(rhs);
            }
        };

        std::vector<Model::Node*> MapDocument::removeImplicitelyRemovedNodes(std::vector<Model::Node*> nodes) const {
            if (nodes.empty())
                return nodes;

            nodes = kdl::vec_sort(std::move(nodes), CompareByAncestry());

            std::vector<Model::Node*> result;
            result.reserve(nodes.size());
            result.push_back(nodes.front());

            for (size_t i = 1; i < nodes.size(); ++i) {
                Model::Node* node = nodes[i];
                if (!node->isDescendantOf(result))
                    result.push_back(node);
            }

            return result;
        }

        void MapDocument::closeRemovedGroups(const std::map<Model::Node*, std::vector<Model::Node*>>& toRemove) {
            for (const auto& entry : toRemove) {
                const std::vector<Model::Node*>& nodes = entry.second;
                for (const Model::Node* node : nodes) {
                    if (node == currentGroup()) {
                        closeGroup();
                        closeRemovedGroups(toRemove);
                        return;
                    }
                }
            }
        }

        bool MapDocument::reparentNodes(Model::Node* newParent, const std::vector<Model::Node*>& children) {
            std::map<Model::Node*, std::vector<Model::Node*>> nodes;
            nodes.insert(std::make_pair(newParent, children));
            return reparentNodes(nodes);
        }

        bool MapDocument::reparentNodes(const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) {
            if (!checkReparenting(nodesToAdd))
                return false;

            std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove;
            for (const auto& entry : nodesToAdd) {
                const std::vector<Model::Node*>& children = entry.second;
                nodesToRemove = kdl::map_merge(nodesToRemove, Model::parentChildrenMap(children));
            }

            Transaction transaction(this, "Reparent Objects");

            // This handles two main cases:
            // - creating brushes in a hidden layer, and then grouping / ungrouping them keeps them visible
            // - creating brushes in a hidden layer, then moving them to a hidden layer, should downgrade them
            //   to inherited and hide them
            for (auto& [newParent, nodes] : nodesToAdd) {
                Model::LayerNode* newParentLayer = Model::findContainingLayer(newParent);

                const auto nodesToDowngrade = kdl::vec_filter(
                    Model::collectNodes(nodes), 
                    [&](auto* node) { return Model::findContainingLayer(node) != newParentLayer; });

                downgradeUnlockedToInherit(nodesToDowngrade);
                downgradeShownToInherit(nodesToDowngrade);
            }

            executeAndStore(ReparentNodesCommand::reparent(nodesToAdd, nodesToRemove));

            std::map<Model::Node*, std::vector<Model::Node*>> removableNodes = collectRemovableParents(nodesToRemove);
            while (!removableNodes.empty()) {
                closeRemovedGroups(removableNodes);
                executeAndStore(AddRemoveNodesCommand::remove(removableNodes));

                removableNodes = collectRemovableParents(removableNodes);
            }

            return true;
        }

        bool MapDocument::checkReparenting(const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) const {
            for (const auto& entry : nodesToAdd) {
                const Model::Node* newParent = entry.first;
                const std::vector<Model::Node*>& children = entry.second;
                if (!newParent->canAddChildren(std::begin(children), std::end(children)))
                    return false;
            }
            return true;
        }

        bool MapDocument::deleteObjects() {
            Transaction transaction(this, "Delete Objects");
            const std::vector<Model::Node*> nodes = m_selectedNodes.nodes();
            deselectAll();
            removeNodes(nodes);
            return true;
        }

        bool MapDocument::duplicateObjects() {
            const auto result = executeAndStore(DuplicateNodesCommand::duplicate());
            if (result->success()) {
                if (m_viewEffectsService) {
                    m_viewEffectsService->flashSelection();
                }
                m_repeatStack->push([=]() { this->duplicateObjects(); });
                return true;
            }
            return false;
        }

        Model::EntityNode* MapDocument::createPointEntity(const Assets::PointEntityDefinition* definition, const vm::vec3& delta) {
            ensure(definition != nullptr, "definition is null");

            auto* entity = m_world->createEntity(Model::Entity({
                {Model::AttributeNames::Classname, definition->name()}
            }));

            std::stringstream name;
            name << "Create " << definition->name();

            const Transaction transaction(this, name.str());
            deselectAll();
            addNode(entity, parentForNodes());
            select(entity);
            translateObjects(delta);

            return entity;
        }

        Model::EntityNode* MapDocument::createBrushEntity(const Assets::BrushEntityDefinition* definition) {
            ensure(definition != nullptr, "definition is null");

            const auto brushes = selectedNodes().brushes();
            assert(!brushes.empty());

            auto entity = Model::Entity();

            // if all brushes belong to the same entity, and that entity is not worldspawn, copy its properties
            auto* entityTemplate = brushes.front()->entity();
            if (entityTemplate != m_world.get()) {
                for (auto* brush : brushes) {
                    if (brush->entity() != entityTemplate) {
                        entityTemplate = nullptr;
                        break;
                    }
                }

                if (entityTemplate != nullptr) {
                    entity = entityTemplate->entity();
                }
            }

            entity.addOrUpdateAttribute(Model::AttributeNames::Classname, definition->name());
            auto* entityNode = m_world->createEntity(std::move(entity));

            std::stringstream name;
            name << "Create " << definition->name();

            const std::vector<Model::Node*> nodes(std::begin(brushes), std::end(brushes));

            const Transaction transaction(this, name.str());
            deselectAll();
            addNode(entityNode, parentForNodes(nodes));
            reparentNodes(entityNode, nodes);
            select(nodes);

            return entityNode;
        }

        Model::GroupNode* MapDocument::groupSelection(const std::string& name) {
            if (!hasSelectedNodes())
                return nullptr;

            const std::vector<Model::Node*> nodes = collectGroupableNodes(selectedNodes().nodes());
            if (nodes.empty())
                return nullptr;

            Model::GroupNode* group = new Model::GroupNode(name);

            const Transaction transaction(this, "Group Selected Objects");
            deselectAll();
            addNode(group, parentForNodes(nodes));
            reparentNodes(group, nodes);
            select(group);

            return group;
        }

        void MapDocument::mergeSelectedGroupsWithGroup(Model::GroupNode* group) {
            if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
                return;

            const Transaction transaction(this, "Merge Groups");
            const std::vector<Model::GroupNode*> groupsToMerge = m_selectedNodes.groups();

            deselectAll();
            for (auto groupToMerge : groupsToMerge) {
                if (groupToMerge == group)
                    continue;

                const std::vector<Model::Node*> children = groupToMerge->children();
                reparentNodes(group, children);
            }
            select(group);
        }

        std::vector<Model::Node*> MapDocument::collectGroupableNodes(const std::vector<Model::Node*>& selectedNodes) const {
            std::vector<Model::Node*> result;
            Model::Node::visitAll(selectedNodes, kdl::overload(
                [] (Model::WorldNode*)         {},
                [] (Model::LayerNode*)         {},
                [&](Model::GroupNode* group)   { result.push_back(group); },
                [&](Model::EntityNode* entity) { result.push_back(entity); },
                [&](auto&& thisLambda, Model::BrushNode* brush) {
                    if (brush->entity() == world()) {
                        result.push_back(brush);
                    } else {
                        brush->visitParent(thisLambda);
                    }
                }
            ));
            return kdl::vec_sort_and_remove_duplicates(std::move(result));
        }

        void MapDocument::ungroupSelection() {
            if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
                return;

            const std::vector<Model::Node*> groups = m_selectedNodes.nodes();
            std::vector<Model::Node*> allChildren;

            const Transaction transaction(this, "Ungroup");
            deselectAll();

            for (Model::Node* group : groups) {
                Model::Node* parent = group->parent();
                const std::vector<Model::Node*> children = group->children();
                reparentNodes(parent, children);
                allChildren = kdl::vec_concat(std::move(allChildren), children);
            }

            select(allChildren);
        }

        void MapDocument::renameGroups(const std::string& name) {
            if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
                return;
            
            const std::vector<Model::AttributableNode*> groups = kdl::vec_element_cast<Model::AttributableNode*>(m_selectedNodes.groups());

            const Transaction transaction(this, "Rename Groups");
            executeAndStore(ChangeEntityAttributesCommand::setForNodes(groups, Model::AttributeNames::GroupName, name));
        }

        void MapDocument::openGroup(Model::GroupNode* group) {
            const Transaction transaction(this, "Open Group");

            deselectAll();
            Model::GroupNode* previousGroup = m_editorContext->currentGroup();
            if (previousGroup == nullptr)
                lock(std::vector<Model::Node*>(1, m_world.get()));
            else
                resetLock(std::vector<Model::Node*>(1, previousGroup));
            unlock(std::vector<Model::Node*>(1, group));
            executeAndStore(CurrentGroupCommand::push(group));
        }

        void MapDocument::closeGroup() {
            const Transaction transaction(this, "Close Group");

            deselectAll();
            Model::GroupNode* previousGroup = m_editorContext->currentGroup();
            resetLock(std::vector<Model::Node*>(1, previousGroup));
            executeAndStore(CurrentGroupCommand::pop());

            Model::GroupNode* currentGroup = m_editorContext->currentGroup();
            if (currentGroup != nullptr) {
                unlock(std::vector<Model::Node*>(1, currentGroup));
            } else {
                unlock(std::vector<Model::Node*>(1, m_world.get()));
            }
        }

        void MapDocument::renameLayer(Model::LayerNode* layer, const std::string& name) {
            const Transaction transaction(this, "Rename Layer");

            const auto result = executeAndStore(ChangeEntityAttributesCommand::setForNodes({ layer }, Model::AttributeNames::LayerName, name));
            unused(result);
        }

        bool MapDocument::moveLayerByOne(Model::LayerNode* layer, MoveDirection direction) {
            const std::vector<Model::LayerNode*> sorted = m_world->customLayersUserSorted();

            const auto maybeIndex = kdl::vec_index_of(sorted, layer);
            if (!maybeIndex.has_value()) {
                return false;
            }

            const int newIndex = static_cast<int>(*maybeIndex) + (direction == MoveDirection::Down ? 1 : -1);
            if (newIndex < 0 || newIndex >= static_cast<int>(sorted.size())) {
                return false;
            }
            
            Model::LayerNode* neighbour = sorted.at(static_cast<size_t>(newIndex));           
            const int ourSortIndex = layer->sortIndex();
            const int neighbourSortIndex = neighbour->sortIndex();

            // Swap the sort indices of `layer` and `neighbour`
            executeAndStore(ChangeEntityAttributesCommand::setForNodes({ layer },     Model::AttributeNames::LayerSortIndex, std::to_string(neighbourSortIndex)));
            executeAndStore(ChangeEntityAttributesCommand::setForNodes({ neighbour }, Model::AttributeNames::LayerSortIndex, std::to_string(ourSortIndex)));
            return true;
        }

        void MapDocument::moveLayer(Model::LayerNode* layer, const int offset) {
            ensure(layer != m_world->defaultLayer(), "attempted to move default layer");

            const Transaction transaction(this, "Move Layer");
            
            const MoveDirection direction = (offset > 0) ? MoveDirection::Down : MoveDirection::Up;
            for (int i = 0; i < std::abs(offset); ++i) {
                if (!moveLayerByOne(layer, direction)) {
                    break;
                }
            }
        }
        
        bool MapDocument::canMoveLayer(Model::LayerNode* layer, const int offset) const {
            ensure(layer != nullptr, "null layer");

            Model::WorldNode* world = this->world();
            if (layer == world->defaultLayer()) {
                return false;
            }

            const std::vector<Model::LayerNode*> sorted = world->customLayersUserSorted();            
            const auto maybeIndex = kdl::vec_index_of(sorted, layer);
            if (!maybeIndex.has_value()) {
                return false;
            }

            const int newIndex = static_cast<int>(*maybeIndex) + offset;
            return (newIndex >= 0 && newIndex < static_cast<int>(sorted.size()));
        }

        void MapDocument::moveSelectionToLayer(Model::LayerNode* layer) {
            Transaction transaction(this, "Move Nodes to " + layer->name());

            const auto& selectedNodes = this->selectedNodes().nodes();
            
            auto nodesToMove = std::vector<Model::Node*>{};
            auto nodesToSelect = std::vector<Model::Node*>{};

            for (auto* node : selectedNodes) {
                node->accept(kdl::overload(
                    [] (Model::WorldNode*) {},
                    [] (Model::LayerNode*) {},
                    [&](Model::GroupNode* group) {
                        assert(group->selected());

                        if (!group->grouped()) {
                            nodesToMove.push_back(group);
                            nodesToSelect.push_back(group);
                        }
                    },
                    [&](Model::EntityNode* entity) {
                        assert(entity->selected());
                        if (!entity->grouped()) {
                            nodesToMove.push_back(entity);
                            nodesToSelect.push_back(entity);
                        }
                    },
                    [&](Model::BrushNode* brush) {
                        assert(brush->selected());

                        if (!brush->grouped()) {
                            auto* entity = brush->entity();
                            if (entity == m_world.get()) {
                                nodesToMove.push_back(brush);
                                nodesToSelect.push_back(brush);
                            } else {
                                if (!kdl::vec_contains(nodesToMove, entity)) {
                                    nodesToMove.push_back(entity);
                                    nodesToSelect = kdl::vec_concat(std::move(nodesToSelect), entity->children());
                                }
                            }
                        }
                    }
                ));
            }

            if (!nodesToMove.empty()) {
                deselectAll();
                reparentNodes(layer, nodesToMove);
                if (!layer->hidden() && !layer->locked()) {
                    select(nodesToSelect);
                }
            }
        }

        bool MapDocument::canMoveSelectionToLayer(Model::LayerNode* layer) const {
            ensure(layer != nullptr, "null layer");
            const auto& nodes = selectedNodes().nodes();

            const bool isAnyNodeInGroup = std::any_of(std::begin(nodes), std::end(nodes), [&](auto* node) { return Model::findContainingGroup(node) != nullptr; });
            const bool isAnyNodeInOtherLayer = std::any_of(std::begin(nodes), std::end(nodes), [&](auto* node) { return Model::findContainingLayer(node) != layer; });

            return !nodes.empty() && !isAnyNodeInGroup && isAnyNodeInOtherLayer;
        }

        void MapDocument::hideLayers(const std::vector<Model::LayerNode*>& layers) {
            Transaction transaction(this, "Hide Layers");
            hide(std::vector<Model::Node*>(std::begin(layers), std::end(layers)));
        }

        bool MapDocument::canHideLayers(const std::vector<Model::LayerNode*>& layers) const {
            for (auto* layer : layers) {
                if (layer->visible()) {
                    return true;
                }
            }
            return false;
        }

        void MapDocument::isolateLayers(const std::vector<Model::LayerNode*>& layers) {
            const auto allLayers = world()->allLayers();

            Transaction transaction(this, "Isolate Layers");
            hide(std::vector<Model::Node*>(std::begin(allLayers), std::end(allLayers)));
            show(std::vector<Model::Node*>(std::begin(layers), std::end(layers)));
        }

        bool MapDocument::canIsolateLayers(const std::vector<Model::LayerNode*>& layers) const {
            if (layers.empty()) {
                return false;
            }
            for (auto* layer : m_world->allLayers()) {
                const bool shouldShowLayer = kdl::vec_contains(layers, layer);

                if (shouldShowLayer != layer->visible()) {
                    return true;
                }
            }
            // The layers are already isolated
            return false;
        }

        void MapDocument::isolate() {
            auto selectedNodes = std::vector<Model::Node*>{};
            auto unselectedNodes = std::vector<Model::Node*>{};

            const auto collectNode = [&](auto* node) {
                if (node->transitivelySelected() || node->descendantSelected()) {
                    selectedNodes.push_back(node);
                } else {
                    unselectedNodes.push_back(node);
                }
            };

            m_world->accept(kdl::overload(
                [] (auto&& thisLambda, Model::WorldNode* world)   { world->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::LayerNode* layer)   { layer->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::GroupNode* group)   { collectNode(group); group->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::EntityNode* entity) { collectNode(entity); entity->visitChildren(thisLambda); },
                [&](Model::BrushNode* brush) { collectNode(brush); }
            ));

            Transaction transaction(this, "Isolate Objects");
            executeAndStore(SetVisibilityCommand::hide(unselectedNodes));
            executeAndStore(SetVisibilityCommand::show(selectedNodes));
        }

        void MapDocument::setOmitLayerFromExport(Model::LayerNode* layer, const bool omitFromExport) {
            if (omitFromExport) {
                Transaction transaction(this, "Omit Layer From Export");
                executeAndStore(ChangeEntityAttributesCommand::setForNodes({ layer }, Model::AttributeNames::LayerOmitFromExport, Model::AttributeValues::LayerOmitFromExportValue));
            } else {
                Transaction transaction(this, "Include Layer In Export");
                executeAndStore(ChangeEntityAttributesCommand::removeForNodes({ layer }, Model::AttributeNames::LayerOmitFromExport));
            }
        }

        void MapDocument::hide(const std::vector<Model::Node*> nodes) {
            const Transaction transaction(this, "Hide Objects");

            // Deselect any selected nodes inside `nodes`
            deselect(Model::collectSelectedNodes(nodes));

            // Reset visibility of any forced shown children of `nodes`
            downgradeShownToInherit(Model::collectDescendants(nodes));

            executeAndStore(SetVisibilityCommand::hide(nodes));
        }

        void MapDocument::hideSelection() {
            hide(m_selectedNodes.nodes());
        }

        void MapDocument::show(const std::vector<Model::Node*>& nodes) {
            executeAndStore(SetVisibilityCommand::show(nodes));
        }

        void MapDocument::showAll() {
            resetVisibility(Model::collectDescendants(kdl::vec_element_cast<Model::Node*>(m_world->allLayers())));
        }

        void MapDocument::ensureVisible(const std::vector<Model::Node*>& nodes) {
            executeAndStore(SetVisibilityCommand::ensureVisible(nodes));
        }

        void MapDocument::resetVisibility(const std::vector<Model::Node*>& nodes) {
            executeAndStore(SetVisibilityCommand::reset(nodes));
        }

        void MapDocument::lock(const std::vector<Model::Node*>& nodes) {
            const Transaction transaction(this, "Lock Objects");

            // Deselect any selected nodes inside `nodes`
            deselect(Model::collectSelectedNodes(nodes));

            // Reset lock state of any forced unlocked children of `nodes`
            downgradeUnlockedToInherit(Model::collectDescendants(nodes));

            executeAndStore(SetLockStateCommand::lock(nodes));
        }

        void MapDocument::unlock(const std::vector<Model::Node*>& nodes) {
            executeAndStore(SetLockStateCommand::unlock(nodes));
        }

        /**
         * Unlocks only those nodes from the given list whose lock state resolves to "locked"
         */
        void MapDocument::ensureUnlocked(const std::vector<Model::Node*>& nodes) {
            std::vector<Model::Node*> nodesToUnlock;
            for (auto* node : nodes) {
                if (node->locked()) {
                    nodesToUnlock.push_back(node);
                }
            }
            unlock(nodesToUnlock);
        }

        void MapDocument::resetLock(const std::vector<Model::Node*>& nodes) {
            executeAndStore(SetLockStateCommand::reset(nodes));
        }

        /**
         * This is called to clear the forced Visibility_Shown that was set on newly created nodes
         * so they could be visible if created in a hidden layer
         */
        void MapDocument::downgradeShownToInherit(const std::vector<Model::Node*>& nodes) {
            std::vector<Model::Node*> nodesToReset;
            for (auto* node : nodes) {
                if (node->visibilityState() == Model::VisibilityState::Visibility_Shown) {
                    nodesToReset.push_back(node);
                }
            }
            resetVisibility(nodesToReset);
        }

        /**
         * See downgradeShownToInherit
         */
        void MapDocument::downgradeUnlockedToInherit(const std::vector<Model::Node*>& nodes) {
            std::vector<Model::Node*> nodesToReset;
            for (auto* node : nodes) {
                if (node->lockState() == Model::LockState::Lock_Unlocked) {
                    nodesToReset.push_back(node);
                }
            }
            resetLock(nodesToReset);
        }

        void MapDocument::swapNodeContents(const std::string& commandName, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodesToSwap) {
            executeAndStore(std::make_unique<SwapNodeContentsCommand>(commandName, std::move(nodesToSwap)));
        }

        bool MapDocument::translateObjects(const vm::vec3& delta) {
            const auto result = executeAndStore(TransformObjectsCommand::translate(delta, pref(Preferences::TextureLock)));
            if (result->success()) {
                m_repeatStack->push([=]() { this->translateObjects(delta); });
                return true;
            }
            return false;
        }

        bool MapDocument::rotateObjects(const vm::vec3& center, const vm::vec3& axis, const FloatType angle) {
            const auto result = executeAndStore(TransformObjectsCommand::rotate(center, axis, angle, pref(Preferences::TextureLock)));
            if (result->success()) {
                m_repeatStack->push([=]() { this->rotateObjects(center, axis, angle); });
                return true;
            }
            return false;
        }

        bool MapDocument::scaleObjects(const vm::bbox3& oldBBox, const vm::bbox3& newBBox) {
            const auto result = executeAndStore(TransformObjectsCommand::scale(oldBBox, newBBox, pref(Preferences::TextureLock)));
            if (result->success()) {
                m_repeatStack->push([=]() { this->scaleObjects(oldBBox, newBBox); });
                return true;
            }
            return false;
        }

        bool MapDocument::scaleObjects(const vm::vec3& center, const vm::vec3& scaleFactors) {
            const auto result = executeAndStore(TransformObjectsCommand::scale(center, scaleFactors, pref(Preferences::TextureLock)));
            if (result->success()) {
                m_repeatStack->push([=]() { this->scaleObjects(center, scaleFactors); });
                return true;
            }
            return false;
        }

        bool MapDocument::shearObjects(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta) {
            const auto result = executeAndStore(TransformObjectsCommand::shearBBox(box, sideToShear, delta,  pref(Preferences::TextureLock)));
            if (result->success()) {
                m_repeatStack->push([=]() { this->shearObjects(box, sideToShear, delta); });
                return true;
            }
            return false;
        }

        bool MapDocument::flipObjects(const vm::vec3& center, const vm::axis::type axis) {
            const auto result = executeAndStore(TransformObjectsCommand::flip(center, axis, pref(Preferences::TextureLock)));
            if (result->success()) {
                m_repeatStack->push([=]() { this->flipObjects(center, axis); });
                return true;
            }
            return false;
        }

        bool MapDocument::createBrush(const std::vector<vm::vec3>& points) {
            Model::BrushBuilder builder(m_world.get(), m_worldBounds, m_game->defaultFaceAttribs());
            return builder.createBrush(points, currentTextureName())
                .visit(kdl::overload(
                    [&](Model::Brush&& b) {
                        Model::BrushNode* brushNode = m_world->createBrush(std::move(b));
                        
                        Transaction transaction(this, "Create Brush");
                        deselectAll();
                        addNode(brushNode, parentForNodes());
                        select(brushNode);
                        return true;
                    },
                    [&](const Model::BrushError e) {
                        error() << "Could not create brush: " << e;
                        return false;
                    }
                ));
        }

        bool MapDocument::csgConvexMerge() {
            if (!hasSelectedBrushFaces() && !selectedNodes().hasOnlyBrushes()) {
                return false;
            }

            std::vector<vm::vec3> points;

            if (hasSelectedBrushFaces()) {
                for (const auto& handle : selectedBrushFaces()) {
                    for (const Model::BrushVertex* vertex : handle.face().vertices()) {
                        points.push_back(vertex->position());
                    }
                }
            } else if (selectedNodes().hasOnlyBrushes()) {
                for (const Model::BrushNode* brushNode : selectedNodes().brushes()) {
                    const Model::Brush& brush = brushNode->brush();
                    for (const Model::BrushVertex* vertex : brush.vertices()) {
                        points.push_back(vertex->position());
                    }
                }
            }

            Model::Polyhedron3 polyhedron(std::move(points));
            if (!polyhedron.polyhedron() || !polyhedron.closed()) {
                return false;
            }

            const Model::BrushBuilder builder(m_world.get(), m_worldBounds, m_game->defaultFaceAttribs());
            return builder.createBrush(polyhedron, currentTextureName())
                .visit(kdl::overload(
                    [&](Model::Brush&& b) {
                        for (const Model::BrushNode* selectedBrushNode : selectedNodes().brushes()) {
                            b.cloneFaceAttributesFrom(selectedBrushNode->brush());
                        }

                        // The nodelist is either empty or contains only brushes.
                        const auto toRemove = selectedNodes().nodes();

                        // We could be merging brushes that have different parents; use the parent of the first brush.
                        Model::Node* parentNode = nullptr;
                        if (!selectedNodes().brushes().empty()) {
                            parentNode = selectedNodes().brushes().front()->parent();
                        } else if (!selectedBrushFaces().empty()) {
                            parentNode = selectedBrushFaces().front().node()->parent();
                        } else {
                            parentNode = parentForNodes();
                        }

                        Model::BrushNode* brushNode = new Model::BrushNode(std::move(b));
                        
                        const Transaction transaction(this, "CSG Convex Merge");
                        deselectAll();
                        addNode(brushNode, parentNode);
                        removeNodes(toRemove);
                        select(brushNode);
                        return true;
                    },
                    [&](const Model::BrushError e) {
                        error() << "Could not create brush: " << e;
                        return false;
                    }
                ));
        }

        bool MapDocument::csgSubtract() {
            const auto subtrahendNodes = std::vector<Model::BrushNode*>{selectedNodes().brushes()};
            if (subtrahendNodes.empty()) {
                return false;
            }

            Transaction transaction(this, "CSG Subtract");
            // Select touching, but don't delete the subtrahends yet
            selectTouching(false);

            const auto minuendNodes = std::vector<Model::BrushNode*>{selectedNodes().brushes()};

            std::map<Model::Node*, std::vector<Model::Node*>> toAdd;
            std::vector<Model::Node*> toRemove(std::begin(subtrahendNodes), std::end(subtrahendNodes));
            const std::vector<const Model::Brush*> subtrahends = kdl::vec_transform(subtrahendNodes, [](const auto* subtrahendNode) { return &subtrahendNode->brush(); });
            
            for (Model::BrushNode* minuendNode : minuendNodes) {
                const Model::Brush& minuend = minuendNode->brush();
                minuend.subtract(*m_world, m_worldBounds, currentTextureName(), subtrahends)
                    .visit(kdl::overload(
                        [&](const std::vector<Model::Brush>& brushes) {
                            if (!brushes.empty()) {
                                std::vector<Model::BrushNode*> resultNodes = kdl::vec_transform(std::move(brushes), [&](auto b) { return m_world->createBrush(std::move(b)); });
                                auto& toAddForParent = toAdd[minuendNode->parent()];
                                toAddForParent = kdl::vec_concat(std::move(toAddForParent), std::move(resultNodes));
                            }
                        },
                        [&](const Model::BrushError e) {
                            error() << "Could not create brush: " << e;
                        }
                    ));
                toRemove.push_back(minuendNode);
            }

            deselectAll();
            const std::vector<Model::Node*> added = addNodes(toAdd);
            removeNodes(toRemove);
            select(added);

            return true;
        }

        bool MapDocument::csgIntersect() {
            const std::vector<Model::BrushNode*> brushes = selectedNodes().brushes();
            if (brushes.size() < 2u) {
                return false;
            }

            Model::Brush intersection = brushes.front()->brush();

            bool valid = true;
            for (auto it = std::next(std::begin(brushes)), end = std::end(brushes); it != end && valid; ++it) {
                Model::BrushNode* brushNode = *it;
                const Model::Brush& brush = brushNode->brush();
                valid = intersection.intersect(m_worldBounds, brush)
                    .visit(kdl::overload(
                        [&](Model::Brush&& b) {
                            intersection = std::move(b);
                            return true;
                        },
                        [&](const Model::BrushError e) {
                            error() << "Could not intersect brushes: " << e;
                            return false;
                        }
                    ));
            }

            const std::vector<Model::Node*> toRemove(std::begin(brushes), std::end(brushes));

            Transaction transaction(this, "CSG Intersect");
            deselect(toRemove);

            if (valid) {
                Model::BrushNode* intersectionNode = new Model::BrushNode(std::move(intersection));
                addNode(intersectionNode, parentForNodes(toRemove));
                removeNodes(toRemove);
                select(intersectionNode);
            } else {
                removeNodes(toRemove);
            }

            return true;
        }

        bool MapDocument::csgHollow() {
            const std::vector<Model::BrushNode*> brushNodes = selectedNodes().brushes();
            if (brushNodes.empty()) {
                return false;
            }

            std::map<Model::Node*, std::vector<Model::Node*>> toAdd;
            std::vector<Model::Node*> toRemove;

            for (Model::BrushNode* brushNode : brushNodes) {
                const Model::Brush& brush = brushNode->brush();

                // make an shrunken copy of brush
                brush.expand(m_worldBounds, -1.0 * static_cast<FloatType>(m_grid->actualSize()), true)
                    .and_then(
                        [&](const Model::Brush& shrunken) {
                            return brush.subtract(*m_world, m_worldBounds, currentTextureName(), shrunken);
                        }
                    ).visit(kdl::overload(
                        [&](const std::vector<Model::Brush>& fragments) {
                            auto fragmentNodes = kdl::vec_transform(std::move(fragments), [](auto&& b) {
                                return new Model::BrushNode(std::move(b));
                            });

                            auto& toAddForParent = toAdd[brushNode->parent()];
                            toAddForParent = kdl::vec_concat(std::move(toAddForParent), fragmentNodes);
                            toRemove.push_back(brushNode);
                        },
                        [&](const Model::BrushError e) {
                            error() << "Could not hollow brush: " << e;
                        }
                    ));
                
            }

            Transaction transaction(this, "CSG Hollow");
            deselectAll();
            const std::vector<Model::Node*> added = addNodes(toAdd);
            removeNodes(toRemove);
            select(added);

            return true;
        }

        bool MapDocument::clipBrushes(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3) {
            const std::vector<Model::BrushNode*>& brushes = m_selectedNodes.brushes();
            std::map<Model::Node*, std::vector<Model::Node*>> clippedBrushes;

            for (const Model::BrushNode* originalBrush : brushes) {
                const bool success = m_world->createFace(p1, p2, p3, Model::BrushFaceAttributes(currentTextureName()))
                    .and_then(
                        [&](Model::BrushFace&& clipFace) {
                            return originalBrush->brush().clip(m_worldBounds, std::move(clipFace));
                        }
                    ).and_then(
                        [&](Model::Brush&& clippedBrush) {
                            clippedBrushes[originalBrush->parent()].push_back(new Model::BrushNode(std::move(clippedBrush)));
                            return kdl::void_result;
                        }
                    ).handle_errors(
                        [&](const Model::BrushError e) {
                            error() << "Could not clip brushes: " << e;
                        }
                    );
                
                if (!success) {
                    kdl::map_clear_and_delete(clippedBrushes);
                    return false;
                }
            }

            Transaction transaction(this, "Clip Brushes");
            const std::vector<Model::Node*> toRemove(std::begin(brushes), std::end(brushes));
            deselectAll();
            removeNodes(toRemove);
            select(addNodes(clippedBrushes));

            return true;
        }

        bool MapDocument::setAttribute(const std::string& name, const std::string& value) {
            const auto result = executeAndStore(ChangeEntityAttributesCommand::set(name, value));
            return result->success();
        }

        bool MapDocument::renameAttribute(const std::string& oldName, const std::string& newName) {
            const auto result = executeAndStore(ChangeEntityAttributesCommand::rename(oldName, newName));
            return result->success();
        }

        bool MapDocument::removeAttribute(const std::string& name) {
            const auto result = executeAndStore(ChangeEntityAttributesCommand::remove(name));
            return result->success();
        }

        bool MapDocument::convertEntityColorRange(const std::string& name, Assets::ColorRange::Type range) {
            const auto result = executeAndStore(ConvertEntityColorCommand::convert(name, range));
            return result->success();
        }

        bool MapDocument::updateSpawnflag(const std::string& name, const size_t flagIndex, const bool setFlag) {
            const auto result = executeAndStore(UpdateEntitySpawnflagCommand::update(name, flagIndex, setFlag));
            return result->success();
        }

        bool MapDocument::resizeBrushes(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) {
            const auto result = executeAndStore(ResizeBrushesCommand::resize(faces, delta));
            return result->success();
        }

        bool MapDocument::setFaceAttributes(const Model::BrushFaceAttributes& attributes) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setAll(attributes);
            return setFaceAttributes(request);
        }

        bool MapDocument::setFaceAttributesExceptContentFlags(const Model::BrushFaceAttributes& attributes) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setAllExceptContentFlags(attributes);
            return setFaceAttributes(request);
        }

        bool MapDocument::setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) {
            const auto result = executeAndStore(ChangeBrushFaceAttributesCommand::command(request));
            return result->success();
        }

        bool MapDocument::copyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle) {
            const auto result = executeAndStore(CopyTexCoordSystemFromFaceCommand::command(coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle));
            return result->success();
        }

        bool MapDocument::moveTextures(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) {
            const auto result = executeAndStore(MoveTexturesCommand::move(cameraUp, cameraRight, delta));
            return result->success();
        }

        bool MapDocument::rotateTextures(const float angle) {
            const auto result = executeAndStore(RotateTexturesCommand::rotate(angle));
            return result->success();
        }

        bool MapDocument::shearTextures(const vm::vec2f& factors) {
            const auto result = executeAndStore(ShearTexturesCommand::shear(factors));
            return result->success();
        }

        bool MapDocument::snapVertices(const FloatType snapTo) {
            const auto result = executeAndStore(SnapBrushVerticesCommand::snap(snapTo));
            return result->success();
        }

        MapDocument::MoveVerticesResult MapDocument::moveVertices(const std::map<vm::vec3, std::vector<Model::BrushNode*>>& vertices, const vm::vec3& delta) {
            const auto result = executeAndStore(MoveBrushVerticesCommand::move(vertices, delta));
            const auto* moveVerticesResult = dynamic_cast<MoveBrushVerticesCommandResult*>(result.get());
            ensure(moveVerticesResult != nullptr, "command processor returned unexpected command result type");

            return MoveVerticesResult(moveVerticesResult->success(), moveVerticesResult->hasRemainingVertices());
        }

        bool MapDocument::moveEdges(const std::map<vm::segment3, std::vector<Model::BrushNode*>>& edges, const vm::vec3& delta) {
            const auto result = executeAndStore(MoveBrushEdgesCommand::move(edges, delta));
            return result->success();
        }

        bool MapDocument::moveFaces(const std::map<vm::polygon3, std::vector<Model::BrushNode*>>& faces, const vm::vec3& delta) {
            const auto result = executeAndStore(MoveBrushFacesCommand::move(faces, delta));
            return result->success();
        }

        bool MapDocument::addVertices(const std::map<vm::vec3, std::vector<Model::BrushNode*>>& vertices) {
            const auto result = executeAndStore(AddBrushVerticesCommand::add(vertices));
            return result->success();
        }

        bool MapDocument::removeVertices(const std::map<vm::vec3, std::vector<Model::BrushNode*>>& vertices) {
            const auto result = executeAndStore(RemoveBrushVerticesCommand::remove(vertices));
            return result->success();
        }

        bool MapDocument::removeEdges(const std::map<vm::segment3, std::vector<Model::BrushNode*>>& edges) {
            const auto result = executeAndStore(RemoveBrushEdgesCommand::remove(edges));
            return result->success();
        }

        bool MapDocument::removeFaces(const std::map<vm::polygon3, std::vector<Model::BrushNode*>>& faces) {
            const auto result = executeAndStore(RemoveBrushFacesCommand::remove(faces));
            return result->success();
        }

        void MapDocument::printVertices() {
            if (hasSelectedBrushFaces()) {
                for (const auto& handle : m_selectedBrushFaces) {
                    std::stringstream str;
                    str.precision(17);
                    for (const Model::BrushVertex* vertex : handle.face().vertices()) {
                        str << "(" << vertex->position() << ") ";
                    }
                    info(str.str());
                }
            } else if (selectedNodes().hasBrushes()) {
                for (const Model::BrushNode* brushNode : selectedNodes().brushes()) {
                    const Model::Brush& brush = brushNode->brush();

                    std::stringstream str;
                    str.precision(17);
                    for (const Model::BrushVertex* vertex : brush.vertices()) {
                        str << vertex->position() << " ";
                    }
                    info(str.str());
                }
            }
        }

        class ThrowExceptionCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ThrowExceptionCommand>;
        public:
            ThrowExceptionCommand() : DocumentCommand(Type, "Throw Exception") {}

        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade*) override {
                throw CommandProcessorException();
            }

            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade*) override {
                return std::make_unique<CommandResult>(true);
            }

            bool doCollateWith(UndoableCommand*) override {
                return false;
            }
        };

        const ThrowExceptionCommand::CommandType ThrowExceptionCommand::Type = Command::freeType();

        bool MapDocument::throwExceptionDuringCommand() {
            const auto result = executeAndStore(std::make_unique<ThrowExceptionCommand>());
            return result->success();
        }

        bool MapDocument::canUndoCommand() const {
            return doCanUndoCommand();
        }

        bool MapDocument::canRedoCommand() const {
            return doCanRedoCommand();
        }

        const std::string& MapDocument::undoCommandName() const {
            return doGetUndoCommandName();
        }

        const std::string& MapDocument::redoCommandName() const {
            return doGetRedoCommandName();
        }

        void MapDocument::undoCommand() {
            doUndoCommand();
        }

        void MapDocument::redoCommand() {
            doRedoCommand();
        }

        bool MapDocument::canRepeatCommands() const {
            return m_repeatStack->size() > 0u;
        }

        void MapDocument::repeatCommands() {
            m_repeatStack->repeat();
        }

        void MapDocument::clearRepeatableCommands() {
            m_repeatStack->clear();
        }

        void MapDocument::startTransaction(const std::string& name) {
            debug("Starting transaction '" + name + "'");
            doStartTransaction(name);
        }

        void MapDocument::rollbackTransaction() {
            debug("Rolling back transaction");
            doRollbackTransaction();
        }

        void MapDocument::commitTransaction() {
            debug("Committing transaction");
            doCommitTransaction();
        }

        void MapDocument::cancelTransaction() {
            debug("Cancelling transaction");
            doRollbackTransaction();
            doCommitTransaction();
        }

        std::unique_ptr<CommandResult> MapDocument::execute(std::unique_ptr<Command>&& command) {
            return doExecute(std::move(command));
        }

        std::unique_ptr<CommandResult> MapDocument::executeAndStore(std::unique_ptr<UndoableCommand>&& command) {
            return doExecuteAndStore(std::move(command));
        }

        void MapDocument::commitPendingAssets() {
            m_textureManager->commitChanges();
        }

        void MapDocument::pick(const vm::ray3& pickRay, Model::PickResult& pickResult) const {
            if (m_world != nullptr)
                m_world->pick(pickRay, pickResult);
        }

        std::vector<Model::Node*> MapDocument::findNodesContaining(const vm::vec3& point) const {
            std::vector<Model::Node*> result;
            if (m_world != nullptr) {
                m_world->findNodesContaining(point, result);
            }
            return result;
        }

        void MapDocument::createWorld(const Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game) {
            m_worldBounds = worldBounds;
            m_game = game;
            m_world = m_game->newMap(mapFormat, m_worldBounds, logger());
            performSetCurrentLayer(m_world->defaultLayer());

            updateGameSearchPaths();
            setPath(IO::Path(DefaultDocumentName));
        }

        void MapDocument::loadWorld(const Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game, const IO::Path& path) {
            m_worldBounds = worldBounds;
            m_game = game;
            m_world = m_game->loadMap(mapFormat, m_worldBounds, path, logger());
            performSetCurrentLayer(m_world->defaultLayer());

            updateGameSearchPaths();
            setPath(path);
        }

        void MapDocument::clearWorld() {
            m_world.reset();
            m_currentLayer = nullptr;
        }

        Assets::EntityDefinitionFileSpec MapDocument::entityDefinitionFile() const {
            if (m_world != nullptr) {
                return m_game->extractEntityDefinitionFile(*m_world);
            } else {
                return Assets::EntityDefinitionFileSpec();
            }
        }

        std::vector<Assets::EntityDefinitionFileSpec> MapDocument::allEntityDefinitionFiles() const {
            return m_game->allEntityDefinitionFiles();
        }

        void MapDocument::setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec) {
            executeAndStore(EntityDefinitionFileCommand::set(spec));
        }

        void MapDocument::setEntityDefinitions(const std::vector<Assets::EntityDefinition*>& definitions) {
            m_entityDefinitionManager->setDefinitions(definitions);
        }

        std::vector<IO::Path> MapDocument::enabledTextureCollections() const {
            return m_game->extractTextureCollections(*m_world);
        }

        std::vector<IO::Path> MapDocument::availableTextureCollections() const {
            return m_game->findTextureCollections();
        }

        void MapDocument::setEnabledTextureCollections(const std::vector<IO::Path>& paths) {
            executeAndStore(SetTextureCollectionsCommand::set(paths));
        }

        void MapDocument::reloadTextureCollections() {
            const std::vector<Model::Node*> nodes(1, m_world.get());
            Notifier<const std::vector<Model::Node*>&>::NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            Notifier<>::NotifyBeforeAndAfter notifyTextureCollections(textureCollectionsWillChangeNotifier, textureCollectionsDidChangeNotifier);

            info("Reloading texture collections");
            reloadTextures();
            setTextures();
            initializeNodeTags(this);
        }

        void MapDocument::reloadEntityDefinitions() {
            auto oldSpec = entityDefinitionFile();
            setEntityDefinitionFile(oldSpec);
        }

        void MapDocument::loadAssets() {
            loadEntityDefinitions();
            setEntityDefinitions();
            loadEntityModels();
            loadTextures();
            setTextures();
        }

        void MapDocument::unloadAssets() {
            unloadEntityDefinitions();
            unloadEntityModels();
            unloadTextures();
        }

        void MapDocument::loadEntityDefinitions() {
            const Assets::EntityDefinitionFileSpec spec = entityDefinitionFile();
            try {
                const IO::Path path = m_game->findEntityDefinitionFile(spec, externalSearchPaths());
                IO::SimpleParserStatus status(logger());
                m_entityDefinitionManager->loadDefinitions(path, *m_game, status);
                info("Loaded entity definition file " + path.lastComponent().asString());

                createEntityDefinitionActions();
            } catch (const Exception& e) {
                if (spec.builtin()) {
                    error() << "Could not load builtin entity definition file '" << spec.path() << "': " << e.what();
                } else {
                    error() << "Could not load external entity definition file '" << spec.path() << "': " << e.what();
                }
            }
        }

        void MapDocument::unloadEntityDefinitions() {
            unsetEntityDefinitions();
            m_entityDefinitionManager->clear();
            m_entityDefinitionActions.clear();
        }

        void MapDocument::loadEntityModels() {
            m_entityModelManager->setLoader(m_game.get());
            setEntityModels();
        }

        void MapDocument::unloadEntityModels() {
            clearEntityModels();
            m_entityModelManager->setLoader(nullptr);
        }

        void MapDocument::reloadTextures() {
            unloadTextures();
            m_game->reloadShaders();
            loadTextures();
        }

        void MapDocument::loadTextures() {
            try {
                const IO::Path docDir = m_path.isEmpty() ? IO::Path() : m_path.deleteLastComponent();
                m_game->loadTextureCollections(*m_world, docDir, *m_textureManager, logger());
            } catch (const Exception& e) {
                error(e.what());
            }
        }

        void MapDocument::unloadTextures() {
            unsetTextures();
            m_textureManager->clear();
        }

        static auto makeSetTexturesVisitor(Assets::TextureManager& manager) {
            return kdl::overload(
                [] (auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::EntityNode* entity) { entity->visitChildren(thisLambda); },
                [&](Model::BrushNode* brushNode) { 
                    const Model::Brush& brush = brushNode->brush();
                    for (size_t i = 0u; i < brush.faceCount(); ++i) {
                        const Model::BrushFace& face = brush.face(i);
                        Assets::Texture* texture = manager.texture(face.attributes().textureName());
                        brushNode->setFaceTexture(i, texture);
                    }
                }
            );
        }

        static auto makeUnsetTexturesVisitor() {
            return kdl::overload (
                [](auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::EntityNode* entity) { entity->visitChildren(thisLambda); },
                [](Model::BrushNode* brushNode) { 
                    const Model::Brush& brush = brushNode->brush();
                    for (size_t i = 0u; i < brush.faceCount(); ++i) {
                        brushNode->setFaceTexture(i, nullptr);
                    }
                }
            );
        }

        void MapDocument::setTextures() {
            m_world->accept(makeSetTexturesVisitor(*m_textureManager));
            textureUsageCountsDidChangeNotifier();
        }

        void MapDocument::setTextures(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeSetTexturesVisitor(*m_textureManager));
            textureUsageCountsDidChangeNotifier();
        }

        void MapDocument::setTextures(const std::vector<Model::BrushFaceHandle>& faceHandles) {
            for (const auto& faceHandle : faceHandles) {
                Model::BrushNode* node = faceHandle.node();
                const Model::BrushFace& face = faceHandle.face();
                Assets::Texture* texture = m_textureManager->texture(face.attributes().textureName());
                node->setFaceTexture(faceHandle.faceIndex(), texture);
            }
            textureUsageCountsDidChangeNotifier();
        }

        void MapDocument::unsetTextures() {
            m_world->accept(makeUnsetTexturesVisitor());
            textureUsageCountsDidChangeNotifier();
        }

        void MapDocument::unsetTextures(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeUnsetTexturesVisitor());
            textureUsageCountsDidChangeNotifier();
        }

        static auto makeSetEntityDefinitionsVisitor(Assets::EntityDefinitionManager& manager) {
            // this helper lambda must be captured by value
            const auto setEntityDefinition = [&](auto* attributable) {
                auto* definition = manager.definition(attributable);
                attributable->setDefinition(definition);
            };

            return kdl::overload(
                [=](auto&& thisLambda, Model::WorldNode* world) { setEntityDefinition(world); world->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
                [=](Model::EntityNode* entity)                  { setEntityDefinition(entity); },
                [] (Model::BrushNode*) {}
            );
        }

        static auto makeUnsetEntityDefinitionsVisitor() {
            return kdl::overload(
                [](auto&& thisLambda, Model::WorldNode* world) { world->setDefinition(nullptr); world->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
                [](Model::EntityNode* entity)                  { entity->setDefinition(nullptr); },
                [](Model::BrushNode*) {}
            );
        }

        void MapDocument::setEntityDefinitions() {
            m_world->accept(makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
        }

        void MapDocument::setEntityDefinitions(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
        }

        void MapDocument::unsetEntityDefinitions() {
            m_world->accept(makeUnsetEntityDefinitionsVisitor());
        }

        void MapDocument::unsetEntityDefinitions(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeUnsetEntityDefinitionsVisitor());
        }

        void MapDocument::reloadEntityDefinitionsInternal() {
            unloadEntityDefinitions();
            clearEntityModels();
            loadEntityDefinitions();
            setEntityDefinitions();
            setEntityModels();
        }

        void MapDocument::clearEntityModels() {
            unsetEntityModels();
            m_entityModelManager->clear();
        }

        static auto makeSetEntityModelsVisitor(Logger& logger, Assets::EntityModelManager& manager) {
            return kdl::overload(
                [] (auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
                [&](Model::EntityNode* entityNode)                  {
                    const auto modelSpec = Assets::safeGetModelSpecification(logger, entityNode->entity().classname(), [&]() {
                        return entityNode->entity().modelSpecification();
                    });
                    const auto* frame = manager.frame(modelSpec);
                    entityNode->setModelFrame(frame);
                },
                [] (Model::BrushNode*) {}
            );
        }

        static auto makeUnsetEntityModelsVisitor() {
            return kdl::overload(
                [](auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
                [](Model::EntityNode* entity)                  { entity->setModelFrame(nullptr); },
                [](Model::BrushNode*) {}
            );
        }

        void MapDocument::setEntityModels() {
            m_world->accept(makeSetEntityModelsVisitor(*this, *m_entityModelManager));
        }

        void MapDocument::setEntityModels(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeSetEntityModelsVisitor(*this, *m_entityModelManager));
        }

        void MapDocument::unsetEntityModels() {
            m_world->accept(makeUnsetEntityModelsVisitor());
        }

        void MapDocument::unsetEntityModels(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeUnsetEntityModelsVisitor());
        }

        std::vector<IO::Path> MapDocument::externalSearchPaths() const {
            std::vector<IO::Path> searchPaths;
            if (!m_path.isEmpty() && m_path.isAbsolute()) {
                searchPaths.push_back(m_path.deleteLastComponent());
            }

            const IO::Path gamePath = m_game->gamePath();
            if (!gamePath.isEmpty()) {
                searchPaths.push_back(gamePath);
            }

            searchPaths.push_back(IO::SystemPaths::appDirectory());
            return searchPaths;
        }

        void MapDocument::updateGameSearchPaths() {
            const std::vector<IO::Path> additionalSearchPaths = IO::Path::asPaths(mods());
            m_game->setAdditionalSearchPaths(additionalSearchPaths, logger());
        }

        std::vector<std::string> MapDocument::mods() const {
            return m_game->extractEnabledMods(*m_world);
        }

        void MapDocument::setMods(const std::vector<std::string>& mods) {
            executeAndStore(SetModsCommand::set(mods));
        }

        std::string MapDocument::defaultMod() const {
            return m_game->defaultMod();
        }

        /**
         * Note if bounds.source is SoftMapBoundsType::Game, bounds.bounds is ignored.
         */
        void MapDocument::setSoftMapBounds(const Model::Game::SoftMapBounds& bounds) {
            switch (bounds.source) {
                case Model::Game::SoftMapBoundsType::Map:
                    if (!bounds.bounds.has_value()) {
                        // Set the worldspawn key AttributeNames::SoftMaxMapSize's value to the empty string
                        // to indicate that we are overriding the Game's bounds with unlimited.
                        executeAndStore(ChangeEntityAttributesCommand::setForNodes({world()}, Model::AttributeNames::SoftMapBounds, Model::AttributeValues::NoSoftMapBounds));
                    } else {
                        executeAndStore(ChangeEntityAttributesCommand::setForNodes({world()}, Model::AttributeNames::SoftMapBounds, IO::serializeSoftMapBoundsString(*bounds.bounds)));
                    }
                    break;
                case Model::Game::SoftMapBoundsType::Game:
                    // Unset the map's setting
                    executeAndStore(ChangeEntityAttributesCommand::removeForNodes({world()}, Model::AttributeNames::SoftMapBounds));
                    break;
                switchDefault()
            }
        }

        Model::Game::SoftMapBounds MapDocument::softMapBounds() const {
            if (!m_world) {
                return {Model::Game::SoftMapBoundsType::Game, std::nullopt};
            }
            return m_game->extractSoftMapBounds(*m_world);
        }

        void MapDocument::setIssueHidden(Model::Issue* issue, const bool hidden) {
            doSetIssueHidden(issue, hidden);
        }

        void MapDocument::registerIssueGenerators() {
            ensure(m_world != nullptr, "world is null");
            ensure(m_game.get() != nullptr, "game is null");

            m_world->registerIssueGenerator(new Model::MissingClassnameIssueGenerator());
            m_world->registerIssueGenerator(new Model::MissingDefinitionIssueGenerator());
            m_world->registerIssueGenerator(new Model::MissingModIssueGenerator(m_game));
            m_world->registerIssueGenerator(new Model::EmptyGroupIssueGenerator());
            m_world->registerIssueGenerator(new Model::EmptyBrushEntityIssueGenerator());
            m_world->registerIssueGenerator(new Model::PointEntityWithBrushesIssueGenerator());
            m_world->registerIssueGenerator(new Model::LinkSourceIssueGenerator());
            m_world->registerIssueGenerator(new Model::LinkTargetIssueGenerator());
            m_world->registerIssueGenerator(new Model::NonIntegerVerticesIssueGenerator());
            m_world->registerIssueGenerator(new Model::MixedBrushContentsIssueGenerator());
            m_world->registerIssueGenerator(new Model::WorldBoundsIssueGenerator(worldBounds()));
            m_world->registerIssueGenerator(new Model::SoftMapBoundsIssueGenerator(m_game, m_world.get()));
            m_world->registerIssueGenerator(new Model::EmptyAttributeNameIssueGenerator());
            m_world->registerIssueGenerator(new Model::EmptyAttributeValueIssueGenerator());
            m_world->registerIssueGenerator(new Model::LongAttributeNameIssueGenerator(m_game->maxPropertyLength()));
            m_world->registerIssueGenerator(new Model::LongAttributeValueIssueGenerator(m_game->maxPropertyLength()));
            m_world->registerIssueGenerator(new Model::AttributeNameWithDoubleQuotationMarksIssueGenerator());
            m_world->registerIssueGenerator(new Model::AttributeValueWithDoubleQuotationMarksIssueGenerator());
            m_world->registerIssueGenerator(new Model::InvalidTextureScaleIssueGenerator());
        }

        void MapDocument::registerSmartTags() {
            ensure(m_game.get() != nullptr, "game is null");

            m_tagManager->clearSmartTags();
            m_tagManager->registerSmartTags(m_game->smartTags());
        }

        const std::vector<Model::SmartTag>& MapDocument::smartTags() const {
            return m_tagManager->smartTags();
        }

        bool MapDocument::isRegisteredSmartTag(const std::string& name) const {
            return m_tagManager->isRegisteredSmartTag(name);
        }

        const Model::SmartTag& MapDocument::smartTag(const std::string& name) const {
            return m_tagManager->smartTag(name);
        }

        bool MapDocument::isRegisteredSmartTag(const size_t index) const {
            return m_tagManager->isRegisteredSmartTag(index);
        }

        const Model::SmartTag& MapDocument::smartTag(const size_t index) const {
            return m_tagManager->smartTag(index);
        }

        static auto makeInitializeNodeTagsVisitor(Model::TagManager& tagManager) {
            return kdl::overload(
                [&](auto&& thisLambda, Model::WorldNode* world) { world->initializeTags(tagManager); world->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::LayerNode* layer) { layer->initializeTags(tagManager); layer->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::GroupNode* group) { group->initializeTags(tagManager); group->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::EntityNode* entity) { entity->initializeTags(tagManager); entity->visitChildren(thisLambda); },
                [&](Model::BrushNode* brush) { brush->initializeTags(tagManager); }
            );
        }

        static auto makeClearNodeTagsVisitor() {
            return kdl::overload(
                [](auto&& thisLambda, Model::WorldNode* world) { world->clearTags(); world->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::LayerNode* layer) { layer->clearTags(); layer->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::GroupNode* group) { group->clearTags(); group->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::EntityNode* entity) { entity->clearTags(); entity->visitChildren(thisLambda); },
                [](Model::BrushNode* brush) { brush->clearTags(); }
            );
        }

        void MapDocument::initializeNodeTags(MapDocument* document) {
            assert(document == this);
            unused(document);
            m_world->accept(makeInitializeNodeTagsVisitor(*m_tagManager));
        }

        void MapDocument::initializeNodeTags(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeInitializeNodeTagsVisitor(*m_tagManager));
        }

        void MapDocument::clearNodeTags(const std::vector<Model::Node*>& nodes) {
            Model::Node::visitAll(nodes, makeClearNodeTagsVisitor());
        }

        void MapDocument::updateNodeTags(const std::vector<Model::Node*>& nodes) {
            for (auto* node : nodes) {
                node->updateTags(*m_tagManager);
            }
        }

        void MapDocument::updateFaceTags(const std::vector<Model::BrushFaceHandle>& faceHandles) {
            for (const auto& faceHandle : faceHandles) {
                Model::BrushNode* node = faceHandle.node();
                node->updateFaceTags(faceHandle.faceIndex(), *m_tagManager);
            }
        }

        void MapDocument::updateAllFaceTags() {
            m_world->accept(kdl::overload(
                [] (auto&& thisLambda, Model::WorldNode* world)   { world->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::LayerNode* layer)   { layer->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::GroupNode* group)   { group->visitChildren(thisLambda); },
                [] (auto&& thisLambda, Model::EntityNode* entity) { entity->visitChildren(thisLambda); },
                [&](Model::BrushNode* brush)                      { brush->initializeTags(*m_tagManager); }
            ));
        }

        bool MapDocument::persistent() const {
            return m_path.isAbsolute() && IO::Disk::fileExists(IO::Disk::fixPath(m_path));
        }

        std::string MapDocument::filename() const {
            if (m_path.isEmpty()) {
                return "";
            }
            return  m_path.lastComponent().asString();
        }

        const IO::Path& MapDocument::path() const {
            return m_path;
        }

        void MapDocument::setPath(const IO::Path& path) {
            m_path = path;
        }

        bool MapDocument::modified() const {
            return m_modificationCount != m_lastSaveModificationCount;
        }

        size_t MapDocument::modificationCount() const {
            return m_modificationCount;
        }

        void MapDocument::setLastSaveModificationCount() {
            m_lastSaveModificationCount = m_modificationCount;
            documentModificationStateDidChangeNotifier();
        }

        void MapDocument::clearModificationCount() {
            m_lastSaveModificationCount = m_modificationCount = 0;
            documentModificationStateDidChangeNotifier();
        }

        void MapDocument::bindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapDocument::preferenceDidChange);
            m_editorContext->editorContextDidChangeNotifier.addObserver(editorContextDidChangeNotifier);
            commandDoneNotifier.addObserver(this, &MapDocument::commandDone);
            commandUndoneNotifier.addObserver(this, &MapDocument::commandUndone);

            // tag management
            documentWasNewedNotifier.addObserver(this, &MapDocument::initializeNodeTags);
            documentWasLoadedNotifier.addObserver(this, &MapDocument::initializeNodeTags);
            nodesWereAddedNotifier.addObserver(this, &MapDocument::initializeNodeTags);
            nodesWillBeRemovedNotifier.addObserver(this, &MapDocument::clearNodeTags);
            nodesDidChangeNotifier.addObserver(this, &MapDocument::updateNodeTags);
            brushFacesDidChangeNotifier.addObserver(this, &MapDocument::updateFaceTags);
            modsDidChangeNotifier.addObserver(this, &MapDocument::updateAllFaceTags);
            textureCollectionsDidChangeNotifier.addObserver(this, &MapDocument::updateAllFaceTags);
        }

        void MapDocument::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapDocument::preferenceDidChange);
            m_editorContext->editorContextDidChangeNotifier.removeObserver(editorContextDidChangeNotifier);
            commandDoneNotifier.removeObserver(this, &MapDocument::commandDone);
            commandUndoneNotifier.removeObserver(this, &MapDocument::commandUndone);

            // tag management
            documentWasNewedNotifier.removeObserver(this, &MapDocument::initializeNodeTags);
            documentWasLoadedNotifier.removeObserver(this, &MapDocument::initializeNodeTags);
            nodesWereAddedNotifier.removeObserver(this, &MapDocument::initializeNodeTags);
            nodesWillBeRemovedNotifier.removeObserver(this, &MapDocument::clearNodeTags);
            nodesDidChangeNotifier.removeObserver(this, &MapDocument::updateNodeTags);
            brushFacesDidChangeNotifier.removeObserver(this, &MapDocument::updateFaceTags);
            modsDidChangeNotifier.removeObserver(this, &MapDocument::updateAllFaceTags);
            textureCollectionsDidChangeNotifier.removeObserver(this, &MapDocument::updateAllFaceTags);
        }

        void MapDocument::preferenceDidChange(const IO::Path& path) {
            if (isGamePathPreference(path)) {
                const Model::GameFactory& gameFactory = Model::GameFactory::instance();
                const IO::Path newGamePath = gameFactory.gamePath(m_game->gameName());
                m_game->setGamePath(newGamePath, logger());

                clearEntityModels();
                setEntityModels();

                reloadTextures();
                setTextures();
            } else if (path == Preferences::TextureMinFilter.path() ||
                       path == Preferences::TextureMagFilter.path()) {
                m_entityModelManager->setTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
                m_textureManager->setTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
            }
        }

        void MapDocument::commandDone(Command* command) {
            debug() << "Command " << command->name() << "' executed";
        }

        void MapDocument::commandUndone(UndoableCommand* command) {
            debug() << "Command " << command->name() << " undone";
        }

        Transaction::Transaction(std::weak_ptr<MapDocument> document, const std::string& name) :
        m_document(kdl::mem_lock(document).get()),
        m_cancelled(false) {
            begin(name);
        }

        Transaction::Transaction(std::shared_ptr<MapDocument> document, const std::string& name) :
        m_document(document.get()),
        m_cancelled(false) {
            begin(name);
        }

        Transaction::Transaction(MapDocument* document, const std::string& name) :
        m_document(document),
        m_cancelled(false) {
            begin(name);
        }

        Transaction::~Transaction() {
            if (!m_cancelled)
                commit();
        }

        void Transaction::rollback() {
            m_document->rollbackTransaction();
        }

        void Transaction::cancel() {
            m_document->cancelTransaction();
            m_cancelled = true;
        }

        void Transaction::begin(const std::string& name) {
            m_document->startTransaction(name);
        }

        void Transaction::commit() {
            m_document->commitTransaction();
        }
    }
}
