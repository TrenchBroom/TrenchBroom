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

#ifndef TrenchBroom_CreateBrushFigure_h
#define TrenchBroom_CreateBrushFigure_h

#include "Controller/CreateBrushTool.h"
#include "Renderer/Figures/Figure.h"
#include "Utilities/Event.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
    }
    
    namespace Renderer {
        class BrushFigure;
        class FontManager;
        class RenderContext;
        class SizeGuideFigure;
        class Vbo;
        
        class CreateBrushFigure : public Figure {
        protected:
            Controller::CreateBrushTool& m_createBrushTool;

            Model::Brush* m_brush;
            bool m_valid;
            BrushFigure* m_brushFigure;
            SizeGuideFigure* m_sizeGuideFigure;
            
            FontManager& m_fontManager;
            
            typedef Controller::CreateBrushTool::CreateBrushToolEvent::Listener<CreateBrushFigure> Listener;
            
            void createBrush(Model::Brush& brush);
            void modifyBrush(Model::Brush& brush);
            void finishBrush(Model::Brush& brush);
        public:
            CreateBrushFigure(Controller::CreateBrushTool& createBrushTool, FontManager& fontManager);
            ~CreateBrushFigure();
            
            virtual void render(RenderContext& context, Vbo& vbo);
        };
    }
}

#endif
