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

#ifndef TrenchBroom_RenderContext_h
#define TrenchBroom_RenderContext_h

#include "Model/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        class Camera;
        class Grid;
        class TransientOptions;
    }
    
    class Filter;
    
    namespace Renderer {
        class GridRenderer;
        
        class RenderContext {
        public:
            Controller::Camera& camera;
            Filter& filter;
            Controller::Grid& grid;
            Controller::TransientOptions& options;
            Model::Preferences& preferences;
            GridRenderer& gridRenderer;
            
            RenderContext(Controller::Camera& camera, Filter& filter, Controller::Grid& grid, Controller::TransientOptions& options, GridRenderer& gridRenderer) : camera(camera), filter(filter), grid(grid), options(options), preferences(*Model::Preferences::sharedPreferences), gridRenderer(gridRenderer) {}
        };
    }
}

#endif
