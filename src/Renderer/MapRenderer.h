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

#ifndef __TrenchBroom__MapRenderer__
#define __TrenchBroom__MapRenderer__

#include "Controller/Command.h"
#include "Model/Map.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class MapRenderer {
        private:
            Vbo m_auxVbo;
            Vbo m_edgeVbo;
            VertexArray m_edgeArray;
        public:
            MapRenderer();
            
            void render(RenderContext& context);

            void commandDone(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);
        private:
            void setupGL(RenderContext& context);
            void clearBackground(RenderContext& context);
            void renderCoordinateSystem(RenderContext& context);
            void renderEdges(RenderContext& context);
            void clearState();
            void loadMap(Model::Map::Ptr map);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
