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

#ifndef TrenchBroom_VertexTool_h
#define TrenchBroom_VertexTool_h

#include "Controller/DragTool.h"
#include "Model/Map/BrushTypes.h"
#include "Model/Map/Picker.h"

#include <string>

namespace TrenchBroom {
    namespace Model {
        class Hit;
        class MoveResult;
        class SelectionEventData;
    }
    
    namespace Renderer {
        class HandleFigure;
        class PointGuideFigure;
    }
    
    namespace Controller {
        class Editor;
        
        class VertexTool : public DragTool {
        private:
            bool m_selected;
            Model::Brush* m_brush;
            size_t m_index;
        protected:
            Renderer::HandleFigure* m_handleFigure;
            Renderer::HandleFigure* m_selectedHandleFigure;
            Renderer::PointGuideFigure* m_guideFigure;
            
            virtual int hitType() = 0;
            virtual size_t index(Model::Hit& hit);
            virtual std::string undoName() = 0;
            virtual Vec3f movePosition(const Model::Brush& brush, size_t index) = 0;
            virtual const Vec4f& handleColor() = 0;
            virtual const Vec4f& hiddenHandleColor() = 0;
            virtual const Vec4f& selectedHandleColor() = 0;
            virtual const Vec4f& hiddenSelectedHandleColor() = 0;
            virtual Model::MoveResult performMove(Model::Brush& brush, size_t index, const Vec3f& delta) = 0;
            
            virtual void brushesDidChange(const Model::BrushList& brushes);
            virtual void selectionChanged(const Model::SelectionEventData& event);

            virtual void createHandleFigure();
            virtual void updateHandleFigure(Renderer::HandleFigure& figure) = 0;
            virtual void deleteHandleFigure();
            virtual void createSelectedHandleFigures();
            virtual void updateSelectedHandleFigures(Renderer::HandleFigure& handleFigure, Renderer::PointGuideFigure& guideFigure, const Model::Brush& brush, size_t index) = 0;
            virtual void deleteSelectedHandleFigures();
        public:
            VertexTool(Controller::Editor& editor);
            virtual ~VertexTool() {}

            bool selected();
            Model::Brush* brush();
            size_t index();
            
            bool handleActivated(InputEvent& event);
            bool handleDeactivated(InputEvent& event);
            bool handleMouseDown(InputEvent& event);
            bool handleMouseUp(InputEvent& event);

            bool handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint);
            bool handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            void handleEndPlaneDrag(InputEvent& event);
        };
    }
}

#endif
