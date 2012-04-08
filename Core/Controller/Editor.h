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

#include "Camera.h"
#include "InputController.h"
#include "Map.h"
#include "Texture.h"
#include <string>

namespace TrenchBroom {
    namespace Controller {
        class InputController;
        class Editor {
        private:
            Model::Map* m_map;
            Model::Camera* m_camera;
            InputController* m_inputController;
            Model::Assets::TextureManager* m_textureManager;
            Model::Assets::Palette* m_palette;
            string m_entityDefinitionFilePath;
        public:
            Editor(const string& entityDefinitionFilePath, const string& palettePath);
            ~Editor();
            
            void loadMap(const string& path);
            void saveMap(const string& path);
            
            Model::Camera& camera();
            InputController& inputController();
        };
    }
}

#endif
