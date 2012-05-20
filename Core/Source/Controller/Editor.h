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

#ifndef TrenchBroom_Editor_h
#define TrenchBroom_Editor_h

#include <string>

using namespace std;

namespace TrenchBroom {
    class Filter;
    
    namespace Model {
        namespace Assets {
            class TextureManager;
            class Palette;
        }
        class Map;
    }
    
    namespace Renderer {
        class MapRenderer;
    }
    
    namespace Controller {
        class Camera;
        class Grid;
        class InputController;
        class TransientOptions;
        class ProgressIndicator;

        class Editor {
        private:
            Model::Map* m_map;
            Camera* m_camera;
            Grid* m_grid;
            InputController* m_inputController;
            TransientOptions* m_options;
            Filter* m_filter;
            Model::Assets::TextureManager* m_textureManager;
            Model::Assets::Palette* m_palette;
            Renderer::MapRenderer* m_renderer;
            string m_entityDefinitionFilePath;

            void updateFaceTextures();
            void preferencesDidChange(const string& key);
        public:
            Editor(const string& entityDefinitionFilePath, const string& palettePath);
            ~Editor();
            
            void loadMap(const string& path, ProgressIndicator* indicator);
            void saveMap(const string& path);
            
            Model::Map& map();
            Camera& camera();
            Grid& grid();
            InputController& inputController();
            TransientOptions& options();
            Filter& filter();
            Model::Assets::Palette& palette();
            Model::Assets::TextureManager& textureManager();
            
            void setRenderer(Renderer::MapRenderer* renderer);
            Renderer::MapRenderer* renderer();
        };
    }
}

#endif
