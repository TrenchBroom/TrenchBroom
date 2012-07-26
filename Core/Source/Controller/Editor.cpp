/*
 Copyright (C) 2010-2012 Kristian Duske

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

#include "Editor.h"

#include "Controller/Autosaver.h"
#include "Controller/Camera.h"
#include "Controller/Grid.h"
#include "Controller/InputController.h"
#include "Controller/Options.h"
#include "Controller/ProgressIndicator.h"
#include "IO/FileManager.h"
#include "IO/MapParser.h"
#include "IO/MapWriter.h"
#include "IO/Wad.h"
#include "Model/Map/Map.h"
#include "Model/Map/Entity.h"
#include "Model/Preferences.h"
#include "Model/Undo/UndoManager.h"
#include "Utilities/Filter.h"
#include "Utilities/Utils.h"
#include "Utilities/Console.h"

#include <ctime>

namespace TrenchBroom {
    namespace Controller {
        void Editor::loadTextureWad(const std::string& path) {
            IO::FileManager& fileManager = *IO::FileManager::sharedFileManager;
            
            std::string wadPath = path;
            if (!fileManager.exists(wadPath) && !m_mapPath.empty()) {
                std::string folderPath = fileManager.deleteLastPathComponent(m_mapPath);
                wadPath = fileManager.appendPath(folderPath, wadPath);
            }
            
            if (fileManager.exists(wadPath)) {
                clock_t start = clock();
                IO::Wad wad(wadPath);
                Model::Assets::TextureCollection* collection = new Model::Assets::TextureCollection(wadPath, wad, *m_palette);
                unsigned int index = static_cast<unsigned int>(m_textureManager->collections().size());
                m_textureManager->addCollection(collection, index);
                log(TB_LL_INFO, "Loaded %s in %f seconds\n", wadPath.c_str(), (clock() - start) / CLOCKS_PER_SEC / 10000.0f);
            } else {
                log(TB_LL_WARN, "Could not open texture wad %s\n", path.c_str());
            }
        }

        void Editor::updateFaceTextures() {
            std::vector<Model::Face*> changedFaces;
            std::vector<Model::Assets::Texture*> newTextures;

            const std::vector<Model::Entity*>& entities = m_map->entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                const std::vector<Model::Brush*>& brushes = entities[i]->brushes();
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    const std::vector<Model::Face*>& faces = brushes[j]->faces;
                    for (unsigned int k = 0; k < faces.size(); k++) {
                        const std::string& textureName = faces[k]->textureName;
                        Model::Assets::Texture* oldTexture = faces[k]->texture;
                        Model::Assets::Texture* newTexture = m_textureManager->texture(textureName);
                        if (oldTexture != newTexture) {
                            changedFaces.push_back(faces[k]);
                            newTextures.push_back(newTexture);
                        }
                    }
                }
            }

            if (!changedFaces.empty()) {
                m_map->facesWillChange(changedFaces);
                for (unsigned int i = 0; i < changedFaces.size(); i++)
                    changedFaces[i]->setTexture(newTextures[i]);
                m_map->facesDidChange(changedFaces);
            }
        }

        void Editor::updateWadProperty() {
            Model::Selection& selection = m_map->selection();
            selection.push();
            selection.addEntity(*m_map->worldspawn(true));
            
            std::vector<Model::Assets::TextureCollection*> collections = m_textureManager->collections();
            if (collections.empty()) {
                m_map->setEntityProperty(Model::WadKey, static_cast<std::string*>(NULL));
            } else {
                std::stringstream value;
                for (unsigned int i = 0; i < collections.size(); i++) {
                    value << collections[i]->name();
                    if (i < collections.size() - 1)
                        value << ";";
                }
                
                m_map->setEntityProperty(Model::WadKey, value.str());
            }
            
            selection.pop();
        }

        void Editor::textureManagerDidChange(Model::Assets::TextureManager& textureManager) {
            updateFaceTextures();
        }

        void Editor::preferencesDidChange(const std::string& key) {
            m_camera->setFieldOfVision(Model::Preferences::sharedPreferences->cameraFov());
            m_camera->setNearPlane(Model::Preferences::sharedPreferences->cameraNear());
            m_camera->setFarPlane(Model::Preferences::sharedPreferences->cameraFar());
        }

        void Editor::undoGroupCreated(const Model::UndoGroup& group) {
            m_autosaver->updateLastModificationTime();
        }

        void Editor::selectionDidChange(const Model::SelectionEventData& data) {
            Model::Selection& selection = m_map->selection();
            if (selection.mode() == Model::TB_SM_FACES || selection.mode() == Model::TB_SM_NONE) {
                if (m_inputController->moveVertexToolActive())
                    m_inputController->toggleMoveVertexTool();
                else if (m_inputController->moveEdgeToolActive())
                    m_inputController->toggleMoveEdgeTool();
                else if (m_inputController->moveFaceToolActive())
                    m_inputController->toggleMoveFaceTool();
                else if (m_inputController->clipToolActive())
                    m_inputController->toggleClipTool();
            }
        }

        Editor::Editor(const std::string& entityDefinitionFilePath, const std::string& palettePath) : m_entityDefinitionFilePath(entityDefinitionFilePath), m_renderer(NULL) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;

            m_textureManager = new Model::Assets::TextureManager();
            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));
            m_map = new Model::Map(worldBounds, m_entityDefinitionFilePath);
            m_camera = new Camera(prefs.cameraFov(), prefs.cameraNear(), prefs.cameraFar(),
                                  Vec3f(-32, -32, 32), Vec3f::PosX);
            m_grid = new Grid(5);
            m_inputController = new InputController(*this);

            m_palette = new Model::Assets::Palette(palettePath);
            m_options = new TransientOptions();
            m_filter = new Filter(*this);
            m_autosaver = new Autosaver(*this);

            Model::Preferences::sharedPreferences->preferencesDidChange += new Model::Preferences::PreferencesEvent::Listener<Editor>(this, &Editor::preferencesDidChange);
            m_textureManager->textureManagerDidChange += new Model::Assets::TextureManager::TextureManagerEvent::Listener<Editor>(this, &Editor::textureManagerDidChange);

            m_map->undoManager().undoGroupCreated   += new Model::UndoManager::UndoEvent::Listener<Editor>(this, &Editor::undoGroupCreated);
            m_map->selection().selectionAdded       += new Model::Selection::SelectionEvent::Listener<Editor>(this, &Editor::selectionDidChange);
            m_map->selection().selectionRemoved     += new Model::Selection::SelectionEvent::Listener<Editor>(this, &Editor::selectionDidChange);
        }

        Editor::~Editor() {
            m_map->undoManager().undoGroupCreated  -= new Model::UndoManager::UndoEvent::Listener<Editor>(this, &Editor::undoGroupCreated);

            delete m_autosaver;
            
            Model::Preferences::sharedPreferences->preferencesDidChange -= new Model::Preferences::PreferencesEvent::Listener<Editor>(this, &Editor::preferencesDidChange);
            m_textureManager->textureManagerDidChange -= new Model::Assets::TextureManager::TextureManagerEvent::Listener<Editor>(this, &Editor::textureManagerDidChange);

            delete m_inputController;
            delete m_camera;
            delete m_map;
            delete m_grid;
            delete m_textureManager;
            delete m_palette;
            delete m_options;
            delete m_filter;
        }

		void Editor::loadMap(const std::string& path, ProgressIndicator* indicator) {
			indicator->setText("Clearing map...");
			clear();

			indicator->setText("Loading map file...");
			m_map->setPostNotifications(false);
            m_mapPath = path;

            clock_t start = clock();
            std::ifstream stream(path.c_str());
            IO::MapParser parser(stream);
            parser.parseMap(*m_map, indicator);
            log(TB_LL_INFO, "Loaded %s in %f seconds\n", path.c_str(), (clock() - start) / CLOCKS_PER_SEC / 10000.0f);

            indicator->setText("Loading wad files...");

            // load wad files
            const std::string* wads = m_map->worldspawn(true)->propertyForKey(Model::WadKey);
            if (wads != NULL) {
                std::vector<std::string> wadPaths = split(*wads, ';');
                for (unsigned int i = 0; i < wadPaths.size(); i++) {
                    std::string wadPath = trim(wadPaths[i]);
                    loadTextureWad(wadPath);
                }
            }

            updateFaceTextures();
			m_map->setPostNotifications(true);

			m_map->mapLoaded(*m_map);
            m_autosaver->clearDirtyFlag();
        }

        void Editor::saveMap(const std::string& path) {
            IO::MapWriter mapWriter(*m_map);
            mapWriter.writeToFileAtPath(path, true);
            m_autosaver->clearDirtyFlag();
        }

		void Editor::clear() {
			m_map->clear();
			m_textureManager->clear();
			m_mapPath = "";
			m_autosaver->clearDirtyFlag();
		}

        void Editor::addTextureWad(const std::string& path) {
            loadTextureWad(path);
            updateWadProperty();
        }
        
        void Editor::removeTextureWad(unsigned int index) {
            m_textureManager->removeCollection(index);
            updateWadProperty();
        }
        
        const std::string& Editor::mapPath() const {
            return m_mapPath;
        }

        Model::Map& Editor::map() const {
            return *m_map;
        }

        Camera& Editor::camera() const {
            return *m_camera;
        }

        Grid& Editor::grid() const {
            return *m_grid;
        }

        InputController& Editor::inputController() const {
            return *m_inputController;
        }

        TransientOptions& Editor::options() const {
            return *m_options;
        }

        Filter& Editor::filter() const {
            return *m_filter;
        }
        
        Autosaver& Editor::autosaver() const {
            return *m_autosaver;
        }

        Model::Assets::Palette& Editor::palette() const {
            return *m_palette;
        }

        Model::Assets::TextureManager& Editor::textureManager() const {
            return *m_textureManager;
        }

        void Editor::setRenderer(Renderer::MapRenderer* renderer) {
            m_renderer = renderer;
        }

        Renderer::MapRenderer* Editor::renderer() const {
            return m_renderer;
        }

        void Editor::undo() {
            m_map->undoManager().undo();
        }
        
        void Editor::redo() {
            m_map->undoManager().redo();
        }
        
        void Editor::selectAll() {
            Model::Selection& selection = m_map->selection();
            selection.removeAll();

            const Model::EntityList& entities = m_map->entities();
            Model::BrushList brushes;
            for (unsigned int i = 0; i < entities.size(); i++)
                brushes.insert(brushes.begin(), entities[i]->brushes().begin(), entities[i]->brushes().end());
            if (!brushes.empty())
                selection.addBrushes(brushes);
            if (!entities.empty())
                selection.addEntities(entities);
        }
        
        void Editor::selectEntities() {
            Model::Selection& selection = m_map->selection();

            if (selection.mode() == Model::TB_SM_BRUSHES) {
                Model::EntitySet entitySet;
                const Model::BrushList& selectedBrushes = selection.brushes();
                for (unsigned int i = 0; i < selectedBrushes.size(); i++)
                    entitySet.insert(selectedBrushes[i]->entity);
                
                Model::EntityList entityList;
                entityList.insert(entityList.begin(), entitySet.begin(), entitySet.end());
                
                Model::BrushList brushList;
                for (unsigned int i = 0; i < entityList.size(); i++) {
                    Model::Entity* entity = entityList[i];
                    brushList.insert(brushList.end(), entity->brushes().begin(), entity->brushes().end());
                }
                
                selection.removeAll();
                selection.addEntities(entityList);
                selection.addBrushes(brushList);
            }
        }
        
        void Editor::selectTouching(bool deleteBrush) {
            Model::Selection& selection = m_map->selection();
            
            if (selection.mode() == Model::TB_SM_BRUSHES && selection.brushes().size() == 1) {
                Model::Brush* selectionBrush = selection.brushes().front();
                
                Model::EntityList selectedEntities;
                Model::BrushList selectedBrushes;
                
                const Model::EntityList& entities = m_map->entities();
                for (unsigned int i = 0; i < entities.size(); i++) {
                    Model::Entity* entity = entities[i];
                    if (entity->entityDefinition()->type == Model::TB_EDT_POINT && selectionBrush->intersectsEntity(*entity)) {
                        selectedEntities.push_back(entity);
                    } else {
                        const Model::BrushList entityBrushes = entity->brushes();
                        for (unsigned int j = 0; j < entityBrushes.size(); j++) {
                            Model::Brush* brush = entityBrushes[j];
                            if (selectionBrush->intersectsBrush(*brush) && brush != selectionBrush)
                                selectedBrushes.push_back(brush);
                        }
                    }
                }
                
                if (deleteBrush)
                    m_map->deleteObjects();
                
                selection.addEntities(selectedEntities);
                selection.addBrushes(selectedBrushes);
            }
        }
        
        void Editor::selectNone() {
            Model::Selection& selection = m_map->selection();
            selection.removeAll();
        }
        
        void Editor::moveTextures(EMoveDirection direction, bool disableSnapToGrid) {
            Vec3f moveDirection;
            switch (direction) {
                case LEFT:
                    moveDirection = m_camera->right() * -1.0f;
                    break;
                case UP:
                    moveDirection = m_camera->up();
                    break;
                case RIGHT:
                    moveDirection = m_camera->right();
                    break;
                case DOWN:
                    moveDirection = m_camera->up() * -1.0f;
                    break;
                default:
                    assert(false);
            }
            
            float delta = disableSnapToGrid ? 1.0f : static_cast<float>(m_grid->actualSize());
            m_map->translateFaces(delta, moveDirection);
        }
        
        void Editor::rotateTextures(bool clockwise, bool disableSnapToGrid) {
            float angle = disableSnapToGrid ? 1.0f : m_grid->angle();
			if (clockwise)
				angle *= -1.0f;
            m_map->rotateFaces(angle);
        }
        
        void Editor::moveObjects(EMoveDirection direction, bool disableSnapToGrid) {
            Vec3f moveDirection;
            switch (direction) {
                case LEFT:
                    moveDirection = (m_camera->right() * -1.0f).firstAxis();
                    break;
                case UP:
                    moveDirection = Vec3f::PosZ;
                    break;
                case RIGHT:
                    moveDirection = m_camera->right().firstAxis();
                    break;
                case DOWN:
                    moveDirection = Vec3f::NegZ;
                    break;
                case FORWARD:
                    moveDirection = m_camera->direction().firstAxis();
                    if (moveDirection.firstComponent() == TB_AX_Z)
                        moveDirection = m_camera->direction().secondAxis();
                    break;
                case BACKWARD:
                    moveDirection = (m_camera->direction() * -1.0f).firstAxis();
                    if (moveDirection.firstComponent() == TB_AX_Z)
                        moveDirection = (m_camera->direction() * -1.0f).secondAxis();
                    break;
            }

            Model::Selection& selection = m_map->selection();
            
            float dist = disableSnapToGrid ? 1.0f : static_cast<float>(m_grid->actualSize());
            Vec3f delta = m_grid->moveDelta(selection.bounds(), m_map->worldBounds(), moveDirection * dist);
            
            m_map->translateObjects(delta, true);
        }
        
        void Editor::rotateObjects(ERotationAxis axis, bool clockwise) {
            EAxis absoluteAxis;
            switch (axis) {
                case ROLL:
                    absoluteAxis = m_camera->direction().firstComponent();
                    break;
                case PITCH:
                    absoluteAxis = m_camera->right().firstComponent();
                    break;
                case YAW:
                    absoluteAxis = TB_AX_Z;
                    break;
            }
            
            Model::Selection& selection = m_map->selection();
            m_map->rotateObjects90(absoluteAxis, selection.center(), clockwise, true);
        }
        
        void Editor::flipObjects(bool horizontally) {
            EAxis axis = horizontally ? m_camera->right().firstComponent() : TB_AX_Z;
            Model::Selection& selection = m_map->selection();
            m_map->flipObjects(axis, selection.center(), true);
        }
        
        void Editor::duplicateObjects() {
        }
        
        void Editor::enlargeBrushes() {
        }
        
        void Editor::toggleGrid() {
            m_grid->toggleVisible();
        }
        
        void Editor::toggleSnapToGrid() {
            m_grid->toggleSnap();
        }
        
        void Editor::setGridSize(int size) {
            m_grid->setSize(size);
        }

        void Editor::moveCamera(EMoveDirection direction, bool disableSnapToGrid) {
            float delta = disableSnapToGrid ? 1.0f : 16.0f;
            switch (direction) {
                case LEFT:
                    m_camera->moveBy(0.0f, -delta, 0.0f);
                    break;
                case UP:
                    m_camera->moveBy(0.0f, 0.0f, delta);
                    break;
                case RIGHT:
                    m_camera->moveBy(0.0f, delta, 0.0f);
                    break;
                case DOWN:
                    m_camera->moveBy(0.0f, 0.0f, -delta);
                    break;
                case FORWARD:
                    m_camera->moveBy(delta, 0.0f, 0.0f);
                    break;
                case BACKWARD:
                    m_camera->moveBy(-delta, 0.0f, 0.0f);
                    break;
            }
        }

        void Editor::toggleIsolateSelection() {
            switch (m_options->isolationMode()) {
                case IM_NONE:
                    m_options->setIsolationMode(IM_WIREFRAME);
                    break;
                case IM_WIREFRAME:
                    m_options->setIsolationMode(IM_DISCARD);
                    break;
                default:
                    m_options->setIsolationMode(IM_NONE);
                    break;
            }
        }
    }
}
