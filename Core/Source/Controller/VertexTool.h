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
#include "Model/Map/Brush.h"
#include "Model/Map/Map.h"
#include "Model/Selection.h"
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
            bool m_figureCreated;
        protected:
            virtual int hitType() = 0;
            virtual size_t index(Model::Hit& hit);
            virtual std::string undoName() = 0;
            virtual Vec3f movePosition(const Model::Brush& brush, size_t index) = 0;
            virtual Model::MoveResult performMove(Model::Brush& brush, size_t index, const Vec3f& delta) = 0;
            
            typedef Model::Map::BrushEvent::Listener<VertexTool> BrushListener;
            typedef Model::Map::MapEvent::Listener<VertexTool> MapListener;
            typedef Model::Selection::SelectionEvent::Listener<VertexTool> SelectionListener;
            
            virtual void brushesDidChange(const Model::BrushList& brushes);
            virtual void mapCleared(Model::Map& map);
            virtual void selectionChanged(const Model::SelectionEventData& event);
        public:
            VertexTool(Controller::Editor& editor);
            virtual ~VertexTool() {}

            bool selected() {
                return m_selected;
            }
            
            Model::Brush* brush() {
                return m_brush;
            }
            
            size_t index() {
                return m_index;
            }
            
            bool handleActivated(InputEvent& event);
            bool handleDeactivated(InputEvent& event);
            bool handleMouseDown(InputEvent& event);
            bool handleMouseUp(InputEvent& event);

            bool handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint);
            bool handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            void handleEndPlaneDrag(InputEvent& event);

            virtual const Vec3fList handlePositions() = 0;
            virtual const Vec3fList selectedHandlePositions() = 0;
            virtual const Vec3f draggedHandlePosition() = 0;
            
            virtual const Vec4f& handleColor() = 0;
            virtual const Vec4f& hiddenHandleColor() = 0;
            virtual const Vec4f& selectedHandleColor() = 0;
            virtual const Vec4f& hiddenSelectedHandleColor() = 0;
        };
    }
}

#endif
