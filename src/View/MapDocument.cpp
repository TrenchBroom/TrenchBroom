/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MapDocument.h"

#include "Logger.h"
#include "Assets/EntityDefinition.h"
#include "Assets/FaceTexture.h"
#include "Assets/ModelDefinition.h"
#include "Controller/EntityPropertyCommand.h"
#include "IO/FileSystem.h"
#include "Model/BrushFace.h"
#include "Model/EntityBrushesIterator.h"
#include "Model/EntityFacesIterator.h"
#include "Model/Game.h"
#include "Model/Map.h"
#include "Model/MapFacesIterator.h"
#include "Model/MapObjectsIterator.h"
#include "Model/ModelUtils.h"
#include "Model/SelectionResult.h"
#include "View/MapFrame.h"

#include <cassert>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>

namespace TrenchBroom {
    namespace View {
        class SetEntityDefinition {
        private:
            Assets::EntityDefinitionManager& m_definitionManager;
        public:
            SetEntityDefinition(Assets::EntityDefinitionManager& definitionManager) :
            m_definitionManager(definitionManager) {}
            
            inline void operator()(Model::Entity* entity) const {
                Assets::EntityDefinition* definition = m_definitionManager.definition(entity);
                entity->setDefinition(definition);
            }
        };
        
        struct UnsetEntityDefinition {
            inline void operator()(Model::Entity* entity) const {
                entity->setDefinition(NULL);
            }
        };

        class SetEntityModel {
        private:
            Assets::EntityModelManager& m_modelManager;
        public:
            SetEntityModel(Assets::EntityModelManager& modelManager) :
            m_modelManager(modelManager) {}
            
            inline void operator()(Model::Entity* entity) const {
                const Assets::ModelSpecification spec = entity->modelSpecification();
                if (spec.path.isEmpty()) {
                    entity->setModel(NULL);
                } else {
                    Assets::EntityModel* model = m_modelManager.model(spec.path);
                    entity->setModel(model);
                }
            }
        };
        
        struct UnsetEntityModel {
            inline void operator()(Model::Entity* entity) const {
                entity->setModel(NULL);
            }
        };

        class SetFaceTexture {
        private:
            Assets::TextureManager& m_textureManager;
        public:
            SetFaceTexture(Assets::TextureManager& textureManager) :
            m_textureManager(textureManager) {}
            
            inline void operator()(Model::BrushFace* face) const {
                const String& textureName = face->textureName();
                Assets::FaceTexture* texture = m_textureManager.texture(textureName);
                face->setTexture(texture);
            }
        };
        
        struct UnsetFaceTexture {
            inline void operator()(Model::BrushFace* face) const {
                face->setTexture(NULL);
            }
        };
        
        class AddToPicker {
        private:
            Model::Picker& m_picker;
        public:
            AddToPicker(Model::Picker& picker) :
            m_picker(picker) {}
            
            inline void operator()(Model::Object* object) const {
                m_picker.addObject(object);
            }
        };
        
        class RemoveFromPicker {
        private:
            Model::Picker& m_picker;
        public:
            RemoveFromPicker(Model::Picker& picker) :
            m_picker(picker) {}
            
            inline void operator()(Model::Object* object) const {
                m_picker.removeObject(object);
            }
        };
        
        class AddToMap {
        private:
            Model::Map& m_map;
        public:
            AddToMap(Model::Map& map) :
            m_map(map) {}
            
            inline void operator()(Model::Entity* entity) const {
                m_map.addEntity(entity);
            }
        };
        
        class RemoveFromMap {
        private:
            Model::Map& m_map;
        public:
            RemoveFromMap(Model::Map& map) :
            m_map(map) {}
            
            inline void operator()(Model::Entity* entity) const {
                m_map.removeEntity(entity);
            }
        };
        
        class AddToEntity {
        private:
            Model::Entity& m_entity;
        public:
            AddToEntity(Model::Entity& entity) :
            m_entity(entity) {}
            
            inline void operator()(Model::Brush* brush) const {
                m_entity.addBrush(brush);
            }
        };
        
        class RemoveFromEntity {
        private:
            Model::Entity& m_entity;
        public:
            RemoveFromEntity(Model::Entity& entity) :
            m_entity(entity) {}
            
            inline void operator()(Model::Brush* brush) const {
                m_entity.removeBrush(brush);
            }
        };
        
        const BBox3 MapDocument::DefaultWorldBounds(-16384.0, 16384.0);
        
        MapDocumentPtr MapDocument::newMapDocument() {
            return MapDocumentPtr(new MapDocument());
        }

        MapDocument::~MapDocument() {
            delete m_map;
            m_map = NULL;
        }
        
        const BBox3& MapDocument::worldBounds() const {
            return m_worldBounds;
        }

        const IO::Path& MapDocument::path() const {
            return m_path;
        }
        
        String MapDocument::filename() const {
            if (m_path.isEmpty())
                return "";
            return  m_path.lastComponent();
        }
        
        Model::GamePtr MapDocument::game() const {
            return m_game;
        }
        
        Model::Map* MapDocument::map() const {
            return m_map;
        }

        const Model::Filter& MapDocument::filter() const {
            return m_filter;
        }

        Assets::EntityDefinitionManager& MapDocument::entityDefinitionManager() {
            return m_entityDefinitionManager;
        }
        
        Assets::EntityModelManager& MapDocument::entityModelManager() {
            return m_entityModelManager;
        }
        
        Assets::TextureManager& MapDocument::textureManager() {
            return m_textureManager;
        }
        
        View::Grid& MapDocument::grid() {
            return m_grid;
        }

        bool MapDocument::modified() const {
            return m_modificationCount > 0;
        }

        void MapDocument::incModificationCount() {
            ++m_modificationCount;
        }
        
        void MapDocument::decModificationCount() {
            assert(m_modificationCount > 0);
            --m_modificationCount;
        }

        void MapDocument::clearModificationCount() {
            m_modificationCount = 0;
        }

        void MapDocument::newDocument(const BBox3& worldBounds, Model::GamePtr game) {
            assert(game != NULL);
            info("Creating new document");

            m_worldBounds = worldBounds;
            m_game = game;
            delete m_map;
            m_map = game->newMap();

            m_selection = Model::Selection(m_map);
            m_entityDefinitionManager.clear();
            m_entityModelManager.reset(m_game);
            m_textureManager.reset(m_game);
            m_picker = Model::Picker(m_worldBounds);
            
            setDocumentPath(IO::Path("unnamed.map"));
            clearModificationCount();
            loadAndUpdateEntityDefinitions();
        }
        
        void MapDocument::openDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path) {
            assert(game != NULL);
            info("Opening document document " + path.asString());

            m_worldBounds = worldBounds;
            m_game = game;
            delete m_map;
            m_map = m_game->loadMap(worldBounds, path);
            
            m_selection = Model::Selection(m_map);
            m_entityDefinitionManager.clear();
            m_entityModelManager.reset(m_game);
            m_textureManager.reset(m_game);
            m_picker = Model::Picker(m_worldBounds);
            
            setDocumentPath(path);
            clearModificationCount();
            loadAndUpdateEntityDefinitions();
            loadAndUpdateTextures();
            
            Model::each(Model::MapObjectsIterator::begin(*m_map),
                        Model::MapObjectsIterator::end(*m_map),
                        AddToPicker(m_picker),
                        Model::MatchAll());
        }

        void MapDocument::saveDocument() {
            assert(!m_path.isEmpty());
            doSaveDocument(m_path);
        }
        
        void MapDocument::saveDocumentAs(const IO::Path& path) {
            doSaveDocument(path);
        }
        
        void MapDocument::addEntity(Model::Entity* entity) {
            SetEntityDefinition setDefinition(m_entityDefinitionManager);
            SetEntityModel setModel(m_entityModelManager);
            AddToMap addToMap(*m_map);
            AddToPicker addToPicker(m_picker);
            SetFaceTexture setTexture(m_textureManager);
            
            setDefinition(entity);
            setModel(entity);
            addToMap(entity);
            addToPicker(entity);
            
            Model::each(entity->brushes().begin(),
                        entity->brushes().end(),
                        addToPicker,
                        Model::MatchAll());
            Model::each(Model::BrushFacesIterator::begin(entity->brushes()),
                        Model::BrushFacesIterator::end(entity->brushes()),
                        setTexture, Model::MatchAll());
        }
        
        void MapDocument::addWorldBrush(Model::Brush* brush) {
            addBrush(worldspawn(), brush);
        }
        
        void MapDocument::addBrush(Model::Entity* entity, Model::Brush* brush) {
            AddToEntity addToEntity(*entity);
            AddToPicker addToPicker(m_picker);
            RemoveFromPicker removeFromPicker(m_picker);
            SetFaceTexture setTexture(m_textureManager);
            
            removeFromPicker(entity);
            addToEntity(brush);
            addToPicker(brush);
            Model::each(brush->faces().begin(),
                        brush->faces().end(),
                        setTexture,
                        Model::MatchAll());
            addToPicker(entity);
        }

        void MapDocument::removeEntity(Model::Entity* entity) {
            assert(!entity->worldspawn());
            
            UnsetEntityDefinition unsetDefinition;
            UnsetEntityModel unsetModel;
            RemoveFromMap removeFromMap(*m_map);
            RemoveFromPicker removeFromPicker(m_picker);
            UnsetFaceTexture unsetTexture;
            
            unsetDefinition(entity);
            unsetModel(entity);
            removeFromMap(entity);
            removeFromPicker(entity);
            
            Model::each(entity->brushes().begin(),
                        entity->brushes().end(),
                        removeFromPicker,
                        Model::MatchAll());
            Model::each(Model::BrushFacesIterator::begin(entity->brushes()),
                        Model::BrushFacesIterator::end(entity->brushes()),
                        unsetTexture, Model::MatchAll());
        }
        
        void MapDocument::removeWorldBrush(Model::Brush* brush) {
            removeBrush(worldspawn(), brush);
        }

        void MapDocument::removeBrush(Model::Entity* entity, Model::Brush* brush) {
            RemoveFromEntity removeFromEntity(*entity);
            RemoveFromPicker removeFromPicker(m_picker);
            UnsetFaceTexture unsetTexture;
            AddToPicker addToPicker(m_picker);
            
            removeFromPicker(entity);
            removeFromEntity(brush);
            removeFromPicker(brush);
            Model::each(brush->faces().begin(),
                        brush->faces().end(),
                        unsetTexture,
                        Model::MatchAll());
            addToPicker(entity);
        }

        bool MapDocument::hasSelectedObjects() const {
            return m_selection.hasSelectedObjects();
        }
        
        bool MapDocument::hasSelectedFaces() const {
            return m_selection.hasSelectedFaces();
        }
        
        bool MapDocument::hasSelection() const {
            return m_selection.hasSelection();
        }

        Model::ObjectList MapDocument::selectedObjects() const {
            return m_selection.selectedObjects();
        }

        Model::EntityList MapDocument::selectedEntities() const {
            return m_selection.selectedEntities();
        }
        
        Model::EntityList MapDocument::allSelectedEntities() const {
            return m_selection.allSelectedEntities();
        }

        Model::EntityList MapDocument::unselectedEntities() const {
            return m_selection.unselectedEntities();
        }

        Model::BrushList MapDocument::selectedBrushes() const {
            return m_selection.selectedBrushes();
        }
        
        Model::BrushList MapDocument::unselectedBrushes() const {
            return m_selection.unselectedBrushes();
        }

        Model::BrushFaceList MapDocument::selectedFaces() const {
            return m_selection.selectedFaces();
        }
        
        Model::SelectionResult MapDocument::selectObjects(const Model::ObjectList& objects) {
            return m_selection.selectObjects(objects);
        }
        
        Model::SelectionResult MapDocument::deselectObjects(const Model::ObjectList& objects) {
            return m_selection.deselectObjects(objects);
        }
        
        Model::SelectionResult MapDocument::selectAllObjects() {
            return m_selection.selectAllObjects();
        }
        
        Model::SelectionResult MapDocument::selectFaces(const Model::BrushFaceList& faces) {
            return m_selection.selectFaces(faces);
        }
        
        Model::SelectionResult MapDocument::deselectFaces(const Model::BrushFaceList& faces) {
            return m_selection.deselectFaces(faces);
        }

        Model::SelectionResult MapDocument::deselectAll() {
            return m_selection.deselectAll();
        }

        Assets::FaceTexture* MapDocument::currentTexture() const {
            if (m_selection.lastSelectedFace() == NULL)
                return NULL;
            return m_selection.lastSelectedFace()->texture();
        }

        String MapDocument::currentTextureName() const {
            if (currentTexture() != NULL)
                return currentTexture()->name();
            return Model::BrushFace::NoTextureName;
        }

        void MapDocument::commitPendingRenderStateChanges() {
            m_textureManager.commitChanges();
        }

        void MapDocument::commandDo(Controller::Command::Ptr command) {
            m_picker.removeObjects(command->affectedObjects());
        }
        
        void MapDocument::commandDone(Controller::Command::Ptr command) {
            m_picker.addObjects(command->affectedObjects());
            
            if (command->type() == Controller::EntityPropertyCommand::Type) {
                const Model::EntityList entities = command->affectedEntities();
                updateEntityDefinitions(entities);
                updateEntityModels(entities);
            }
        }
        
        void MapDocument::commandDoFailed(Controller::Command::Ptr command) {
            m_picker.addObjects(command->affectedObjects());
        }
        
        void MapDocument::commandUndo(Controller::Command::Ptr command) {
            m_picker.removeObjects(command->affectedObjects());
        }
        
        void MapDocument::commandUndone(Controller::Command::Ptr command) {
            m_picker.addObjects(command->affectedObjects());

            if (command->type() == Controller::EntityPropertyCommand::Type) {
                const Model::EntityList entities = command->affectedEntities();
                updateEntityDefinitions(entities);
                updateEntityModels(entities);
            }
        }
        
        void MapDocument::commandUndoFailed(Controller::Command::Ptr command) {
            m_picker.addObjects(command->affectedObjects());
        }

        Model::PickResult MapDocument::pick(const Ray3& ray) {
            return m_picker.pick(ray);
        }

        MapDocument::MapDocument() :
        CachingLogger(),
        m_worldBounds(DefaultWorldBounds),
        m_path(""),
        m_map(NULL),
        m_picker(m_worldBounds),
        m_grid(5),
        m_modificationCount(0) {}
        
        void MapDocument::loadAndUpdateEntityDefinitions() {
            loadEntityDefinitions();
            updateEntityDefinitions(m_map->entities());
            updateEntityModels(m_map->entities());
        }
        
        void MapDocument::loadEntityDefinitions() {
            const IO::Path path = m_game->extractEntityDefinitionFile(m_map);
            m_entityDefinitionManager.loadDefinitions(m_game, path);
            info("Loaded entity definition file " + path.asString());
        }
        
        void MapDocument::updateEntityDefinitions(const Model::EntityList& entities) {
            Model::each(entities.begin(),
                        entities.end(),
                        SetEntityDefinition(m_entityDefinitionManager),
                        Model::MatchAll());
        }

        void MapDocument::updateEntityModels(const Model::EntityList& entities) {
            Model::each(entities.begin(),
                        entities.end(),
                        SetEntityModel(m_entityModelManager),
                        Model::MatchAll());
        }

        void MapDocument::loadAndUpdateTextures() {
            loadTextures();
            updateTextures();
        }

        void MapDocument::loadTextures() {
            IO::FileSystem fs;
            
            IO::Path::List rootPaths;
            rootPaths.push_back(fs.appDirectory());
            if (m_path.isAbsolute())
                rootPaths.push_back(m_path.deleteLastComponent());
            
            const IO::Path::List wadPaths = m_game->extractTexturePaths(m_map);
            IO::Path::List::const_iterator it, end;
            for (it = wadPaths.begin(), end = wadPaths.end(); it != end; ++it) {
                const IO::Path& wadPath = *it;
                try {
                    const IO::Path path = fs.resolvePath(rootPaths, *it);
                    m_textureManager.addTextureCollection(path);
                    info("Loaded texture collection " + wadPath.asString());
                } catch (Exception e) {
                    error("Error loading texture collection " + wadPath.asString() + ": " + e.what());
                }
            }
        }

        void MapDocument::updateTextures() {
            Model::each(Model::MapFacesIterator::begin(*m_map),
                        Model::MapFacesIterator::end(*m_map),
                        SetFaceTexture(m_textureManager),
                        Model::MatchAll());
        }

        Model::Entity* MapDocument::worldspawn() {
            Model::Entity* worldspawn = m_map->worldspawn();
            if (worldspawn == NULL) {
                worldspawn = m_map->createEntity();
                worldspawn->addOrUpdateProperty(Model::PropertyKeys::Classname, Model::PropertyValues::WorldspawnClassname);
                addEntity(worldspawn);
            }
            return worldspawn;
        }

        void MapDocument::doSaveDocument(const IO::Path& path) {
            setDocumentPath(path);
        }

        void MapDocument::setDocumentPath(const IO::Path& path) {
            m_path = path;
        }
    }
}
