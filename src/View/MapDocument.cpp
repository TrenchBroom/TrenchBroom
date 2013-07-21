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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapDocument.h"

#include "IO/FileSystem.h"
#include "Model/BrushFace.h"
#include "Model/Game.h"
#include "Model/Map.h"
#include "View/Logger.h"
#include "View/MapFrame.h"

#include <cassert>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>

namespace TrenchBroom {
    namespace View {
        class SetEntityDefinition {
        private:
            Model::EntityDefinitionManager& m_definitionManager;
        public:
            SetEntityDefinition(Model::EntityDefinitionManager& definitionManager) :
            m_definitionManager(definitionManager) {}
            
            inline void operator()(Model::Entity* entity) const {
                Model::EntityDefinition* definition = m_definitionManager.definition(entity);
                entity->setDefinition(definition);
            }
        };
        
        class SetFaceTexture {
        private:
            Model::TextureManager& m_textureManager;
        public:
            SetFaceTexture(Model::TextureManager& textureManager) :
            m_textureManager(textureManager) {}
            
            inline void operator()(Model::Brush* brush, Model::BrushFace* face) const {
                const String& textureName = face->textureName();
                Model::Texture* texture = m_textureManager.texture(textureName);
                face->setTexture(texture);
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
        
        struct MatchAllFilter {
            inline bool operator()(Model::Entity* entity) const {
                return true;
            }
            
            inline bool operator()(Model::Brush* brush) const {
                return true;
            }
            
            inline bool operator()(Model::Brush* brush, Model::BrushFace* face) const {
                return true;
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
            m_map = new Model::Map();

            m_selection = Model::Selection(m_map);
            m_entityDefinitionManager.clear();
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
            m_textureManager.reset(m_game);
            m_picker = Model::Picker(m_worldBounds);
            m_map->eachObject(AddToPicker(m_picker), MatchAllFilter());
            
            setDocumentPath(path);
            clearModificationCount();
            loadAndUpdateEntityDefinitions();
            loadAndUpdateTextures();
        }

        void MapDocument::saveDocument() {
            assert(!m_path.isEmpty());
            doSaveDocument(m_path);
        }
        
        void MapDocument::saveDocumentAs(const IO::Path& path) {
            doSaveDocument(path);
        }
        
        Model::ObjectList MapDocument::selectedObjects() const {
            return m_selection.selectedObjects();
        }

        Model::EntityList MapDocument::selectedEntities() const {
            return m_selection.selectedEntities();
        }
        
        Model::BrushList MapDocument::selectedBrushes() const {
            return m_selection.selectedBrushes();
        }
        
        Model::BrushFaceList MapDocument::selectedFaces() const {
            return m_selection.selectedFaces();
        }
        
        void MapDocument::selectObjects(const Model::ObjectList& objects) {
            m_selection.selectObjects(objects);
        }
        
        void MapDocument::deselectObjects(const Model::ObjectList& objects) {
            m_selection.deselectObjects(objects);
        }
        
        void MapDocument::selectAllObjects() {
            m_selection.selectAllObjects();
        }
        
        void MapDocument::selectFaces(const Model::BrushFaceList& faces) {
            m_selection.selectFaces(faces);
        }
        
        void MapDocument::deselectFaces(const Model::BrushFaceList& faces) {
            m_selection.deselectFaces(faces);
        }

        void MapDocument::deselectAll() {
            m_selection.deselectAll();
        }

        void MapDocument::commitPendingRenderStateChanges() {
            m_textureManager.commitChanges();
        }

        void MapDocument::commandDo(Controller::Command::Ptr command) {}
        void MapDocument::commandDone(Controller::Command::Ptr command) {}
        void MapDocument::commandDoFailed(Controller::Command::Ptr command) {}
        void MapDocument::commandUndo(Controller::Command::Ptr command) {}
        void MapDocument::commandUndone(Controller::Command::Ptr command) {}
        void MapDocument::commandUndoFailed(Controller::Command::Ptr command) {}

        Model::PickResult MapDocument::pick(const Ray3& ray) {
            return m_picker.pick(ray);
        }

        MapDocument::MapDocument() :
        CachingLogger(),
        m_worldBounds(DefaultWorldBounds),
        m_path(""),
        m_map(NULL),
        m_picker(m_worldBounds),
        m_modificationCount(0) {}
        
        void MapDocument::loadAndUpdateEntityDefinitions() {
            loadEntityDefinitions();
            updateEntityDefinitions();
        }
        
        void MapDocument::loadEntityDefinitions() {
            const IO::Path path = m_game->extractEntityDefinitionFile(m_map);
            m_entityDefinitionManager.loadDefinitions(m_game, path);
            info("Loaded entity definition file " + path.asString());
        }
        
        void MapDocument::updateEntityDefinitions() {
            m_map->eachEntity(SetEntityDefinition(m_entityDefinitionManager), MatchAllFilter());
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
            m_map->eachBrushFace(SetFaceTexture(m_textureManager), MatchAllFilter());
        }

        void MapDocument::doSaveDocument(const IO::Path& path) {
            setDocumentPath(path);
        }

        void MapDocument::setDocumentPath(const IO::Path& path) {
            m_path = path;
        }
    }
}
