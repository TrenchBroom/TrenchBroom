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


#ifndef TrenchBroom_VertexToolFigure_h
#define TrenchBroom_VertexToolFigure_h

#include "Controller/Tool.h"
#include "Renderer/Figures/Figure.h"

namespace TrenchBroom {
    namespace Controller {
        class VertexTool;
    }
    
    namespace Renderer {
        class HandleFigure;
        class PointGuideFigure;
        
        class VertexToolFigure : public Figure {
        protected:
            Controller::VertexTool& m_vertexTool;
            
            HandleFigure* m_handleFigure;
            HandleFigure* m_selectedHandleFigure;
            PointGuideFigure* m_guideFigure;

            void createFigures();
            void deleteFigures();
        public:
            VertexToolFigure(Controller::VertexTool& vertexTool);
            ~VertexToolFigure();
            
            virtual void render(RenderContext& context, Vbo& vbo);
        };
    }
}

#endif
