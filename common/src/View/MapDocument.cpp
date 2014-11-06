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
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/TextureCollectionSpec.h"
#include "Assets/TextureManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/SystemPaths.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/CollectContainedNodesVisitor.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/CollectSelectableNodesWithFilePositionVisitor.h"
#include "Model/CollectTouchingNodesVisitor.h"
#include "Model/CollectUniqueNodesVisitor.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Game.h"
#include "Model/MergeNodesIntoWorldVisitor.h"
#include "Model/NodeVisitor.h"
#include "Model/PointFile.h"
#include "Model/World.h"
#include "View/AddRemoveNodesCommand.h"
#include "View/ChangeBrushFaceAttributesCommand.h"
#include "View/DuplicateNodesCommand.h"
#include "View/Grid.h"
#include "View/MapViewConfig.h"
#include "View/MoveBrushEdgesCommand.h"
#include "View/MoveBrushFacesCommand.h"
#include "View/MoveBrushVerticesCommand.h"
#include "View/MoveTexturesCommand.h"
#include "View/RotateTexturesCommand.h"
#include "View/SelectionCommand.h"
#include "View/SnapBrushVerticesCommand.h"
#include "View/TransformObjectsCommand.h"
#include "View/VertexHandleManager.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const BBox3 MapDocument::DefaultWorldBounds(-16384.0, 16384.0);
        const String MapDocument::DefaultDocumentName("unnamed.map");

        MapDocument::MapDocument() :
        m_worldBounds(DefaultWorldBounds),
        m_world(NULL),
        m_pointFile(NULL),
        m_editorContext(new Model::EditorContext()),
        m_entityDefinitionManager(new Assets::EntityDefinitionManager()),
        m_entityModelManager(new Assets::EntityModelManager(this)),
        m_textureManager(new Assets::TextureManager(this, pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter))),
        m_mapViewConfig(new MapViewConfig(*m_editorContext)),
        m_grid(new Grid(4)),
        m_textureLock(false),
        m_path(DefaultDocumentName),
        m_modificationCount(0),
        m_selectionBoundsValid(true) {}
        
        MapDocument::~MapDocument() {
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

        const BBox3& MapDocument::worldBounds() const {
            return m_worldBounds;
        }

        Model::World* MapDocument::world() const {
            return m_world;
        }

        const Model::EditorContext& MapDocument::editorContext() const {
            return *m_editorContext;
        }

        bool MapDocument::textureLock() {
            return m_editorContext->textureLock();
        }
            
        void MapDocument::setTextureLock(const bool textureLock) {
            m_editorContext->setTextureLock(textureLock);
        }

        Assets::EntityModelManager& MapDocument::entityModelManager() {
            return *m_entityModelManager;
        }

        const View::MapViewConfig& MapDocument::mapViewConfig() const {
            return *m_mapViewConfig;
        }

        Grid& MapDocument::grid() const {
            return *m_grid;
        }

        Model::PointFile* MapDocument::pointFile() const {
            return m_pointFile;
        }

        void MapDocument::newDocument(const BBox3& worldBounds, Model::GamePtr game, const Model::MapFormat::Type mapFormat) {
            info("Creating new document");

            clearDocument();
            createWorld(worldBounds, game, mapFormat);

            loadAssets();
            registerIssueGenerators();
            
            documentWasNewedNotifier(this);
        }
        
        void MapDocument::loadDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path) {
            info("Loading document from " + path.asString());

            clearDocument();
            loadWorld(worldBounds, game, path);

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
            clearModificationCount();
            setPath(path);
            documentWasSavedNotifier(this);
        }
        
        void MapDocument::clearDocument() {
            if (m_world != NULL) {
                documentWillBeClearedNotifier(this);
                
                clearSelection();
                clearWorld();
                unloadAssets();
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

        bool MapDocument::paste(const String& str) {
            const Model::NodeList nodes = m_game->parseNodes(str, m_world, m_worldBounds, this);
            if (!nodes.empty())
                return pasteNodes(nodes);
            
            const Model::BrushFaceList faces = m_game->parseBrushFaces(str, m_world, m_worldBounds, this);
            if (!faces.empty())
                return pasteBrushFaces(faces);
            
            return false;
        }
        
        bool MapDocument::pasteNodes(const Model::NodeList& nodes) {
            Model::MergeNodesIntoWorldVisitor mergeNodes(m_world, NULL);
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

        const Model::NodeCollection& MapDocument::selectedNodes() const {
            return m_selectedNodes;
        }
        
        const Model::BrushFaceList& MapDocument::selectedBrushFaces() const {
            return m_selectedBrushFaces;
        }

        const BBox3& MapDocument::selectionBounds() const {
            if (!m_selectionBoundsValid)
                validateSelectionBounds();
            return m_selectionBounds;
        }

        void MapDocument::selectAllNodes() {
            submit(SelectionCommand::selectAllNodes());
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

        template <typename V, typename I>
        Model::NodeList collectContainedOrTouchingNodes(I cur, I end, Model::World* world) {
            Model::NodeList result;
            while (cur != end) {
                V visitor(*cur);
                world->acceptAndRecurse(visitor);
                result = VectorUtils::setUnion(result, visitor.nodes());
                ++cur;
            }
            return result;
        };
        
        void MapDocument::selectTouching(const bool del) {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            const Model::NodeList nodes = collectContainedOrTouchingNodes<Model::CollectTouchingNodesVisitor>(brushes.begin(), brushes.end(), m_world);
            
            Transaction transaction(this, "Select touching");
            if (del)
                deleteObjects();
            else
                deselectAll();
            select(nodes);
        }
        
        void MapDocument::selectInside(const bool del) {
            const Model::BrushList& brushes = m_selectedNodes.brushes();
            const Model::NodeList nodes = collectContainedOrTouchingNodes<Model::CollectContainedNodesVisitor>(brushes.begin(), brushes.end(), m_world);

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
            submit(SelectionCommand::select(nodes));
        }
        
        void MapDocument::select(Model::Node* node) {
            submit(SelectionCommand::select(Model::NodeList(1, node)));
        }
        
        void MapDocument::select(const Model::BrushFaceList& faces) {
            submit(SelectionCommand::select(faces));
        }
        
        void MapDocument::select(Model::BrushFace* face) {
            submit(SelectionCommand::select(Model::BrushFaceList(1, face)));
        }
        
        void MapDocument::convertToFaceSelection() {
            submit(SelectionCommand::convertToFaces());
        }
        
        void MapDocument::deselectAll() {
            submit(SelectionCommand::deselectAll());
        }
        
        void MapDocument::deselect(Model::Node* node) {
            submit(SelectionCommand::deselect(Model::NodeList(1, node)));
        }
        
        void MapDocument::deselect(Model::BrushFace* face) {
            submit(SelectionCommand::deselect(Model::BrushFaceList(1, face)));
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

        bool MapDocument::deleteObjects() {
            Transaction transaction(this, "Delete objects");
            const Model::NodeList nodes = m_selectedNodes.nodes();
            deselectAll();
            return submit(AddRemoveNodesCommand::remove(nodes));
        }

        bool MapDocument::duplicateObjects() {
            return submit(DuplicateNodesCommand::duplicate());
        }

        bool MapDocument::translateObjects(const Vec3& delta) {
            return submit(TransformObjectsCommand::translate(delta, m_textureLock));
        }

        bool MapDocument::rotateObjects(const Vec3& center, const Vec3& axis, const FloatType angle) {
            return submit(TransformObjectsCommand::rotate(center, axis, angle, m_textureLock));
        }
        
        bool MapDocument::flipObjects(const Vec3& center, const Math::Axis::Type axis) {
            return submit(TransformObjectsCommand::flip(center, axis, m_textureLock));
        }

        bool MapDocument::setTexture(Assets::Texture* texture) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setTexture(texture);
            return submit(ChangeBrushFaceAttributesCommand::command(request));
        }
        
        bool MapDocument::setFaceAttributes(const Model::BrushFaceAttributes& attributes) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setAll(attributes);
            
            // try to find the texture if it is null, maybe it just wasn't set?
            if (attributes.texture() == NULL) {
                Assets::Texture* texture = m_textureManager->texture(attributes.textureName());
                request.setTexture(texture);
            }
            
            return submit(ChangeBrushFaceAttributesCommand::command(request));
        }

        bool MapDocument::moveTextures(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta) {
            return submit(MoveTexturesCommand::move(cameraUp, cameraRight, delta));
        }
        
        bool MapDocument::rotateTextures(const float angle) {
            return submit(RotateTexturesCommand::rotate(angle));
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

        MapDocument::MoveVerticesResult::MoveVerticesResult(const bool i_success, const bool i_hasRemainingVertices) :
        success(i_success),
        hasRemainingVertices(i_hasRemainingVertices) {}

        MapDocument::MoveVerticesResult MapDocument::moveVertices(const Model::VertexToBrushesMap& vertices, const Vec3& delta) {
            MoveBrushVerticesCommand* command = MoveBrushVerticesCommand::move(vertices, delta);
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
        }
        
        bool MapDocument::splitFaces(const Model::VertexToFacesMap& faces, const Vec3& delta) {
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
        
        void MapDocument::endTransaction() {
            doEndTransaction();
        }
        
        void MapDocument::rollbackTransaction() {
            doRollbackTransaction();
        }

        void MapDocument::cancelTransaction() {
            rollbackTransaction();
            endTransaction();
        }

        bool MapDocument::submit(UndoableCommand* command) {
            return doSubmit(command);
        }
        
        void MapDocument::commitPendingAssets() {
            m_textureManager->commitChanges();
        }

        Hits MapDocument::pick(const Ray3& pickRay) const {
            assert(m_world != NULL);
            return m_world->pick(pickRay);
        }

        void MapDocument::createWorld(const BBox3& worldBounds, Model::GamePtr game, const Model::MapFormat::Type mapFormat) {
            m_worldBounds = worldBounds;
            m_game = game;
            m_world = m_game->newMap(mapFormat);
            
            updateGameSearchPaths();
            setPath(DefaultDocumentName);
        }
        
        void MapDocument::loadWorld(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path) {
            m_worldBounds = worldBounds;
            m_game = game;
            m_world = m_game->loadMap(m_worldBounds, path, this);

            updateGameSearchPaths();
            setPath(path);
        }
        
        void MapDocument::clearWorld() {
            delete m_world;
            m_world = NULL;
        }

        Model::NodeList MapDocument::addNodes(const Model::ParentChildrenMap& nodes) {
            AddRemoveNodesCommand* command = AddRemoveNodesCommand::add(nodes);
            if (!submit(command))
                return Model::EmptyNodeList;
            return command->addedNodes();
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
            m_entityDefinitionManager->loadDefinitions(path, *m_game);
            info("Loaded entity definition file " + path.lastComponent().asString());
        }

        void MapDocument::unloadEntityDefinitions() {
            unsetEntityDefinitions();
            m_entityDefinitionManager->clear();
        }

        Assets::EntityDefinitionFileSpec MapDocument::entityDefinitionFile() const {
            return m_game->extractEntityDefinitionFile(m_world);
        }
        
        void MapDocument::loadEntityModels() {
            m_entityModelManager->setLoader(m_game.get());
        }

        void MapDocument::unloadEntityModels() {
            unsetEntityModels();
            m_entityModelManager->clear();
            m_entityModelManager->setLoader(NULL);
        }

        void MapDocument::loadTextures() {
            m_textureManager->setLoader(m_game.get());
            loadBuiltinTextures();
            loadExternalTextures();
        }
        
        void MapDocument::loadBuiltinTextures() {
            try {
                const IO::Path::List paths = m_game->findBuiltinTextureCollections();
                m_textureManager->setBuiltinTextureCollections(paths);
                info("Loaded builtin texture collections " + StringUtils::join(IO::Path::asStrings(paths), ", "));
            } catch (Exception e) {
                error(String(e.what()));
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
        
        void MapDocument::addExternalTextureCollections(const StringList& names) {
            const IO::Path::List searchPaths = externalSearchPaths();
            
            StringList::const_iterator it, end;
            for (it = names.begin(), end = names.end(); it != end; ++it) {
                const String& name = *it;
                const IO::Path texturePath(name);
                const IO::Path absPath = IO::Disk::resolvePath(searchPaths, texturePath);
                
                const Assets::TextureCollectionSpec spec(name, absPath);
                if (m_textureManager->addExternalTextureCollection(spec))
                    info("Loaded external texture collection '" + name +  "'");
                else
                    warn("External texture collection not found: '" + name +  "'");
            }
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
            void handle(Model::Attributable* attributable) {
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
        
        void MapDocument::registerIssueGenerators() {
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
            return m_modificationCount > 0;
        }

        void MapDocument::clearModificationCount() {
            m_modificationCount = 0;
            documentModificationStateDidChangeNotifier();
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
                end();
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
        
        void Transaction::end() {
            m_document->endTransaction();
        }
    }
}
