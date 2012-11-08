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

#ifndef TrenchBroom_ToolNew_h
#define TrenchBroom_ToolNew_h

#include <cassert>

#include "Utility/String.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
    }
    
    namespace Controller {
        class InputState {
        public:
            Ray pickRay;
        };
        
        class ActivationProtocol {
        private:
            bool m_active;
        protected:
            inline bool active() const {
                return m_active();
            }
            
            virtual void doActivate(InputState& inputState) = 0;
            virtual void doDeactivate(InputState& inputState) = 0;
            virtual bool doWantsFocus(InputState& inputState) = 0;
        public:
            ActivationProtocol() :
            m_active(false) {}
            
            virtual ~ActivationProtocol() {}
            
            void activate(InputState& inputState) {
                assert(!active());
                doActivate(inputState);
                m_active = true;
            }
            
            void deactivate(InputState& inputState) {
                assert(active());
                doDeactivate(inputState);
                m_active = false;
            }
            
            bool wantsFocus(InputState& inputState) {
                return doWantsFocus(inputState);
            }
        };
        
        class DragProtocol {
        private:
            bool m_dragging;
        protected:
            inline bool dragging() const {
                return m_dragging;
            }
            
            virtual ~DragProtocol() {}
            
            virtual bool doStartDrag(InputState& inputState) = 0;
            virtual void doDrag(InputState& inputState) = 0;
            virtual void doEndDrag(InputState& inputState) = 0;
        public:
            DragProtocol() :
            m_dragging(false) {}
            
            bool startDrag(InputState& inputState) {
                assert(!dragging());
                m_dragging = doStartDrag(inputState);
                return m_dragging;
            }
            
            void drag(InputState& inputState) {
                assert(dragging());
                doDrag(inputState);
            }
            
            void endDrag(InputState& inputState) {
                assert(dragging());
                doEndDrag(inputState);
                m_dragging = false;
            }
        };
        
        class DragPlaneProtocol : public DragProtocol {
        private:
            Plane m_dragPlane;
            Vec3f m_lastPoint;
            Vec3f m_refPoint;
        protected:
            virtual bool doStartPlaneDrag(InputState& inputState, Plane& dragPlane, Vec3f& initialDragPoint) = 0;
            virtual void doPlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) = 0;
            virtual void doEndPlaneDrag(InputState& inputState) = 0;
            
            bool doStartDrag(InputState& inputState) {
                if (doStartPlaneDrag(inputState, m_dragPlane, m_lastPoint)) {
                    m_refPoint = m_lastPoint;
                    return true;
                }
                return false;
            }
            
            void doDrag(InputState& inputState) {
                float distance = m_dragPlane.intersectWithRay(inputState.pickRay);
                if (Math::isnan(distance))
                    return;

                Vec3f point = inputState.pickRay.pointAtDistance(distance);
                if (point.equals(m_lastPoint))
                    return;
                
                doPlaneDrag(inputState, m_lastPoint, point, m_refPoint);
                m_lastPoint = point;
            }
            
            void doEndDrag(InputState& inputState) {
                doEndPlaneDrag(inputState);
            }
        public:
            DragPlaneProtocol() {}
            
            virtual ~DragPlaneProtocol() {}
        };
        
        class DragTargetProtocol {
        private:
            bool m_dragging;
            String m_payload;
        protected:
            inline bool dragging() const {
                return m_dragging;
            }
            
            inline const String& payload() const {
                return m_payload;
            }
            
            virtual bool doDragEnter(InputEvent& event) = 0;
            virtual void doDragMove(InputEvent& event) = 0;
            virtual void doDragLeave(InputEvent& event) = 0;
            virtual bool doDragDrop(InputEvent& event) = 0;
        public:
            DragTargetProtocol() :
            m_dragging(false) {}
            
            virtual ~DragTargetProtocol() {}
            
            bool dragEnter(InputEvent& event, const String& payload) {
                assert(!dragging());
                m_payload = payload;
                m_dragging = doDragEnter(event);
                return m_dragging;
            }
            
            void dragMove(InputEvent& event) {
                assert(dragging());
                doDragMove(event);
            }
            
            void dragLeave(InputEvent& event) {
                assert(dragging());
                doDragLeave(event);
                m_payload = "";
                m_dragging = false;
            }
            
            bool dragDrop(InputEvent& event) {
                bool success = doDragDrop(event);
                m_payload = "";
                m_dragging = false;
                return success;
            }
        };
        
        class FeedbackProtocol {
        private:
            m_needsUpdate;
        protected:
            inline void setNeedsUpdate() {
                m_needsUpdate = true;
            }
        public:
            FeedbackProtocol() :
            m_needsUpdate(false) {}
            
            virtual ~FeedbackProtocol() {}
            
            inline bool needsUpdate() {
                bool needsUpdate = m_needsUpdate;
                m_needsUpdate = false;
                return needsUpdate;
            }
            
            virtual void render(Vbo& vbo, RenderContext& renderContext) = 0;
        };
        
        class InputProtocol {
        };
        
        class Tool {
        public:
        };
    }
}

#endif
