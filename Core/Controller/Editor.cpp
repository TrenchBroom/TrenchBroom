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
#include "VecMath.h"
#include "Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        Editor::Editor(const string& entityDefinitionFilePath) {
            Model::Preferences& prefs = Model::Preferences::sharedPreferences();

            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));
            m_map = new Model::Map(worldBounds, entityDefinitionFilePath);
            m_camera = new Model::Camera(prefs.cameraFov(), prefs.cameraNear(), prefs.cameraFar(), 
                                         Vec3f(-32, -32, 32), XAxisPos);
            m_inputController = new InputController(*this);
        }
        
        Editor::~Editor() {
            delete m_inputController;
            delete m_camera;
            delete m_map;
        }
        
        Model::Camera& Editor::camera() {
            return *m_camera;
        }

        InputController& Editor::inputController() {
            return *m_inputController;
        }

    }
}