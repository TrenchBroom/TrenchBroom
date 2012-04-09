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
#include "VecMath.h"
#include "Preferences.h"
#include "MapParser.h"
#include "Entity.h"
#include "Utils.h"
#include "Wad.h"

namespace TrenchBroom {
    namespace Controller {
        Editor::Editor(const string& entityDefinitionFilePath, const string& palettePath) : m_entityDefinitionFilePath(entityDefinitionFilePath) {
            Model::Preferences& prefs = Model::Preferences::sharedPreferences();

            m_textureManager = new Model::Assets::TextureManager();
            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));
            m_map = new Model::Map(worldBounds, m_entityDefinitionFilePath);
            m_camera = new Model::Camera(prefs.cameraFov(), prefs.cameraNear(), prefs.cameraFar(), 
                                         Vec3f(-32, -32, 32), XAxisPos);
            m_inputController = new InputController(*this);
            
            m_palette = new Model::Assets::Palette(palettePath);
        }
        
        Editor::~Editor() {
            delete m_inputController;
            delete m_camera;
            delete m_map;
            delete m_textureManager;
            delete m_palette;
        }
        
        void Editor::loadMap(const string& path) {
            m_map->clear();
            delete m_map;
            
            m_textureManager->clear();
            
            clock_t start = clock();
            ifstream stream(path.c_str());
            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));
            IO::MapParser parser(stream, worldBounds, *m_textureManager);
            m_map = parser.parseMap(m_entityDefinitionFilePath);
            fprintf(stdout, "Loaded %s in %f seconds\n", path.c_str(), (clock() - start) / CLK_TCK / 10000.0f);
            
            // load wad files
            const string* wads = m_map->worldspawn(true)->propertyForKey(WadKey);
            if (wads != NULL) {
                vector<string> wadPaths = split(*wads, ';');
                for (int i = 0; i < wadPaths.size(); i++) {
                    string wadPath = trim(wadPaths[i]);
                    start = clock();
                    IO::Wad wad(wadPath);
                    Model::Assets::TextureCollection* collection = new Model::Assets::TextureCollection(wadPath, wad, *m_palette);
                    m_textureManager->addCollection(collection, i);
                    fprintf(stdout, "Loaded %s in %f seconds\n", wadPath.c_str(), (clock() - start) / CLK_TCK / 10000.0f);
                }
                
            }
        }
        
        void Editor::saveMap(const string& path) {
        }
        
        Model::Map& Editor::map() {
            return *m_map;
        }
        
        Model::Camera& Editor::camera() {
            return *m_camera;
        }

        InputController& Editor::inputController() {
            return *m_inputController;
        }

    }
}