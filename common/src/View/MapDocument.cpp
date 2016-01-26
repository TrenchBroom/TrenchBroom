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

#include "View/MapDocument.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Polyhedron.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/TextureCollectionSpec.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/SystemPaths.h"
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
#include "Model/EmptyBrushEntityIssueGenerator.h"
#include "Model/Entity.h"
#include "Model/EntityLinkSourceIssueGenerator.h"
#include "Model/EntityLinkTargetIssueGenerator.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "Model/Group.h"
#include "Model/MergeNodesIntoWorldVisitor.h"
#include "Model/MissingEntityClassnameIssueGenerator.h"
#include "Model/MissingEntityDefinitionIssueGenerator.h"
#include "Model/MixedBrushContentsIssueGenerator.h"
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
#include "View/RenameGroupsCommand.h"
#include "View/ReparentNodesCommand.h"
#include "View/ResizeBrushesCommand.h"
#include "View/RotateTexturesCommand.h"
#include "View/SelectionCommand.h"
#include "View/SetLockStateCommand.h"
#include "View/SetModsCommand.h"
#include "View/SetVisibilityCommand.h"
#include "View/ShearTexturesCommand.h"
#include "View/SimpleParserStatus.h"
#include "View/SnapBrushVerticesCommand.h"
#include "View/SplitBrushEdgesCommand.h"
#include "View/SplitBrushFacesCommand.h"
#include "View/TextureCollectionCommand.h"
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
            return m_game != NULL && m_game->isGamePathPreference(path);
        }
        
        Model::Layer* MapDocument::currentLayer() const {
            assert(m_currentLayer != NULL);
            return m_currentLayer;
        }
        
        void MapDocument::setCurrentLayer(Model::Layer* currentLayer) {
            assert(currentLayer != NULL);
            assert(!currentLayer->locked());
            assert(!currentLayer->hidden());
            m_currentLayer = currentLayer;
            currentLayerDidChangeNotifier();
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
        
        bool MapDocument::textureLock() {
            return m_editorContext->textureLock();
        }
        
        void MapDocument::setTextureLock(const bool textureLock) {
            m_editorContext->setTextureLock(textureLock);
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
            assert(m_game != NULL);
            assert(m_world != NULL);
            m_game->writeMap(m_world, path);
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
                    error("Unable to parse clipboard contents: %s", e.what());
                }
            }
            return PT_Failed;
        }
        
        bool MapDocument::pasteNodes(const Model::NodeList& nodes) {
            Model::MergeNodesIntoWorldVisitor mergeNodes(m_world, currentLayer());
            Model::Node::accept(nodes.begin(), nodes.end(), mergeNodes);
            
            const Model::NodeList addedNodes = addNodes(mergeNodes.result());
            if (addedNodes.empty())
                return false;
            
            deselectAll();
            
            Model::CollectSelectableNodesVisitor collectSelectables(editorContext());
            Model::Node::acceptAndRecurse(addedNodes.begin(), addedNodes.end(), collectSelectables);
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
        
        bool MapDocument::canLoadPointFile() const {
            if (m_path.isEmpty())
                return false;
            const IO::Path pointFilePath = Model::PointFile::pointFilePath(m_path);
            return pointFilePath.isAbsolute() && IO::Disk::fileExists(pointFilePath);
        }
        
        void MapDocument::loadPointFile() {
            assert(canLoadPointFile());
            if (isPointFileLoaded())
                unloadPointFile();
            m_pointFile = new Model::PointFile(m_path);
            info("Loaded point file");
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
            Model::CollectAttributableNodesVisitor visitor;
            Model::Node::accept(m_selectedNodes.begin(), m_selectedNodes.end(), visitor);
            return visitor.nodes();
        }
        
        const Model::NodeCollection& MapDocument::selectedNodes() const {
            return m_selectedNodes;
        }
        
        const Model::BrushFaceList MapDocument::allSelectedBrushFaces() const {
            if (hasSelectedBrushFaces())
                return selectedBrushFaces();
            Model::CollectBrushFacesVisitor visitor;
            Model::Node::acceptAndRecurse(m_selectedNodes.begin(), m_selectedNodes.end(), visitor);
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
        
        void MapDocument::selectAllNodes() {
            submit(UndoableCommand::Ptr(SelectionCommand::selectAllNodes()));
        }
        
        void MapDocument::selectSiblings() {
            const Model::NodeList& nodes = selectedNodes().nodes();
            if (nodes.empty())
                return;
            
            Model::CollectSelectableUniqueNodesVisitor visitor(*m_editorContext);
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                Model::Node* parent = node->parent();
                parent->iterate(visitor);
            }
            
            Transaction transaction(this, "Select siblings");
            deselectAll();
            select(visitor.nodes());
        }
        
        void MapDocument::selectTouching(const bool del) {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            const Model::NodeList nodes = Model::collectMatchingNodes<Model::CollectTouchingNodesVisitor>(brushes.begin(), brushes.end(), m_world);
            
            Transaction transaction(this, "Select touching");
            if (del)
                deleteObjects();
            else
                deselectAll();
            select(nodes);
        }
        
        void MapDocument::selectInside(const bool del) {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            const Model::NodeList nodes = Model::collectMatchingNodes<Model::CollectContainedNodesVisitor>(brushes.begin(), brushes.end(), m_world);
            
            Transaction transaction(this, "Select inside");
            if (del)
                deleteObjects();
            else
                deselectAll();
            select(nodes);
        }
        
        void MapDocument::selectNodesWithFilePosition(const std::vector<size_t>& positions) {
            Model::CollectSelectableNodesWithFilePositionVisitor visitor(*m_editorContext, positions);
            m_world->acceptAndRecurse(visitor);
            
            Transaction transaction(this, "Select by line number");
            deselectAll();
            select(visitor.nodes());
        }
        
        void MapDocument::select(const Model::NodeList& nodes) {
            submit(UndoableCommand::Ptr(SelectionCommand::select(nodes)));
        }
        
        void MapDocument::select(Model::Node* node) {
            submit(UndoableCommand::Ptr(SelectionCommand::select(Model::NodeList(1, node))));
        }
        
        void MapDocument::select(const Model::BrushFaceList& faces) {
            submit(UndoableCommand::Ptr(SelectionCommand::select(faces)));
        }
        
        void MapDocument::select(Model::BrushFace* face) {
            submit(UndoableCommand::Ptr(SelectionCommand::select(Model::BrushFaceList(1, face))));
            m_currentTextureName = face->textureName();
        }
        
        void MapDocument::convertToFaceSelection() {
            submit(UndoableCommand::Ptr(SelectionCommand::convertToFaces()));
        }
        
        void MapDocument::deselectAll() {
            submit(UndoableCommand::Ptr(SelectionCommand::deselectAll()));
        }
        
        void MapDocument::deselect(Model::Node* node) {
            deselect(Model::NodeList(1, node));
        }
        
        void MapDocument::deselect(const Model::NodeList& nodes) {
            submit(UndoableCommand::Ptr(SelectionCommand::deselect(nodes)));
        }
        
        void MapDocument::deselect(Model::BrushFace* face) {
            submit(UndoableCommand::Ptr(SelectionCommand::deselect(Model::BrushFaceList(1, face))));
        }
        
        void MapDocument::updateLastSelectionBounds() {
            m_lastSelectionBounds = selectionBounds();
        }
        
        void MapDocument::invalidateSelectionBounds() {
            m_selectionBoundsValid = false;
        }
        
        void MapDocument::validateSelectionBounds() const {
            Model::ComputeNodeBoundsVisitor visitor;
            Model::Node::accept(m_selectedNodes.begin(), m_selectedNodes.end(), visitor);
            m_selectionBounds = visitor.bounds();
            m_selectionBoundsValid = true;
        }
        
        void MapDocument::clearSelection() {
            m_selectedNodes.clear();
            m_selectedBrushFaces.clear();
        }
        
        void MapDocument::addNode(Model::Node* node, Model::Node* parent) {
            assert(node != NULL);
            assert(node->parent() == NULL);
            assert(parent != NULL);
            assert(parent != node);
            
            Model::ParentChildrenMap map;
            map[parent].push_back(node);
            addNodes(map);
        }
        
        void MapDocument::removeNode(Model::Node* node) {
            removeNodes(Model::NodeList(1, node));
        }
        
        Model::NodeList MapDocument::addNodes(const Model::ParentChildrenMap& nodes) {
            Transaction transaction(this, "Add objects");
            AddRemoveNodesCommand::Ptr command = AddRemoveNodesCommand::add(nodes);
            if (!submit(command))
                return Model::EmptyNodeList;
            
            const Model::NodeList& addedNodes = command->addedNodes();
            ensureVisible(addedNodes);
            return addedNodes;
        }
        
        Model::NodeList MapDocument::addNodes(const Model::NodeList& nodes, Model::Node* parent) {
            AddRemoveNodesCommand::Ptr command = AddRemoveNodesCommand::add(parent, nodes);
            if (!submit(command))
                return Model::EmptyNodeList;

            const Model::NodeList& addedNodes = command->addedNodes();
            ensureVisible(addedNodes);
            return addedNodes;
        }

        void MapDocument::removeNodes(const Model::NodeList& nodes) {
            submit(AddRemoveNodesCommand::remove(nodes));
        }
        
        void MapDocument::reparentNodes(Model::Node* newParent, const Model::NodeList& children) {
            submit(ReparentNodesCommand::reparent(newParent, children));
        }
        
        void MapDocument::reparentNodes(const Model::ParentChildrenMap& nodes) {
            submit(ReparentNodesCommand::reparent(nodes));
        }
        
        bool MapDocument::deleteObjects() {
            Transaction transaction(this, "Delete objects");
            const Model::NodeList nodes = m_selectedNodes.nodes();
            deselectAll();
            return submit(AddRemoveNodesCommand::remove(nodes));
        }
        
        bool MapDocument::duplicateObjects() {
            if (submit(DuplicateNodesCommand::duplicate())) {
                m_viewEffectsService->flashSelection();
                return true;
            }
            return false;
        }
        
        void MapDocument::groupSelection(const String& name) {
            if (!hasSelectedNodes())
                return;
            
            const Model::NodeList nodes = m_selectedNodes.nodes();
            Model::Group* group = new Model::Group(name);
            
            const Transaction transaction(this, "Group Selected Objects");
            deselectAll();
            addNode(group, currentParent());
            reparentNodes(group, nodes);
            select(group);
        }
        
        void MapDocument::ungroupSelection() {
            if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
                return;
            
            const Model::NodeList groups = m_selectedNodes.nodes();
            Model::NodeList allChildren;
            
            const Transaction transaction(this, "Ungroup");
            deselectAll();
            
            Model::NodeList::const_iterator it, end;
            for (it = groups.begin(), end = groups.end(); it != end; ++it) {
                Model::Node* group = *it;
                Model::Layer* layer = Model::findLayer(group);
                const Model::NodeList& children = group->children();
                reparentNodes(layer, children);
                VectorUtils::append(allChildren, children);
            }
            
            select(allChildren);
        }
        
        void MapDocument::renameGroups(const String& name) {
            submit(RenameGroupsCommand::rename(name));
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
            submit(CurrentGroupCommand::push(group));
        }
        
        void MapDocument::closeGroup() {
            const Transaction transaction(this, "Close Group");
            
            deselectAll();
            Model::Group* previousGroup = m_editorContext->currentGroup();
            resetLock(Model::NodeList(1, previousGroup));
            submit(CurrentGroupCommand::pop());

            Model::Group* currentGroup = m_editorContext->currentGroup();
            if (currentGroup != NULL)
                unlock(Model::NodeList(1, currentGroup));
            else
                unlock(Model::NodeList(1, m_world));
        }
        
        void MapDocument::isolate(const Model::NodeList& nodes) {
            const Model::LayerList& layers = m_world->allLayers();
            Model::CollectUnselectedNodesVisitor collect;
            Model::Node::recurse(layers.begin(), layers.end(), collect);
            
            submit(SetVisibilityCommand::hide(collect.nodes()));
        }
        
        void MapDocument::hide(const Model::NodeList nodes) {
            Model::CollectSelectedNodesVisitor collect;
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), collect);
            
            const Transaction transaction(this, "Hide Objects");
            deselect(collect.nodes());
            submit(SetVisibilityCommand::hide(nodes));
        }
        
        void MapDocument::hideSelection() {
            hide(m_selectedNodes.nodes());
        }
        
        void MapDocument::show(const Model::NodeList& nodes) {
            submit(SetVisibilityCommand::show(nodes));
        }
        
        void MapDocument::showAll() {
            const Model::LayerList& layers = m_world->allLayers();
            Model::CollectNodesVisitor collect;
            Model::Node::recurse(layers.begin(), layers.end(), collect);
            resetVisibility(collect.nodes());
        }
        
        void MapDocument::ensureVisible(const Model::NodeList& nodes) {
            submit(SetVisibilityCommand::ensureVisible(nodes));
        }

        void MapDocument::resetVisibility(const Model::NodeList& nodes) {
            submit(SetVisibilityCommand::reset(nodes));
        }
        
        void MapDocument::lock(const Model::NodeList& nodes) {
            Model::CollectSelectedNodesVisitor collect;
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), collect);
            
            const Transaction transaction(this, "Lock Objects");
            submit(SetLockStateCommand::lock(nodes));
            deselect(collect.nodes());
        }
        
        void MapDocument::unlock(const Model::NodeList& nodes) {
            submit(SetLockStateCommand::unlock(nodes));
        }
        
        void MapDocument::resetLock(const Model::NodeList& nodes) {
            submit(SetLockStateCommand::reset(nodes));
        }
        
        bool MapDocument::translateObjects(const Vec3& delta) {
            return submit(TransformObjectsCommand::translate(delta, textureLock()));
        }
        
        bool MapDocument::rotateObjects(const Vec3& center, const Vec3& axis, const FloatType angle) {
            return submit(TransformObjectsCommand::rotate(center, axis, angle, textureLock()));
        }
        
        bool MapDocument::flipObjects(const Vec3& center, const Math::Axis::Type axis) {
            return submit(TransformObjectsCommand::flip(center, axis, textureLock()));
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
                const Model::BrushFaceList& faces = selectedBrushFaces();
                Model::BrushFaceList::const_iterator fIt, fEnd;
                for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                    const Model::BrushFace* face = *fIt;
                    const Model::BrushFace::VertexList vertices = face->vertices();
                    Model::BrushFace::VertexList::const_iterator vIt, vEnd;
                    for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                        const Model::BrushVertex* vertex = *vIt;
                        polyhedron.addPoint(vertex->position());
                    }
                }
            } else if (selectedNodes().hasOnlyBrushes()) {
                const Model::BrushList& brushes = selectedNodes().brushes();
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    const Model::Brush* brush = *bIt;
                    const Model::Brush::VertexList vertices = brush->vertices();
                    Model::Brush::VertexList::const_iterator vIt, vEnd;
                    for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                        const Model::BrushVertex* vertex = *vIt;
                        polyhedron.addPoint(vertex->position());
                    }
                }
            }
            
            if (!polyhedron.polyhedron() || !polyhedron.closed())
                return false;
            
            const Model::BrushBuilder builder(m_world, m_worldBounds);
            Model::Brush* brush = builder.createBrush(polyhedron, currentTextureName());
            brush->cloneFaceAttributesFrom(selectedNodes().brushes());
            
            // The nodelist is either empty or contains only brushes.
            const Model::NodeList toRemove = selectedNodes().nodes();
            
            const Transaction transaction(this, "CSG Convex Merge");
            deselectAll();
            removeNodes(toRemove);
            addNode(brush, currentParent());
            select(brush);
            return true;
        }
        
        bool MapDocument::csgSubtract() {
            const Model::BrushList brushes = selectedNodes().brushes();
            if (brushes.size() < 2)
                return false;
            
            const Model::BrushList minuends(brushes.begin(), brushes.end() - 1);
            Model::Brush* subtrahend = brushes.back();
            
            Model::ParentChildrenMap toAdd;
            Model::NodeList toRemove;
            toRemove.push_back(subtrahend);
            
            Model::BrushList::const_iterator it, end;
            for (it = minuends.begin(), end = minuends.end(); it != end; ++it) {
                Model::Brush* minuend = *it;
                const Model::BrushList result = minuend->subtract(*m_world, m_worldBounds, currentTextureName(), subtrahend);
                if (!result.empty()) {
                    VectorUtils::append(toAdd[minuend->parent()], result);
                    toRemove.push_back(minuend);
                }
            }
            
            Transaction transaction(this, "CSG Subtract");
            deselectAll();
            removeNodes(toRemove);
            select(addNodes(toAdd));
            
            return true;
        }

        bool MapDocument::csgIntersect() {
            const Model::BrushList brushes = selectedNodes().brushes();
            if (brushes.size() < 2)
                return false;
            
            Model::Brush* result = brushes.front()->clone(m_worldBounds);

            bool empty = false;
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end && !empty; ++it) {
                Model::Brush* brush = *it;
                empty |= !result->intersect(m_worldBounds, brush);
            }
            
            const Model::NodeList toRemove(brushes.begin(), brushes.end());
            
            Transaction transaction(this, "CSG Intersect");
            deselect(toRemove);
            removeNodes(toRemove);
            
            if (!empty) {
                addNode(result, currentParent());
                select(result);
            } else {
                delete result;
            }
            
            return true;
        }

        bool MapDocument::setAttribute(const Model::AttributeName& name, const Model::AttributeValue& value) {
            return submit(ChangeEntityAttributesCommand::set(name, value));
        }
        
        bool MapDocument::renameAttribute(const Model::AttributeName& oldName, const Model::AttributeName& newName) {
            return submit(ChangeEntityAttributesCommand::rename(oldName, newName));
        }
        
        bool MapDocument::removeAttribute(const Model::AttributeName& name) {
            return submit(ChangeEntityAttributesCommand::remove(name));
        }
        
        bool MapDocument::convertEntityColorRange(const Model::AttributeName& name, Assets::ColorRange::Type range) {
            return submit(ConvertEntityColorCommand::convert(name, range));
        }
        
        bool MapDocument::resizeBrushes(const Model::BrushFaceList& faces, const Vec3& delta) {
            return submit(ResizeBrushesCommand::resize(faces, delta));
        }
        
        bool MapDocument::setTexture(Assets::Texture* texture) {
            if (hasSelectedBrushFaces() || m_selectedNodes.hasOnlyBrushes()) {
                Model::ChangeBrushFaceAttributesRequest request;
                request.setTexture(texture);
                if (submit(ChangeBrushFaceAttributesCommand::command(request))) {
                    if (texture != NULL)
                        m_currentTextureName = texture->name();
                    return true;
                }
            } else if (texture != NULL) {
                m_currentTextureName = texture->name();
                return true;
            }
            return false;
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
            return submit(ChangeBrushFaceAttributesCommand::command(request));
        }
        
        bool MapDocument::moveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) {
            return submit(MoveTexturesCommand::move(cameraUp, cameraRight, delta));
        }
        
        bool MapDocument::rotateTextures(const float angle) {
            return submit(RotateTexturesCommand::rotate(angle));
        }
        
        bool MapDocument::shearTextures(const Vec2f& factors) {
            return submit(ShearTexturesCommand::shear(factors));
        }
        
        void MapDocument::rebuildBrushGeometry(const Model::BrushList& brushes) {
            performRebuildBrushGeometry(brushes);
        }
        
        bool MapDocument::snapVertices(const Model::VertexToBrushesMap& vertices, const size_t snapTo) {
            if (vertices.empty()) {
                assert(m_selectedNodes.hasOnlyBrushes());
                return submit(SnapBrushVerticesCommand::snap(m_selectedNodes.brushes(), snapTo));
            }
            return submit(SnapBrushVerticesCommand::snap(vertices, snapTo));
        }
        
        bool MapDocument::findPlanePoints() {
            return submit(FindPlanePointsCommand::findPlanePoints());
        }
        
        MapDocument::MoveVerticesResult MapDocument::moveVertices(const Model::VertexToBrushesMap& vertices, const Vec3& delta) {
            MoveBrushVerticesCommand::Ptr command = MoveBrushVerticesCommand::move(vertices, delta);
            const bool success = submit(command);
            const bool hasRemainingVertices = command->hasRemainingVertices();
            return MoveVerticesResult(success, hasRemainingVertices);
        }
        
        bool MapDocument::moveEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta) {
            return submit(MoveBrushEdgesCommand::move(edges, delta));
        }
        
        bool MapDocument::moveFaces(const Model::VertexToFacesMap& faces, const Vec3& delta) {
            return submit(MoveBrushFacesCommand::move(faces, delta));
        }
        
        bool MapDocument::splitEdges(const Model::VertexToEdgesMap& edges, const Vec3& delta) {
            return submit(SplitBrushEdgesCommand::split(edges, delta));
        }
        
        bool MapDocument::splitFaces(const Model::VertexToFacesMap& faces, const Vec3& delta) {
            return submit(SplitBrushFacesCommand::split(faces, delta));
        }
        
        void MapDocument::printVertices() {
            if (hasSelectedBrushFaces()) {
                Model::BrushFaceList::const_iterator fIt, fEnd;
                for (fIt = m_selectedBrushFaces.begin(), fEnd = m_selectedBrushFaces.end(); fIt != fEnd; ++fIt) {
                    const Model::BrushFace* face = *fIt;
                    const Model::BrushFace::VertexList vertices = face->vertices();
                    StringStream str;
                    Model::BrushFace::VertexList::const_iterator vIt, vEnd;
                    for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                        const Model::BrushVertex* vertex = *vIt;
                        str << "(" << vertex->position().asString() << ") ";
                    }
                    info(str.str());
                }
            } else if (selectedNodes().hasBrushes()) {
                const Model::BrushList& brushes = selectedNodes().brushes();
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    const Model::Brush* brush = *bIt;
                    const Model::Brush::VertexList vertices = brush->vertices();
                    StringStream str;
                    Model::Brush::VertexList::const_iterator vIt, vEnd;
                    for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                        const Model::BrushVertex* vertex = *vIt;
                        str << "(" << vertex->position().asString() << ") ";
                    }
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
        
        bool MapDocument::submit(UndoableCommand::Ptr command) {
            return doSubmit(command);
        }
        
        void MapDocument::commitPendingAssets() {
            m_textureManager->commitChanges();
        }
        
        void MapDocument::pick(const Ray3& pickRay, Model::PickResult& pickResult) const {
            if (m_world != NULL)
                m_world->pick(pickRay, pickResult);
        }
        
        void MapDocument::createWorld(const Model::MapFormat::Type mapFormat, const BBox3& worldBounds, Model::GamePtr game) {
            m_worldBounds = worldBounds;
            m_game = game;
            m_world = m_game->newMap(mapFormat, m_worldBounds);
            setCurrentLayer(m_world->defaultLayer());
            
            updateGameSearchPaths();
            setPath(DefaultDocumentName);
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
            submit(EntityDefinitionFileCommand::set(spec));
        }
        
        const StringList MapDocument::externalTextureCollectionNames() const {
            return m_textureManager->externalCollectionNames();
        }
        
        void MapDocument::addTextureCollection(const String& name) {
            submit(TextureCollectionCommand::add(name));
        }
        
        void MapDocument::moveTextureCollectionUp(const String& name) {
            submit(TextureCollectionCommand::moveUp(name));
        }
        
        void MapDocument::moveTextureCollectionDown(const String& name) {
            submit(TextureCollectionCommand::moveDown(name));
        }
        
        void MapDocument::removeTextureCollections(const StringList& names) {
            submit(TextureCollectionCommand::remove(names));
        }
        
        void MapDocument::loadAssets() {
            loadEntityDefinitions();
            setEntityDefinitions();
            loadEntityModels();
            setEntityModels();
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
            const IO::Path path = m_game->findEntityDefinitionFile(spec, externalSearchPaths());
            SimpleParserStatus status(this);
            
            m_entityDefinitionManager->loadDefinitions(path, *m_game, status);
            info("Loaded entity definition file " + path.lastComponent().asString());
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
            m_textureManager->setLoader(m_game.get());
            loadBuiltinTextures();
            loadExternalTextures();
        }
        
        void MapDocument::loadBuiltinTextures() {
            const IO::Path::List paths = m_game->findBuiltinTextureCollections();
            const String names(StringUtils::join(IO::Path::asStrings(paths), ", "));
            try {
                m_textureManager->setBuiltinTextureCollections(paths);
                info("Loaded builtin texture collections '%s'", names.c_str());
            } catch (Exception e) {
                error("Unable to load internal texture collections '%s': %s", names.c_str(), e.what());
            }
        }
        
        void MapDocument::loadExternalTextures() {
            const StringList names = m_game->extractExternalTextureCollections(m_world);
            addExternalTextureCollections(names);
        }
        
        void MapDocument::unloadTextures() {
            unsetTextures();
            m_textureManager->clear();
            m_textureManager->setLoader(NULL);
        }
        
        void MapDocument::reloadTextures() {
            unloadTextures();
            loadTextures();
            setTextures();
        }

        void MapDocument::addExternalTextureCollections(const StringList& names) {
            const IO::Path::List searchPaths = externalSearchPaths();
            
            StringList::const_iterator it, end;
            for (it = names.begin(), end = names.end(); it != end; ++it) {
                const String& name = *it;
                const IO::Path texturePath(name);
                const IO::Path absPath = IO::Disk::resolvePath(searchPaths, texturePath);
                
                try {
                    const Assets::TextureCollectionSpec spec(name, absPath);
                    m_textureManager->addExternalTextureCollection(spec);
                    info("Loaded external texture collection '%s'", name.c_str());
                } catch (std::exception& e) {
                    error("Unable to load external texture collection '%s': %s", name.c_str(), e.what());
                }
            }
        }
        
        void MapDocument::updateExternalTextureCollectionProperty() {
            m_game->updateExternalTextureCollections(m_world, m_textureManager->externalCollectionNames());
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
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
        }
        
        void MapDocument::unsetEntityDefinitions() {
            UnsetEntityDefinition visitor;
            m_world->acceptAndRecurse(visitor);
        }
        
        void MapDocument::reloadEntityDefinitions() {
            unloadEntityDefinitions();
            clearEntityModels();
            loadEntityDefinitions();
            setEntityDefinitions();
            setEntityModels();
        }
        
        class SetEntityModel : public Model::NodeVisitor {
        private:
            Assets::EntityModelManager& m_manager;
            Logger& m_logger;
        public:
            SetEntityModel(Assets::EntityModelManager& manager, Logger& logger) :
            m_manager(manager),
            m_logger(logger) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {
                try {
                    const Assets::ModelSpecification spec = entity->modelSpecification();
                    if (spec.path.isEmpty()) {
                        entity->setModel(NULL);
                    } else {
                        Assets::EntityModel* model = m_manager.model(spec.path);
                        entity->setModel(model);
                    }
                } catch (const GameException& e) {
                    m_logger.error(String(e.what()));
                }
                
            }
            void doVisit(Model::Brush* brush)   {}
        };
        
        class UnsetEntityModel : public Model::NodeVisitor {
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) { entity->setModel(NULL); }
            void doVisit(Model::Brush* brush)   {}
        };
        
        void MapDocument::setEntityModels() {
            SetEntityModel visitor(*m_entityModelManager, *this);
            m_world->acceptAndRecurse(visitor);
        }
        
        void MapDocument::setEntityModels(const Model::NodeList& nodes) {
            SetEntityModel visitor(*m_entityModelManager, *this);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
        }
        
        void MapDocument::clearEntityModels() {
            unsetEntityModels();
            m_entityModelManager->clear();
        }
        
        void MapDocument::unsetEntityModels() {
            UnsetEntityModel visitor;
            m_world->acceptAndRecurse(visitor);
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
                const Model::BrushFaceList& faces = brush->faces();
                Model::BrushFaceList::const_iterator it, end;
                for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                    Model::BrushFace* face = *it;
                    face->updateTexture(m_manager);
                }
            }
        };
        
        void MapDocument::setTextures() {
            SetTextures visitor(m_textureManager);
            m_world->acceptAndRecurse(visitor);
        }
        
        void MapDocument::setTextures(const Model::NodeList& nodes) {
            SetTextures visitor(m_textureManager);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
        }
        
        void MapDocument::setTextures(const Model::BrushFaceList& faces) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                face->updateTexture(m_textureManager);
            }
        }
        
        class UnsetTextures : public Model::NodeVisitor {
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   {
                const Model::BrushFaceList& faces = brush->faces();
                Model::BrushFaceList::const_iterator it, end;
                for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                    Model::BrushFace* face = *it;
                    face->setTexture(NULL);
                }
            }
        };
        
        void MapDocument::unsetTextures() {
            UnsetTextures visitor;
            m_world->acceptAndRecurse(visitor);
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
            
            StringList::const_iterator it, end;
            for (it = modNames.begin(), end = modNames.end(); it != end; ++it)
                additionalSearchPaths.push_back(IO::Path(*it));
            m_game->setAdditionalSearchPaths(additionalSearchPaths);
        }
        
        StringList MapDocument::mods() const {
            return m_game->extractEnabledMods(m_world);
        }
        
        void MapDocument::setMods(const StringList& mods) {
            submit(SetModsCommand::set(mods));
        }
        
        void MapDocument::setIssueHidden(Model::Issue* issue, const bool hidden) {
            doSetIssueHidden(issue, hidden);
        }
        
        void MapDocument::registerIssueGenerators() {
            assert(m_world != NULL);
            
            m_world->registerIssueGenerator(new Model::MissingEntityClassnameIssueGenerator());
            m_world->registerIssueGenerator(new Model::MissingEntityDefinitionIssueGenerator());
            m_world->registerIssueGenerator(new Model::EmptyBrushEntityIssueGenerator());
            m_world->registerIssueGenerator(new Model::PointEntityWithBrushesIssueGenerator());
            m_world->registerIssueGenerator(new Model::EntityLinkSourceIssueGenerator());
            m_world->registerIssueGenerator(new Model::EntityLinkTargetIssueGenerator());
            m_world->registerIssueGenerator(new Model::NonIntegerPlanePointsIssueGenerator());
            m_world->registerIssueGenerator(new Model::NonIntegerVerticesIssueGenerator());
            m_world->registerIssueGenerator(new Model::MixedBrushContentsIssueGenerator());
            m_world->registerIssueGenerator(new Model::WorldBoundsIssueGenerator(m_worldBounds));
        }
        
        const String MapDocument::filename() const {
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
        }
        
        void MapDocument::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapDocument::preferenceDidChange);
            m_editorContext->editorContextDidChangeNotifier.removeObserver(editorContextDidChangeNotifier);
            m_mapViewConfig->mapViewConfigDidChangeNotifier.removeObserver(mapViewConfigDidChangeNotifier);
        }
        
        void MapDocument::preferenceDidChange(const IO::Path& path) {
            if (isGamePathPreference(path)) {
                const Model::GameFactory& gameFactory = Model::GameFactory::instance();
                const IO::Path newGamePath = gameFactory.gamePath(m_game->gameName());
                m_game->setGamePath(newGamePath);
                
                clearEntityModels();
                setEntityModels();
                
                unsetTextures();
                loadBuiltinTextures();
                setTextures();
                
                //reloadIssues();
            } else if (path == Preferences::TextureMinFilter.path() ||
                       path == Preferences::TextureMagFilter.path()) {
                m_entityModelManager->setTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
                m_textureManager->setTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
            }
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
