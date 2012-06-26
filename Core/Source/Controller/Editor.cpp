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
#include <ctime>
#include "Controller/Camera.h"
#include "Controller/Grid.h"
#include "Controller/InputController.h"
#include "Controller/Options.h"
#include "Controller/ProgressIndicator.h"
#include "Model/Map/Map.h"
#include "Model/Map/Entity.h"
#include "Model/Preferences.h"
#include "IO/FileManager.h"
#include "IO/MapParser.h"
#include "IO/Wad.h"
#include "Utilities/Filter.h"
#include "Utilities/Utils.h"
#include "Utilities/VecMath.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace Controller {
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

        void Editor::textureManagerDidChange(Model::Assets::TextureManager& textureManager) {
            updateFaceTextures();
        }

        void Editor::preferencesDidChange(const std::string& key) {
            m_camera->setFieldOfVision(Model::Preferences::sharedPreferences->cameraFov());
            m_camera->setNearPlane(Model::Preferences::sharedPreferences->cameraNear());
            m_camera->setFarPlane(Model::Preferences::sharedPreferences->cameraFar());
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
            m_filter = new Filter();

            Model::Preferences::sharedPreferences->preferencesDidChange += new Model::Preferences::PreferencesEvent::Listener<Editor>(this, &Editor::preferencesDidChange);
            m_textureManager->textureManagerDidChange += new Model::Assets::TextureManager::TextureManagerEvent::Listener<Editor>(this, &Editor::textureManagerDidChange);
        }

        Editor::~Editor() {
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
            m_map->clear();
            m_textureManager->clear();
			m_map->setPostNotifications(false);

			indicator->setText("Loading map file...");
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
        }

        void Editor::saveMap(const std::string& path) {
        }

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
                m_textureManager->addCollection(collection, m_textureManager->collections().size());
                log(TB_LL_INFO, "Loaded %s in %f seconds\n", wadPath.c_str(), (clock() - start) / CLOCKS_PER_SEC / 10000.0f);
            } else {
                log(TB_LL_WARN, "Could not open texture wad %s\n", path.c_str());
            }
        }

        Model::Map& Editor::map() {
            return *m_map;
        }

        Camera& Editor::camera() {
            return *m_camera;
        }

        Grid& Editor::grid() {
            return *m_grid;
        }

        InputController& Editor::inputController() {
            return *m_inputController;
        }

        TransientOptions& Editor::options() {
            return *m_options;
        }

        Filter& Editor::filter() {
            return *m_filter;
        }

        Model::Assets::Palette& Editor::palette() {
            return *m_palette;
        }

        Model::Assets::TextureManager& Editor::textureManager() {
            return *m_textureManager;
        }

        void Editor::setRenderer(Renderer::MapRenderer* renderer) {
            m_renderer = renderer;
        }

        Renderer::MapRenderer* Editor::renderer() {
            return m_renderer;
        }
    }
}
