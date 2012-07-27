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

#ifndef TrenchBroom_DragTool_h
#define TrenchBroom_DragTool_h

#include "Controller/Tool.h"
#include "Controller/DragPlane.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Controller {
        class DragPlane;
        class Editor;
        
        class DragTool : public Tool {
        protected:
            DragPlane m_dragPlane;
            Vec3f m_dragPlanePosition;
            Vec3f m_lastMousePoint;
            Vec3f m_lastRefPoint;
            
            virtual void updateDragPlane(InputEvent& event) {
                if (altPlaneModifierPressed(event))
                    m_dragPlane = DragPlane::vertical(event.ray.direction * -1.0f);
                else
                    m_dragPlane = DragPlane::horizontal();
            }

            virtual bool handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint) { return false; }
            virtual bool handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) { return false; }
            virtual void handleEndPlaneDrag(InputEvent& event) {}
        public:
            static bool altPlaneModifierPressed(InputEvent& event) {
                return event.modifierKeys == MK_ALT;
            }

            DragTool(Editor& editor) : Tool(editor), m_dragPlane(DragPlane::horizontal()), m_dragPlanePosition(Vec3f::Null), m_lastMousePoint(Vec3f::Null), m_lastRefPoint(Vec3f::Null) {};
            virtual ~DragTool() {}
            
            bool handleBeginDrag(InputEvent& event) {
                if (handleBeginPlaneDrag(event, m_lastMousePoint)) {
                    updateDragPlane(event);
                    m_lastRefPoint = m_lastMousePoint;
                    m_dragPlanePosition = m_lastMousePoint;
                    return true;
                }
                return false;
            }
            
            bool handleDrag(InputEvent& event) {
                float dist = m_dragPlane.intersect(event.ray, m_dragPlanePosition);
                if (Math::isnan(dist))
                    return true;
                
                Vec3f currentMousePoint = event.ray.pointAtDistance(dist);
                if (currentMousePoint.equals(m_lastMousePoint))
                    return true;
                
                Vec3f delta = currentMousePoint - m_lastRefPoint;
                if (delta.null())
                    return true;
                
                if (!handlePlaneDrag(event, m_lastMousePoint, currentMousePoint, m_lastRefPoint))
                    return false;
                
                m_lastMousePoint = currentMousePoint;
                return true;
            }
            
            void handleEndDrag(InputEvent& event) {
                handleEndPlaneDrag(event);
            }
        };
    }
}

#endif
