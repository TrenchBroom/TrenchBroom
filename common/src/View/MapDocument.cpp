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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Polyhedron.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SystemPaths.h"
#include "Model/AttributeNameWithDoubleQuotationMarksIssueGenerator.h"
#include "Model/AttributeValueWithDoubleQuotationMarksIssueGenerator.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/CollectAttributableNodesVisitor.h"
#include "Model/CollectContainedNodesVisitor.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/CollectNodesVisitor.h"
#include "Model/CollectNodesByVisibilityVisitor.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/CollectSelectableNodesWithFilePositionVisitor.h"
#include "Model/CollectSelectedNodesVisitor.h"
#include "Model/CollectTouchingNodesVisitor.h"
#include "Model/CollectUniqueNodesVisitor.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/EditorContext.h"
#include "Model/EmptyAttributeNameIssueGenerator.h"
#include "Model/EmptyAttributeValueIssueGenerator.h"
#include "Model/EmptyBrushEntityIssueGenerator.h"
#include "Model/EmptyGroupIssueGenerator.h"
#include "Model/Entity.h"
#include "Model/LinkSourceIssueGenerator.h"
#include "Model/LinkTargetIssueGenerator.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "Model/Group.h"
#include "Model/LongAttributeNameIssueGenerator.h"
#include "Model/LongAttributeValueIssueGenerator.h"
#include "Model/MergeNodesIntoWorldVisitor.h"
#include "Model/MissingClassnameIssueGenerator.h"
#include "Model/MissingDefinitionIssueGenerator.h"
#include "Model/MixedBrushContentsIssueGenerator.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/NonIntegerPlanePointsIssueGenerator.h"
#include "Model/NonIntegerVerticesIssueGenerator.h"
#include "Model/WorldBoundsIssueGenerator.h"
#include "Model/PointEntityWithBrushesIssueGenerator.h"
#include "Model/PointFile.h"
#include "Model/World.h"
#include "View/AddRemoveNodesCommand.h"
#include "View/ChangeBrushFaceAttributesCommand.h"
#include "View/ChangeEntityAttributesCommand.h"
#include "View/UpdateEntitySpawnflagCommand.h"
#include "View/ConvertEntityColorCommand.h"
#include "View/CurrentGroupCommand.h"
#include "View/DuplicateNodesCommand.h"
#include "View/EntityDefinitionFileCommand.h"
#include "View/FindPlanePointsCommand.h"
#include "View/Grid.h"
#include "View/MapViewConfig.h"
#include "View/MoveBrushEdgesCommand.h"
#include "View/MoveBrushFacesCommand.h"
#include "View/MoveBrushVerticesCommand.h"
#include "View/MoveTexturesCommand.h"
#include "View/RemoveBrushEdgesCommand.h"
#include "View/RemoveBrushFacesCommand.h"
#include "View/RemoveBrushVerticesCommand.h"
#include "View/RenameGroupsCommand.h"
#include "View/ReparentNodesCommand.h"
#include "View/ResizeBrushesCommand.h"
#include "View/CopyTexCoordSystemFromFaceCommand.h"
#include "View/RotateTexturesCommand.h"
#include "View/SelectionCommand.h"
#include "View/SetLockStateCommand.h"
#include "View/SetModsCommand.h"
#include "View/SetVisibilityCommand.h"
#include "View/ShearTexturesCommand.h"
#include "View/SnapBrushVerticesCommand.h"
#include "View/SplitBrushEdgesCommand.h"
#include "View/SplitBrushFacesCommand.h"
#include "View/SetTextureCollectionsCommand.h"
#include "View/TransformObjectsCommand.h"
#include "View/VertexHandleManager.h"
#include "View/ViewEffectsService.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const BBox3 MapDocument::DefaultWorldBounds(-16384.0, 16384.0);
        const String MapDocument::DefaultDocumentName("unnamed.map");
        
        MapDocument::MapDocument() :
        m_worldBounds(DefaultWorldBounds),
        m_world(NULL),
        m_currentLayer(NULL),
        m_pointFile(NULL),
        m_editorContext(new Model::EditorContext()),
        m_entityDefinitionManager(new Assets::EntityDefinitionManager()),
        m_entityModelManager(new Assets::EntityModelManager(this, pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter))),
        m_textureManager(new Assets::TextureManager(this, pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter))),
        m_mapViewConfig(new MapViewConfig(*m_editorContext)),
        m_grid(new Grid(4)),
        m_path(DefaultDocumentName),
        m_lastSaveModificationCount(0),
        m_modificationCount(0),
        m_currentTextureName(Model::BrushFace::NoTextureName),
        m_lastSelectionBounds(0.0, 32.0),
        m_selectionBoundsValid(true),
        m_viewEffectsService(NULL) {
            bindObservers();
        }
        
        MapDocument::~MapDocument() {
            unbindObservers();
            
            if (isPointFileLoaded())
                unloadPointFile();
            clearWorld();
            
            delete m_grid;
            delete m_mapViewConfig;
            delete m_textureManager;
            delete m_entityModelManager;
            delete m_entityDefinitionManager;
            delete m_editorContext;
        }
        
        Model::GamePtr MapDocument::game() const {
            return m_game;
        }
        
        const BBox3& MapDocument::worldBounds() const {
            return m_worldBounds;
        }
        
        Model::World* MapDocument::world() const {
            return m_world;
        }
        
        bool MapDocument::isGamePathPreference(const IO::Path& path) const {
            return m_game.get() != NULL && m_game->isGamePathPreference(path);
        }
        
        Model::Layer* MapDocument::currentLayer() const {
            ensure(m_currentLayer != NULL, "currentLayer is null");
            return m_currentLayer;
        }
        
        void MapDocument::setCurrentLayer(Model::Layer* currentLayer) {
            ensure(currentLayer != NULL, "currentLayer is null");
            assert(!currentLayer->locked());
            assert(!currentLayer->hidden());
            m_currentLayer = currentLayer;
            currentLayerDidChangeNotifier(m_currentLayer);
        }
        
        Model::Group* MapDocument::currentGroup() const {
            return m_editorContext->currentGroup();
        }
        
        Model::Node* MapDocument::currentParent() const {
            Model::Node* result = currentGroup();
            if (result == NULL)
                result = currentLayer();
            return result;
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
        
        View::MapViewConfig& MapDocument::mapViewConfig() const {
            return *m_mapViewConfig;
        }
        
        Grid& MapDocument::grid() const {
            return *m_grid;
        }
        
        Model::PointFile* MapDocument::pointFile() const {
            return m_pointFile;
        }
        
        void MapDocument::setViewEffectsService(ViewEffectsService* viewEffectsService) {
            m_viewEffectsService = viewEffectsService;
        }
        
        void MapDocument::newDocument(const Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GamePtr game) {
            info("Creating new document");
            
            clearDocument();
            createWorld(mapFormat, worldBounds, game);
            
            loadAssets();
            registerIssueGenerators();
            
            initializeWorld(worldBounds);
            clearModificationCount();
            
            documentWasNewedNotifier(this);
        }
        
        void MapDocument::loadDocument(const Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path) {
            info("Loading document from " + path.asString());
            
            clearDocument();
            loadWorld(mapFormat, worldBounds, game, path);
            
            loadAssets();
            registerIssueGenerators();
            
            documentWasLoadedNotifier(this);
        }
        
        void MapDocument::saveDocument() {
            doSaveDocument(m_path);
        }
        
        void MapDocument::saveDocumentAs(const IO::Path& path) {
            doSaveDocument(path);
        }
        
        void MapDocument::saveDocumentTo(const IO::Path& path) {
            ensure(m_game.get() != NULL, "game is null");
            ensure(m_world != NULL, "world is null");
            m_game->writeMap(m_world, path);
        }
        
        void MapDocument::exportDocumentAs(const Model::ExportFormat format, const IO::Path& path) {
            m_game->exportMap(m_world, format, path);
        }

        void MapDocument::doSaveDocument(const IO::Path& path) {
            saveDocumentTo(path);
            setLastSaveModificationCount();
            setPath(path);
            documentWasSavedNotifier(this);
        }
        
        void MapDocument::clearDocument() {
            if (m_world != NULL) {
                documentWillBeClearedNotifier(this);
                
                clearSelection();
                unloadAssets();
                clearWorld();
                clearModificationCount();
                
                documentWasClearedNotifier(this);
            }
        }
        
        String MapDocument::serializeSelectedNodes() {
            StringStream stream;
            m_game->writeNodesToStream(m_world, m_selectedNodes.nodes(), stream);
            return stream.str();
        }
        
        String MapDocument::serializeSelectedBrushFaces() {
            StringStream stream;
            m_game->writeBrushFacesToStream(m_world, m_selectedBrushFaces, stream);
            return stream.str();
        }
        
        PasteType MapDocument::paste(const String& str) {
            try {
                const Model::NodeList nodes = m_game->parseNodes(str, m_world, m_worldBounds, this);
                if (!nodes.empty() && pasteNodes(nodes))
                    return PT_Node;
            } catch (const ParserException& e) {
                try {
                    const Model::BrushFaceList faces = m_game->parseBrushFaces(str, m_world, m_worldBounds, this);
                    if (!faces.empty() && pasteBrushFaces(faces))
                        return PT_BrushFace;
                } catch (const ParserException&) {
                    error("Could not parse clipboard contents: %s", e.what());
                }
            }
            return PT_Failed;
        }
        
        bool MapDocument::pasteNodes(const Model::NodeList& nodes) {
            Model::MergeNodesIntoWorldVisitor mergeNodes(m_world, currentParent());
            Model::Node::accept(std::begin(nodes), std::end(nodes), mergeNodes);
            
            const Model::NodeList addedNodes = addNodes(mergeNodes.result());
            if (addedNodes.empty())
                return false;
            
            deselectAll();
            
            Model::CollectSelectableNodesVisitor collectSelectables(editorContext());
            Model::Node::acceptAndRecurse(std::begin(addedNodes), std::end(addedNodes), collectSelectables);
            select(collectSelectables.nodes());
            
            return true;
        }
        
        bool MapDocument::pasteBrushFaces(const Model::BrushFaceList& faces) {
            assert(!faces.empty());
            const Model::BrushFace* face = faces.back();
            
            const bool result = setFaceAttributes(face->attribs());
            VectorUtils::deleteAll(faces);
            
            return result;
        }
        
        void MapDocument::loadPointFile(const IO::Path& path) {
            if (isPointFileLoaded())
                unloadPointFile();
            m_pointFile = new Model::PointFile(path);
            info("Loaded point file " + path.asString());
            pointFileWasLoadedNotifier();
        }
        
        bool MapDocument::isPointFileLoaded() const {
            return m_pointFile != NULL;
        }
        
        void MapDocument::unloadPointFile() {
            assert(isPointFileLoaded());
            delete m_pointFile;
            m_pointFile = NULL;
            
            info("Unloaded point file");
            pointFileWasUnloadedNotifier();
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
        
        const Model::AttributableNodeList MapDocument::allSelectedAttributableNodes() const {
            if (!hasSelection())
                return Model::AttributableNodeList(1, m_world);
            
            Model::CollectAttributableNodesVisitor visitor;
            Model::Node::accept(std::begin(m_selectedNodes), std::end(m_selectedNodes), visitor);
            return visitor.nodes();
        }
        
        const Model::NodeCollection& MapDocument::selectedNodes() const {
            return m_selectedNodes;
        }
        
        const Model::BrushFaceList MapDocument::allSelectedBrushFaces() const {
            if (hasSelectedBrushFaces())
                return selectedBrushFaces();
            Model::CollectBrushFacesVisitor visitor;
            Model::Node::acceptAndRecurse(std::begin(m_selectedNodes), std::end(m_selectedNodes), visitor);
            return visitor.faces();
        }
        
        const Model::BrushFaceList& MapDocument::selectedBrushFaces() const {
            return m_selectedBrushFaces;
        }
        
        const BBox3& MapDocument::referenceBounds() const {
            if (hasSelectedNodes())
                return selectionBounds();
            return lastSelectionBounds();
        }
        
        const BBox3& MapDocument::lastSelectionBounds() const {
            return m_lastSelectionBounds;
        }
        
        const BBox3& MapDocument::selectionBounds() const {
            if (!m_selectionBoundsValid)
                validateSelectionBounds();
            return m_selectionBounds;
        }
        
        const String& MapDocument::currentTextureName() const {
            return m_currentTextureName;
        }
        
        void MapDocument::setCurrentTextureName(const String& currentTextureName) {
            if (m_currentTextureName == currentTextureName)
                return;
            m_currentTextureName = currentTextureName;
            currentTextureNameDidChangeNotifier(m_currentTextureName);
        }

        void MapDocument::selectAllNodes() {
            submitAndStore(SelectionCommand::selectAllNodes());
        }
        
        void MapDocument::selectSiblings() {
            const Model::NodeList& nodes = selectedNodes().nodes();
            if (nodes.empty())
                return;
            
            Model::CollectSelectableUniqueNodesVisitor visitor(*m_editorContext);
            for (Model::Node* node : nodes) {
                Model::Node* parent = node->parent();
                parent->iterate(visitor);
            }
            
            Transaction transaction(this, "Select Siblings");
            deselectAll();
            select(visitor.nodes());
        }
        
        void MapDocument::selectTouching(const bool del) {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            
            Model::CollectTouchingNodesVisitor<Model::BrushList::const_iterator> visitor(std::begin(brushes), std::end(brushes), editorContext());
            m_world->acceptAndRecurse(visitor);
            
            const Model::NodeList nodes = visitor.nodes();
            
            Transaction transaction(this, "Select Touching");
            if (del)
                deleteObjects();
            else
                deselectAll();
            select(nodes);
        }
        
        void MapDocument::selectInside(const bool del) {
            const Model::BrushList& brushes = m_selectedNodes.brushes();

            Model::CollectContainedNodesVisitor<Model::BrushList::const_iterator> visitor(std::begin(brushes), std::end(brushes), editorContext());
            m_world->acceptAndRecurse(visitor);
            
            const Model::NodeList nodes = visitor.nodes();

            Transaction transaction(this, "Select Inside");
            if (del)
                deleteObjects();
            else
                deselectAll();
            select(nodes);
        }
        
        void MapDocument::selectNodesWithFilePosition(const std::vector<size_t>& positions) {
            Model::CollectSelectableNodesWithFilePositionVisitor visitor(*m_editorContext, positions);
            m_world->acceptAndRecurse(visitor);
            
            Transaction transaction(this, "Select by Line Number");
            deselectAll();
            select(visitor.nodes());
        }
        
        void MapDocument::select(const Model::NodeList& nodes) {
            submitAndStore(SelectionCommand::select(nodes));
        }
        
        void MapDocument::select(Model::Node* node) {
            submitAndStore(SelectionCommand::select(Model::NodeList(1, node)));
        }
        
        void MapDocument::select(const Model::BrushFaceList& faces) {
            submitAndStore(SelectionCommand::select(faces));
        }
        
        void MapDocument::select(Model::BrushFace* face) {
            submitAndStore(SelectionCommand::select(Model::BrushFaceList(1, face)));
            setCurrentTextureName(face->textureName());
        }
        
        void MapDocument::convertToFaceSelection() {
            submitAndStore(SelectionCommand::convertToFaces());
        }
        
        void MapDocument::deselectAll() {
            if (hasSelection())
                submitAndStore(SelectionCommand::deselectAll());
        }
        
        void MapDocument::deselect(Model::Node* node) {
            deselect(Model::NodeList(1, node));
        }
        
        void MapDocument::deselect(const Model::NodeList& nodes) {
            submitAndStore(SelectionCommand::deselect(nodes));
        }
        
        void MapDocument::deselect(Model::BrushFace* face) {
            submitAndStore(SelectionCommand::deselect(Model::BrushFaceList(1, face)));
        }
        
        void MapDocument::updateLastSelectionBounds() {
            m_lastSelectionBounds = selectionBounds();
        }
        
        void MapDocument::invalidateSelectionBounds() {
            m_selectionBoundsValid = false;
        }
        
        void MapDocument::validateSelectionBounds() const {
            Model::ComputeNodeBoundsVisitor visitor;
            Model::Node::accept(std::begin(m_selectedNodes), std::end(m_selectedNodes), visitor);
            m_selectionBounds = visitor.bounds();
            m_selectionBoundsValid = true;
        }
        
        void MapDocument::clearSelection() {
            m_selectedNodes.clear();
            m_selectedBrushFaces.clear();
        }
        
        void MapDocument::addNode(Model::Node* node, Model::Node* parent) {
            ensure(node != NULL, "node is null");
            assert(node->parent() == NULL);
            ensure(parent != NULL, "parent is null");
            assert(parent != node);
            
            Model::ParentChildrenMap map;
            map[parent].push_back(node);
            addNodes(map);
        }
        
        void MapDocument::removeNode(Model::Node* node) {
            removeNodes(Model::NodeList(1, node));
        }
        
        Model::NodeList MapDocument::addNodes(const Model::ParentChildrenMap& nodes) {
            Transaction transaction(this, "Add Objects");
            AddRemoveNodesCommand::Ptr command = AddRemoveNodesCommand::add(nodes);
            if (!submitAndStore(command))
                return Model::EmptyNodeList;
            
            const Model::NodeList addedNodes = collectChildren(nodes);
            ensureVisible(collectChildren(nodes));
            return addedNodes;
        }
        
        Model::NodeList MapDocument::addNodes(const Model::NodeList& nodes, Model::Node* parent) {
            AddRemoveNodesCommand::Ptr command = AddRemoveNodesCommand::add(parent, nodes);
            if (!submitAndStore(command))
                return Model::EmptyNodeList;

            ensureVisible(nodes);
            return nodes;
        }

        void MapDocument::removeNodes(const Model::NodeList& nodes) {
            Model::ParentChildrenMap removableNodes = parentChildrenMap(removeImplicitelyRemovedNodes(nodes));
            
            Transaction transaction(this);
            while (!removableNodes.empty()) {
                closeRemovedGroups(removableNodes);
                submitAndStore(AddRemoveNodesCommand::remove(removableNodes));
                
                removableNodes = collectRemovableParents(removableNodes);
            }
        }
        
        Model::ParentChildrenMap MapDocument::collectRemovableParents(const Model::ParentChildrenMap& nodes) const {
            Model::ParentChildrenMap result;
            for (const auto& entry : nodes) {
                Model::Node* node = entry.first;
                if (node->removeIfEmpty() && !node->hasChildren()) {
                    Model::Node* parent = node->parent();
                    ensure(parent != NULL, "parent is null");
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
        
        Model::NodeList MapDocument::removeImplicitelyRemovedNodes(Model::NodeList nodes) const {
            if (nodes.empty())
                return nodes;
            
            VectorUtils::sort(nodes, CompareByAncestry());
            
            Model::NodeList result;
            result.reserve(nodes.size());
            result.push_back(nodes.front());
            
            for (size_t i = 1; i < nodes.size(); ++i) {
                Model::Node* node = nodes[i];
                if (!node->isDescendantOf(result))
                    result.push_back(node);
            }
            
            return result;
        }
        
        void MapDocument::closeRemovedGroups(const Model::ParentChildrenMap& toRemove) {
            Model::ParentChildrenMap::const_iterator mIt, mEnd;
            for (const auto& entry : toRemove) {
                const Model::NodeList& nodes = entry.second;
                for (const Model::Node* node : nodes) {
                    if (node == currentGroup()) {
                        closeGroup();
                        closeRemovedGroups(toRemove);
                        return;
                    }
                }
            }
        }

        bool MapDocument::reparentNodes(Model::Node* newParent, const Model::NodeList& children) {
            Model::ParentChildrenMap nodes;
            nodes.insert(std::make_pair(newParent, children));
            return reparentNodes(nodes);
        }
        
        bool MapDocument::reparentNodes(const Model::ParentChildrenMap& nodesToAdd) {
            if (!checkReparenting(nodesToAdd))
                return false;
            
            Model::ParentChildrenMap nodesToRemove;
            for (const auto& entry : nodesToAdd) {
                const Model::NodeList& children = entry.second;
                MapUtils::merge(nodesToRemove, Model::parentChildrenMap(children));
            }

            Transaction transaction(this, "Reparent Objects");
            submitAndStore(ReparentNodesCommand::reparent(nodesToAdd, nodesToRemove));

            Model::ParentChildrenMap removableNodes = collectRemovableParents(nodesToRemove);
            while (!removableNodes.empty()) {
                closeRemovedGroups(removableNodes);
                submitAndStore(AddRemoveNodesCommand::remove(removableNodes));
                
                removableNodes = collectRemovableParents(removableNodes);
            }
            
            return true;
        }
        
        bool MapDocument::checkReparenting(const Model::ParentChildrenMap& nodesToAdd) const {
            for (const auto& entry : nodesToAdd) {
                const Model::Node* newParent = entry.first;
                const Model::NodeList& children = entry.second;
                if (!newParent->canAddChildren(std::begin(children), std::end(children)))
                    return false;
            }
            return true;
        }

        bool MapDocument::deleteObjects() {
            Transaction transaction(this, "Delete Objects");
            const Model::NodeList nodes = m_selectedNodes.nodes();
            deselectAll();
            removeNodes(nodes);
            return true;
        }
        
        bool MapDocument::duplicateObjects() {
            if (submitAndStore(DuplicateNodesCommand::duplicate())) {
                m_viewEffectsService->flashSelection();
                return true;
            }
            return false;
        }
        
        Model::Group* MapDocument::groupSelection(const String& name) {
            if (!hasSelectedNodes())
                return NULL;
            
            const Model::NodeList nodes = collectGroupableNodes(selectedNodes().nodes());
            if (nodes.empty())
                return NULL;

            Model::Group* group = new Model::Group(name);
            
            const Transaction transaction(this, "Group Selected Objects");
            deselectAll();
            addNode(group, currentParent());
            reparentNodes(group, nodes);
            select(group);
            
            return group;
        }

        class MapDocument::MatchGroupableNodes {
        private:
            const Model::World* m_world;
        public:
            MatchGroupableNodes(const Model::World* world) : m_world(world) {}
        public:
            bool operator()(const Model::World* world) const   { return false; }
            bool operator()(const Model::Layer* layer) const   { return false; }
            bool operator()(const Model::Group* group) const   { return true;  }
            bool operator()(const Model::Entity* entity) const { return true; }
            bool operator()(const Model::Brush* brush) const   { return brush->entity() == m_world; }
        };
        
        Model::NodeList MapDocument::collectGroupableNodes(const Model::NodeList& selectedNodes) const {
            typedef Model::CollectMatchingNodesVisitor<MatchGroupableNodes, Model::UniqueNodeCollectionStrategy, Model::StopRecursionIfMatched> CollectGroupableNodesVisitor;
            
            CollectGroupableNodesVisitor collect(world());
            Model::Node::acceptAndEscalate(std::begin(selectedNodes), std::end(selectedNodes), collect);
            return collect.nodes();
        }

        void MapDocument::ungroupSelection() {
            if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
                return;
            
            const Model::NodeList groups = m_selectedNodes.nodes();
            Model::NodeList allChildren;
            
            const Transaction transaction(this, "Ungroup");
            deselectAll();
            
            for (Model::Node* group : groups) {
                Model::Layer* layer = Model::findLayer(group);
                const Model::NodeList& children = group->children();
                reparentNodes(layer, children);
                VectorUtils::append(allChildren, children);
            }
            
            select(allChildren);
        }
        
        void MapDocument::renameGroups(const String& name) {
            submitAndStore(RenameGroupsCommand::rename(name));
        }
        
        void MapDocument::openGroup(Model::Group* group) {
            const Transaction transaction(this, "Open Group");
            
            deselectAll();
            Model::Group* previousGroup = m_editorContext->currentGroup();
            if (previousGroup == NULL)
                lock(Model::NodeList(1, m_world));
            else
                resetLock(Model::NodeList(1, previousGroup));
            unlock(Model::NodeList(1, group));
            submitAndStore(CurrentGroupCommand::push(group));
        }
        
        void MapDocument::closeGroup() {
            const Transaction transaction(this, "Close Group");
            
            deselectAll();
            Model::Group* previousGroup = m_editorContext->currentGroup();
            resetLock(Model::NodeList(1, previousGroup));
            submitAndStore(CurrentGroupCommand::pop());

            Model::Group* currentGroup = m_editorContext->currentGroup();
            if (currentGroup != NULL)
                unlock(Model::NodeList(1, currentGroup));
            else
                unlock(Model::NodeList(1, m_world));
        }
        
        void MapDocument::isolate(const Model::NodeList& nodes) {
            const Model::LayerList& layers = m_world->allLayers();

            Model::CollectTransitivelyUnselectedNodesVisitor collectUnselected;
            Model::Node::recurse(std::begin(layers), std::end(layers), collectUnselected);
            
            Model::CollectTransitivelySelectedNodesVisitor collectSelected;
            Model::Node::recurse(std::begin(layers), std::end(layers), collectSelected);

            Transaction transaction(this, "Isolate Objects");
            submitAndStore(SetVisibilityCommand::hide(collectUnselected.nodes()));
            submitAndStore(SetVisibilityCommand::show(collectSelected.nodes()));
        }
        
        void MapDocument::hide(const Model::NodeList nodes) {
            Model::CollectSelectedNodesVisitor collect;
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), collect);
            
            const Transaction transaction(this, "Hide Objects");
            deselect(collect.nodes());
            submitAndStore(SetVisibilityCommand::hide(nodes));
        }
        
        void MapDocument::hideSelection() {
            hide(m_selectedNodes.nodes());
        }
        
        void MapDocument::show(const Model::NodeList& nodes) {
            submitAndStore(SetVisibilityCommand::show(nodes));
        }
        
        void MapDocument::showAll() {
            const Model::LayerList& layers = m_world->allLayers();
            Model::CollectNodesVisitor collect;
            Model::Node::recurse(std::begin(layers), std::end(layers), collect);
            resetVisibility(collect.nodes());
        }
        
        void MapDocument::ensureVisible(const Model::NodeList& nodes) {
            submitAndStore(SetVisibilityCommand::ensureVisible(nodes));
        }

        void MapDocument::resetVisibility(const Model::NodeList& nodes) {
            submitAndStore(SetVisibilityCommand::reset(nodes));
        }
        
        void MapDocument::lock(const Model::NodeList& nodes) {
            Model::CollectSelectedNodesVisitor collect;
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), collect);
            
            const Transaction transaction(this, "Lock Objects");
            submitAndStore(SetLockStateCommand::lock(nodes));
            deselect(collect.nodes());
        }
        
        void MapDocument::unlock(const Model::NodeList& nodes) {
            submitAndStore(SetLockStateCommand::unlock(nodes));
        }
        
        void MapDocument::resetLock(const Model::NodeList& nodes) {
            submitAndStore(SetLockStateCommand::reset(nodes));
        }
        
        bool MapDocument::translateObjects(const Vec3& delta) {
            return submitAndStore(TransformObjectsCommand::translate(delta, pref(Preferences::TextureLock)));
        }
        
        bool MapDocument::rotateObjects(const Vec3& center, const Vec3& axis, const FloatType angle) {
            return submitAndStore(TransformObjectsCommand::rotate(center, axis, angle, pref(Preferences::TextureLock)));
        }
        
        bool MapDocument::flipObjects(const Vec3& center, const Math::Axis::Type axis) {
            return submitAndStore(TransformObjectsCommand::flip(center, axis, pref(Preferences::TextureLock)));
        }
        
        bool MapDocument::createBrush(const Vec3::List& points) {
            Model::BrushBuilder builder(m_world, m_worldBounds);
            Model::Brush* brush = builder.createBrush(points, currentTextureName());
            if (!brush->fullySpecified()) {
                delete brush;
                return false;
            }
            
            Transaction transaction(this, "Create Brush");
            deselectAll();
            addNode(brush, currentParent());
            select(brush);
            return true;
        }

        bool MapDocument::csgConvexMerge() {
            if (!hasSelectedBrushFaces() && !selectedNodes().hasOnlyBrushes())
                return false;
            
            Polyhedron3 polyhedron;
            
            if (hasSelectedBrushFaces()) {
                for (const Model::BrushFace* face : selectedBrushFaces()) {
                    for (const Model::BrushVertex* vertex : face->vertices())
                        polyhedron.addPoint(vertex->position());
                }
            } else if (selectedNodes().hasOnlyBrushes()) {
                for (const Model::Brush* brush : selectedNodes().brushes()) {
                    for (const Model::BrushVertex* vertex : brush->vertices())
                        polyhedron.addPoint(vertex->position());
                }
            }
            
            if (!polyhedron.polyhedron() || !polyhedron.closed())
                return false;
            
            const Model::BrushBuilder builder(m_world, m_worldBounds);
            Model::Brush* brush = builder.createBrush(polyhedron, currentTextureName());
            brush->cloneFaceAttributesFrom(selectedNodes().brushes());
            
            // The nodelist is either empty or contains only brushes.
            const Model::NodeList toRemove = selectedNodes().nodes();
            
            // We could be merging brushes that have different parents; use the parent of the first brush.
            Model::Node* parent;
            if (!selectedNodes().brushes().empty()) {
                parent = selectedNodes().brushes().front()->parent();
            } else {
                parent = currentParent();
            }
            
            const Transaction transaction(this, "CSG Convex Merge");
            deselectAll();
            addNode(brush, parent);
            removeNodes(toRemove);
            select(brush);
            return true;
        }
        
        bool MapDocument::csgSubtract() {
            const Model::BrushList brushes = selectedNodes().brushes();
            if (brushes.size() < 2)
                return false;
            
            const Model::BrushList minuends(std::begin(brushes), std::end(brushes) - 1);
            Model::Brush* subtrahend = brushes.back();
            
            Model::ParentChildrenMap toAdd;
            Model::NodeList toRemove;
            toRemove.push_back(subtrahend);
            
            for (Model::Brush* minuend : minuends) {
                const Model::BrushList result = minuend->subtract(*m_world, m_worldBounds, currentTextureName(), subtrahend);
                if (!result.empty()) {
                    VectorUtils::append(toAdd[minuend->parent()], result);
                    toRemove.push_back(minuend);
                }
            }
            
            Transaction transaction(this, "CSG Subtract");
            deselectAll();
            const Model::NodeList added = addNodes(toAdd);
            removeNodes(toRemove);
            select(added);
            
            return true;
        }

        bool MapDocument::csgIntersect() {
            const Model::BrushList brushes = selectedNodes().brushes();
            if (brushes.size() < 2)
                return false;
            
            Model::Brush* result = brushes.front()->clone(m_worldBounds);

            bool valid = true;
            Model::BrushList::const_iterator it, end;
            for (it = std::begin(brushes), end = std::end(brushes); it != end && valid; ++it) {
                Model::Brush* brush = *it;
                try {
                    result->intersect(m_worldBounds, brush);
                } catch (const GeometryException&) {
                    valid = false;
                }
            }
            
            const Model::NodeList toRemove(std::begin(brushes), std::end(brushes));
            
            Transaction transaction(this, "CSG Intersect");
            deselect(toRemove);
            
            if (valid) {
                addNode(result, currentParent());
                removeNodes(toRemove);
                select(result);
            } else {
                removeNodes(toRemove);
                delete result;
            }
            
            return true;
        }

        bool MapDocument::clipBrushes(const Vec3& p1, const Vec3& p2, const Vec3& p3) {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            Model::ParentChildrenMap clippedBrushes;
            
            for (const Model::Brush* originalBrush : brushes) {
                Model::BrushFace* clipFace = m_world->createFace(p1, p2, p3, Model::BrushFaceAttributes(currentTextureName()));
                Model::Brush* clippedBrush = originalBrush->clone(m_worldBounds);
                if (clippedBrush->clip(m_worldBounds, clipFace))
                    clippedBrushes[originalBrush->parent()].push_back(clippedBrush);
                else
                    delete clippedBrush;
            }
            
            Transaction transaction(this, "Clip Brushes");
            const Model::NodeList toRemove(std::begin(brushes), std::end(brushes));
            deselectAll();
            removeNodes(toRemove);
            select(addNodes(clippedBrushes));
            
            return true;
        }

        bool MapDocument::setAttribute(const Model::AttributeName& name, const Model::AttributeValue& value) {
            return submitAndStore(ChangeEntityAttributesCommand::set(name, value));
        }
        
        bool MapDocument::renameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName) {
            return submitAndStore(ChangeEntityAttributesCommand::rename(oldName, newName));
        }
        
        bool MapDocument::removeAttribute(const Model::AttributeName& name) {
            return submitAndStore(ChangeEntityAttributesCommand::remove(name));
        }
        
        bool MapDocument::convertEntityColorRange(const Model::AttributeName& name, Assets::ColorRange::Type range) {
            return submitAndStore(ConvertEntityColorCommand::convert(name, range));
        }
        
        bool MapDocument::updateSpawnflag(const Model::AttributeName& name, const size_t flagIndex, const bool setFlag) {
            return submitAndStore(UpdateEntitySpawnflagCommand::update(name, flagIndex, setFlag));
        }
        
        bool MapDocument::resizeBrushes(const Polygon3::List& faces, const Vec3& delta) {
            return submitAndStore(ResizeBrushesCommand::resize(faces, delta));
        }
        
        void MapDocument::setTexture(Assets::Texture* texture) {
            const Model::BrushFaceList faces = allSelectedBrushFaces();
            
            if (texture != nullptr) {
                if (faces.empty()) {
                    if (currentTextureName() == texture->name())
                        setCurrentTextureName(Model::BrushFace::NoTextureName);
                    else
                        setCurrentTextureName(texture->name());
                } else {
                    if (hasTexture(faces, texture)) {
                        texture = nullptr;
                        setCurrentTextureName(Model::BrushFace::NoTextureName);
                    } else {
                        setCurrentTextureName(texture->name());
                    }
                }
            }
            
            if (!faces.empty()) {
                Model::ChangeBrushFaceAttributesRequest request;
                if (texture == nullptr)
                    request.unsetTexture();
                else
                    request.setTexture(texture);
                submitAndStore(ChangeBrushFaceAttributesCommand::command(request));
            }
        }
        
        bool MapDocument::hasTexture(const Model::BrushFaceList& faces, Assets::Texture* texture) const {
            for (const Model::BrushFace* face : faces) {
                if (face->texture() != texture)
                    return false;
            }
            
            return true;
        }

        bool MapDocument::setFaceAttributes(const Model::BrushFaceAttributes& attributes) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setAll(attributes);
            
            // try to find the texture if it is null, maybe it just wasn't set?
            if (attributes.texture() == NULL) {
                Assets::Texture* texture = m_textureManager->texture(attributes.textureName());
                request.setTexture(texture);
            }
            
            return setFaceAttributes(request);
        }
        
        bool MapDocument::setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) {
            return submitAndStore(ChangeBrushFaceAttributesCommand::command(request));
        }
        
        bool MapDocument::copyTexCoordSystemFromFace(const Model::TexCoordSystemSnapshot* coordSystemSnapshot, const Vec3f& sourceFaceNormal) {
            return submitAndStore(CopyTexCoordSystemFromFaceCommand::command(coordSystemSnapshot, sourceFaceNormal));
        }
        
        bool MapDocument::moveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) {
            return submitAndStore(MoveTexturesCommand::move(cameraUp, cameraRight, delta));
        }
        
        bool MapDocument::rotateTextures(const float angle) {
            return submitAndStore(RotateTexturesCommand::rotate(angle));
        }
        
        bool MapDocument::shearTextures(const Vec2f& factors) {
            return submitAndStore(ShearTexturesCommand::shear(factors));
        }
        
        void MapDocument::rebuildBrushGeometry(const Model::BrushList& brushes) {
            performRebuildBrushGeometry(brushes);
        }
        
        bool MapDocument::snapVertices(const size_t snapTo) {
            assert(m_selectedNodes.hasOnlyBrushes());
            return submitAndStore(SnapBrushVerticesCommand::snap(snapTo));
        }
        
        bool MapDocument::findPlanePoints() {
            return submitAndStore(FindPlanePointsCommand::findPlanePoints());
        }
        
        MapDocument::MoveVerticesResult MapDocument::moveVertices(const Model::VertexToBrushesMap& vertices, const Vec3& delta) {
            MoveBrushVerticesCommand::Ptr command = MoveBrushVerticesCommand::move(vertices, delta);
            const bool success = submitAndStore(command);
            const bool hasRemainingVertices = command->hasRemainingVertices();
            return MoveVerticesResult(success, hasRemainingVertices);
        }
        
        bool MapDocument::moveEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta) {
            return submitAndStore(MoveBrushEdgesCommand::move(edges, delta));
        }
        
        bool MapDocument::moveFaces(const Model::VertexToFacesMap& faces, const Vec3& delta) {
            return submitAndStore(MoveBrushFacesCommand::move(faces, delta));
        }
        
        bool MapDocument::splitEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta) {
            return submitAndStore(SplitBrushEdgesCommand::split(edges, delta));
        }
        
        bool MapDocument::splitFaces(const Model::VertexToFacesMap& faces, const Vec3& delta) {
            return submitAndStore(SplitBrushFacesCommand::split(faces, delta));
        }
        
        bool MapDocument::removeVertices(const Model::VertexToBrushesMap& vertices) {
            return submitAndStore(RemoveBrushVerticesCommand::remove(vertices));
        }
        
        bool MapDocument::removeEdges(const Model::VertexToEdgesMap& edges) {
            return submitAndStore(RemoveBrushEdgesCommand::remove(edges));
        }
        
        bool MapDocument::removeFaces(const Model::VertexToFacesMap& faces) {
            return submitAndStore(RemoveBrushFacesCommand::remove(faces));
        }

        void MapDocument::printVertices() {
            if (hasSelectedBrushFaces()) {
                for (const Model::BrushFace* face : m_selectedBrushFaces) {
                    StringStream str;
                    for (const Model::BrushVertex* vertex : face->vertices())
                        str << "(" << vertex->position().asString() << ") ";
                    info(str.str());
                }
            } else if (selectedNodes().hasBrushes()) {
                for (const Model::Brush* brush : selectedNodes().brushes()) {
                    StringStream str;
                    for (const Model::BrushVertex* vertex : brush->vertices())
                        str << "(" << vertex->position().asString() << ") ";
                    info(str.str());
                }
            }
        }

        bool MapDocument::canUndoLastCommand() const {
            return doCanUndoLastCommand();
        }
        
        bool MapDocument::canRedoNextCommand() const {
            return doCanRedoNextCommand();
        }
        
        const String& MapDocument::lastCommandName() const {
            return doGetLastCommandName();
        }
        
        const String& MapDocument::nextCommandName() const {
            return doGetNextCommandName();
        }
        
        void MapDocument::undoLastCommand() {
            doUndoLastCommand();
        }
        
        void MapDocument::redoNextCommand() {
            doRedoNextCommand();
        }
        
        bool MapDocument::repeatLastCommands() {
            return doRepeatLastCommands();
        }
        
        void MapDocument::clearRepeatableCommands() {
            doClearRepeatableCommands();
        }
        
        void MapDocument::beginTransaction(const String& name) {
            doBeginTransaction(name);
        }
        
        void MapDocument::rollbackTransaction() {
            doRollbackTransaction();
        }
        
        void MapDocument::commitTransaction() {
            doEndTransaction();
        }
        
        void MapDocument::cancelTransaction() {
            doRollbackTransaction();
            doEndTransaction();
        }
        
        bool MapDocument::submit(Command::Ptr command) {
            return doSubmit(command);
        }

        bool MapDocument::submitAndStore(UndoableCommand::Ptr command) {
            return doSubmitAndStore(command);
        }
        
        void MapDocument::commitPendingAssets() {
            m_textureManager->commitChanges();
        }
        
        void MapDocument::pick(const Ray3& pickRay, Model::PickResult& pickResult) const {
            if (m_world != NULL)
                m_world->pick(pickRay, pickResult);
        }
        
        Model::NodeList MapDocument::findNodesContaining(const Vec3& point) const {
            Model::NodeList result;
            if (m_world != NULL)
                m_world->findNodesContaining(point, result);
            return result;
        }

        void MapDocument::createWorld(const Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GamePtr game) {
            m_worldBounds = worldBounds;
            m_game = game;
            m_world = m_game->newMap(mapFormat, m_worldBounds);
            setCurrentLayer(m_world->defaultLayer());
            
            updateGameSearchPaths();
            setPath(IO::Path(DefaultDocumentName));
        }
        
        void MapDocument::loadWorld(const Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path) {
            m_worldBounds = worldBounds;
            m_game = game;
            m_world = m_game->loadMap(mapFormat, m_worldBounds, path, this);
            setCurrentLayer(m_world->defaultLayer());
            
            updateGameSearchPaths();
            setPath(path);
        }
        
        void MapDocument::clearWorld() {
            delete m_world;
            m_world = NULL;
            m_currentLayer = NULL;
        }
        
        void MapDocument::initializeWorld(const BBox3& worldBounds) {
            const Model::BrushBuilder builder(m_world, worldBounds);
            Model::Brush* brush = builder.createCuboid(Vec3(128.0, 128.0, 32.0), Model::BrushFace::NoTextureName);
            addNode(brush, m_world->defaultLayer());
        }
        
        Assets::EntityDefinitionFileSpec MapDocument::entityDefinitionFile() const {
            return m_game->extractEntityDefinitionFile(m_world);
        }
        
        Assets::EntityDefinitionFileSpec::List MapDocument::allEntityDefinitionFiles() const {
            return m_game->allEntityDefinitionFiles();
        }
        
        void MapDocument::setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec) {
            submitAndStore(EntityDefinitionFileCommand::set(spec));
        }
        
        IO::Path::List MapDocument::enabledTextureCollections() const {
            return m_game->extractTextureCollections(m_world);
        }
        
        IO::Path::List MapDocument::availableTextureCollections() const {
            return m_game->findTextureCollections();
        }
        
        void MapDocument::setEnabledTextureCollections(const IO::Path::List& paths) {
            submitAndStore(SetTextureCollectionsCommand::set(paths));
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
                IO::SimpleParserStatus status(this);
                m_entityDefinitionManager->loadDefinitions(path, *m_game, status);
                info("Loaded entity definition file " + path.lastComponent().asString());
            } catch (const Exception& e) {
                if (spec.builtin())
                    error("Could not load builtin entity definition file '%s': %s", spec.path().asString().c_str(), e.what());
                else
                    error("Could not load external entity definition file '%s': %s", spec.path().asString().c_str(), e.what());
            }
        }
        
        void MapDocument::unloadEntityDefinitions() {
            unsetEntityDefinitions();
            m_entityDefinitionManager->clear();
        }
        
        void MapDocument::loadEntityModels() {
            m_entityModelManager->setLoader(m_game.get());
        }
        
        void MapDocument::unloadEntityModels() {
            clearEntityModels();
            m_entityModelManager->setLoader(NULL);
        }
        
        void MapDocument::loadTextures() {
            try {
                const IO::Path docDir = m_path.isEmpty() ? IO::Path() : m_path.deleteLastComponent();
                m_game->loadTextureCollections(m_world, docDir, *m_textureManager);
            } catch (const Exception& e) {
                error(e.what());
            }
        }
        
        void MapDocument::unloadTextures() {
            unsetTextures();
            m_textureManager->clear();
        }
        
        void MapDocument::reloadTextures() {
            unsetTextures();
            loadTextures();
            setTextures();
        }

        class SetEntityDefinition : public Model::NodeVisitor {
        private:
            Assets::EntityDefinitionManager& m_manager;
        public:
            SetEntityDefinition(Assets::EntityDefinitionManager& manager) :
            m_manager(manager) {}
        private:
            void doVisit(Model::World* world)   { handle(world); }
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) { handle(entity); }
            void doVisit(Model::Brush* brush)   {}
            void handle(Model::AttributableNode* attributable) {
                Assets::EntityDefinition* definition = m_manager.definition(attributable);
                attributable->setDefinition(definition);
            }
        };
        
        class UnsetEntityDefinition : public Model::NodeVisitor {
        private:
            void doVisit(Model::World* world)   { world->setDefinition(NULL); }
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) { entity->setDefinition(NULL); }
            void doVisit(Model::Brush* brush)   {}
        };
        
        void MapDocument::setEntityDefinitions() {
            SetEntityDefinition visitor(*m_entityDefinitionManager);
            m_world->acceptAndRecurse(visitor);
        }
        
        void MapDocument::setEntityDefinitions(const Model::NodeList& nodes) {
            SetEntityDefinition visitor(*m_entityDefinitionManager);
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), visitor);
        }
        
        void MapDocument::unsetEntityDefinitions() {
            UnsetEntityDefinition visitor;
            m_world->acceptAndRecurse(visitor);
        }
        
        void MapDocument::unsetEntityDefinitions(const Model::NodeList& nodes) {
            UnsetEntityDefinition visitor;
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), visitor);
        }

        void MapDocument::reloadEntityDefinitions() {
            unloadEntityDefinitions();
            clearEntityModels();
            loadEntityDefinitions();
            setEntityDefinitions();
        }

        void MapDocument::clearEntityModels() {
            m_entityModelManager->clear();
        }
        
        class SetTextures : public Model::NodeVisitor {
        private:
            Assets::TextureManager* m_manager;
        public:
            SetTextures(Assets::TextureManager* manager) :
            m_manager(manager) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   {
                for (Model::BrushFace* face : brush->faces())
                    face->updateTexture(m_manager);
            }
        };
        
        void MapDocument::setTextures() {
            SetTextures visitor(m_textureManager);
            m_world->acceptAndRecurse(visitor);
        }
        
        void MapDocument::setTextures(const Model::NodeList& nodes) {
            SetTextures visitor(m_textureManager);
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), visitor);
        }
        
        void MapDocument::setTextures(const Model::BrushFaceList& faces) {
            for (Model::BrushFace* face : faces)
                face->updateTexture(m_textureManager);
        }
        
        class UnsetTextures : public Model::NodeVisitor {
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   {
                for (Model::BrushFace* face : brush->faces())
                    face->setTexture(NULL);
            }
        };
        
        void MapDocument::unsetTextures() {
            UnsetTextures visitor;
            m_world->acceptAndRecurse(visitor);
        }
        
        void MapDocument::unsetTextures(const Model::NodeList& nodes) {
            UnsetTextures visitor;
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), visitor);
        }

        IO::Path::List MapDocument::externalSearchPaths() const {
            IO::Path::List searchPaths;
            if (!m_path.isEmpty() && m_path.isAbsolute())
                searchPaths.push_back(m_path.deleteLastComponent());
            
            const IO::Path gamePath = m_game->gamePath();
            if (!gamePath.isEmpty())
                searchPaths.push_back(gamePath);
            
            searchPaths.push_back(IO::SystemPaths::appDirectory());
            return searchPaths;
        }
        
        void MapDocument::updateGameSearchPaths() {
            const StringList modNames = mods();
            IO::Path::List additionalSearchPaths;
            additionalSearchPaths.reserve(modNames.size());
            
            for (const String& modName : modNames)
                additionalSearchPaths.push_back(IO::Path(modName));
            m_game->setAdditionalSearchPaths(additionalSearchPaths);
        }
        
        StringList MapDocument::mods() const {
            return m_game->extractEnabledMods(m_world);
        }
        
        void MapDocument::setMods(const StringList& mods) {
            submitAndStore(SetModsCommand::set(mods));
        }
        
        String MapDocument::defaultMod() const {
            return m_game->defaultMod();
        }

        void MapDocument::setIssueHidden(Model::Issue* issue, const bool hidden) {
            doSetIssueHidden(issue, hidden);
        }
        
        void MapDocument::registerIssueGenerators() {
            ensure(m_world != NULL, "world is null");
            ensure(m_game.get() != NULL, "game is null");
            
            m_world->registerIssueGenerator(new Model::MissingClassnameIssueGenerator());
            m_world->registerIssueGenerator(new Model::MissingDefinitionIssueGenerator());
            m_world->registerIssueGenerator(new Model::EmptyGroupIssueGenerator());
            m_world->registerIssueGenerator(new Model::EmptyBrushEntityIssueGenerator());
            m_world->registerIssueGenerator(new Model::PointEntityWithBrushesIssueGenerator());
            m_world->registerIssueGenerator(new Model::LinkSourceIssueGenerator());
            m_world->registerIssueGenerator(new Model::LinkTargetIssueGenerator());
            m_world->registerIssueGenerator(new Model::NonIntegerPlanePointsIssueGenerator());
            m_world->registerIssueGenerator(new Model::NonIntegerVerticesIssueGenerator());
            m_world->registerIssueGenerator(new Model::MixedBrushContentsIssueGenerator());
            m_world->registerIssueGenerator(new Model::WorldBoundsIssueGenerator(m_worldBounds));
            m_world->registerIssueGenerator(new Model::EmptyAttributeNameIssueGenerator());
            m_world->registerIssueGenerator(new Model::EmptyAttributeValueIssueGenerator());
            m_world->registerIssueGenerator(new Model::LongAttributeNameIssueGenerator(m_game->maxPropertyLength()));
            m_world->registerIssueGenerator(new Model::LongAttributeValueIssueGenerator(m_game->maxPropertyLength()));
            m_world->registerIssueGenerator(new Model::AttributeNameWithDoubleQuotationMarksIssueGenerator());
            m_world->registerIssueGenerator(new Model::AttributeValueWithDoubleQuotationMarksIssueGenerator());
        }
        
        bool MapDocument::persistent() const {
            return m_path.isAbsolute() && IO::Disk::fileExists(IO::Disk::fixPath(m_path));
        }

        String MapDocument::filename() const {
            if (m_path.isEmpty())
                return EmptyString;
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
            m_mapViewConfig->mapViewConfigDidChangeNotifier.addObserver(mapViewConfigDidChangeNotifier);
            commandDoneNotifier.addObserver(this, &MapDocument::commandDone);
            commandUndoneNotifier.addObserver(this, &MapDocument::commandUndone);
        }
        
        void MapDocument::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapDocument::preferenceDidChange);
            m_editorContext->editorContextDidChangeNotifier.removeObserver(editorContextDidChangeNotifier);
            m_mapViewConfig->mapViewConfigDidChangeNotifier.removeObserver(mapViewConfigDidChangeNotifier);
            commandDoneNotifier.removeObserver(this, &MapDocument::commandDone);
            commandUndoneNotifier.removeObserver(this, &MapDocument::commandUndone);
        }
        
        void MapDocument::preferenceDidChange(const IO::Path& path) {
            if (isGamePathPreference(path)) {
                const Model::GameFactory& gameFactory = Model::GameFactory::instance();
                const IO::Path newGamePath = gameFactory.gamePath(m_game->gameName());
                m_game->setGamePath(newGamePath);
                
                clearEntityModels();
                
                unsetTextures();
                loadTextures();
                setTextures();
                
                //reloadIssues();
            } else if (path == Preferences::TextureMinFilter.path() ||
                       path == Preferences::TextureMagFilter.path()) {
                m_entityModelManager->setTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
                m_textureManager->setTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
            }
        }

        void MapDocument::commandDone(Command::Ptr command) {
            debug("Command '%s' executed", command->name().c_str());
        }
        
        void MapDocument::commandUndone(UndoableCommand::Ptr command) {
            debug("Command '%s' undone", command->name().c_str());
        }

        Transaction::Transaction(MapDocumentWPtr document, const String& name) :
        m_document(lock(document).get()),
        m_cancelled(false) {
            begin(name);
        }
        
        Transaction::Transaction(MapDocumentSPtr document, const String& name) :
        m_document(document.get()),
        m_cancelled(false) {
            begin(name);
        }
        
        Transaction::Transaction(MapDocument* document, const String& name) :
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
        
        void Transaction::begin(const String& name) {
            m_document->beginTransaction(name);
        }
        
        void Transaction::commit() {
            m_document->commitTransaction();
        }
    }
}
