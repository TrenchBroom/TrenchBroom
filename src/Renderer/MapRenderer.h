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
#include "Model/ModelTypes.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Model {
        class Filter;
        class Map;
    }
    
    namespace Renderer {
        class FontManager;
        class RenderContext;
        
        class MapRenderer {
        private:
            FontManager& m_fontManager;
            const Model::Filter& m_filter;
            Vbo m_auxVbo;
            BrushRenderer m_unselectedBrushRenderer;
            BrushRenderer m_selectedBrushRenderer;
            EntityRenderer m_unselectedEntityRenderer;
            EntityRenderer m_selectedEntityRenderer;
            Color m_focusColor;
            bool m_hasFocus;
        public:
            MapRenderer(FontManager& fontManager, const Model::Filter& filter);
            
            void render(RenderContext& context);
            void setHasFocus(const bool hasFocus);
            void setFocusColor(const Color& focusColor);

            void commandDone(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);
        private:
            void setupGL(RenderContext& context);
            void clearBackground(RenderContext& context);
            void renderCoordinateSystem(RenderContext& context);
            void renderGeometry(RenderContext& context);
            void renderEntities(RenderContext& context);
            void renderFocusRect(RenderContext& context);
            void clearState();
            void loadMap(Model::Map& map);
            void updateSelection(Controller::Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
