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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Tool_h
#define TrenchBroom_Tool_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        
        class ActivationPolicy {
        public:
            virtual ~ActivationPolicy();
            
            virtual bool activatable() const;
            virtual bool initiallyActive() const;
            virtual bool doActivate(const InputState& inputState) = 0;
            virtual bool doDeactivate(const InputState& inputState) = 0;
        };
        
        class NoActivationPolicy {
        public:
            virtual ~NoActivationPolicy();
            
            bool activatable() const;
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
        };
        
        class MousePolicy {
        public:
            virtual ~MousePolicy();

            virtual bool doMouseDown(const InputState& inputState);
            virtual bool doMouseUp(const InputState& inputState);
            virtual bool doMouseDoubleClick(const InputState& inputState);
            virtual void doScroll(const InputState& inputState);
            virtual void doMouseMove(const InputState& inputState);
        };
        
        class NoMousePolicy {
        public:
            virtual ~NoMousePolicy();
            
            bool doMouseDown(const InputState& inputState);
            bool doMouseUp(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            void doScroll(const InputState& inputState);
            void doMouseMove(const InputState& inputState);
        };
        
        class MouseDragPolicy {
        public:
            virtual ~MouseDragPolicy();
            
            virtual bool doStartMouseDrag(const InputState& inputState) = 0;
            virtual bool doMouseDrag(const InputState& inputState) = 0;
            virtual void doEndMouseDrag(const InputState& inputState) = 0;
            virtual void doCancelMouseDrag(const InputState& inputState) = 0;
        };
        
        class NoMouseDragPolicy {
        public:
            virtual ~NoMouseDragPolicy();
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
        };
        
        class PlaneDragPolicy {
        private:
            Plane3 m_plane;
            Vec3 m_lastPoint;
            Vec3 m_refPoint;
        public:
            virtual ~PlaneDragPolicy();
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
            void resetPlane(const InputState& inputState);

            virtual bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
            virtual bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) = 0;
            virtual void doEndPlaneDrag(const InputState& inputState) = 0;
            virtual void doCancelPlaneDrag(const InputState& inputState) = 0;
            virtual void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        };
        
        class RenderPolicy {
        public:
            virtual ~RenderPolicy();
            virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext) = 0;
        };
        
        class NoRenderPolicy {
        public:
            virtual ~NoRenderPolicy();
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
        };
        
        class BaseTool {
        public:
            virtual ~BaseTool();

            virtual bool active() const = 0;
            virtual bool activate(const InputState& inputState) = 0;
            virtual void deactivate(const InputState& inputState) = 0;
            
            virtual void modifierKeyChange(const InputState& inputState) = 0;
            virtual bool mouseDown(const InputState& inputState) = 0;
            virtual bool mouseUp(const InputState& inputState) = 0;
            virtual bool mouseDoubleClick(const InputState& inputState) = 0;
            virtual void scroll(const InputState& inputState) = 0;
            virtual void mouseMove(const InputState& inputState) = 0;
            
            virtual BaseTool* startMouseDrag(const InputState& inputState) = 0;
            virtual bool mouseDrag(const InputState& inputState) = 0;
            virtual void endMouseDrag(const InputState& inputState) = 0;
            virtual void cancelMouseDrag(const InputState& inputState) = 0;
            
            virtual void render(const InputState& inputState, Renderer::RenderContext& renderContext) = 0;
        };
        
        template <class ActivationPolicyType, class MousePolicyType, class MouseDragPolicyType, class RenderPolicyType>
        class Tool : public BaseTool, protected ActivationPolicyType, protected MousePolicyType, protected MouseDragPolicyType, protected RenderPolicyType {
        private:
            BaseTool* m_next;
            MapDocumentPtr m_document;
            ControllerFacade& m_controller;
            bool m_dragging;
            bool m_active;
        public:
            Tool(BaseTool* next, MapDocumentPtr document, ControllerFacade& controller) :
            m_next(next),
            m_document(document),
            m_controller(controller),
            m_dragging(false),
            m_active(false) {
                m_active = static_cast<ActivationPolicyType&>(*this).initiallyActive();
            }
            
            bool active() const {
                return m_active;
            }
            
            bool activate(const InputState& inputState) {
                if (static_cast<ActivationPolicyType&>(*this).activatable()) {
                    assert(!active());
                    if (static_cast<ActivationPolicyType&>(*this).doActivate(inputState))
                        m_active = true;
                    return m_active;
                }
                return false;
            }
            
            void deactivate(const InputState& inputState) {
                if (static_cast<ActivationPolicyType&>(*this).activatable()) {
                    assert(active());
                    if (static_cast<ActivationPolicyType&>(*this).doDeactivate(inputState))
                        m_active = false;
                }
            }
            
            void modifierKeyChange(const InputState& inputState) {
                if (active())
                    doModifierKeyChange(inputState);
                if (m_next != NULL)
                    m_next->modifierKeyChange(inputState);
            }
            
            bool mouseDown(const InputState& inputState) {
                if (active() && static_cast<MousePolicyType&>(*this).doMouseDown(inputState))
                    return true;
                if (m_next != NULL)
                    return m_next->mouseDown(inputState);
                return false;
            }
            
            bool mouseUp(const InputState& inputState) {
                if (active() && static_cast<MousePolicyType&>(*this).doMouseUp(inputState))
                    return true;
                if (m_next != NULL)
                    return m_next->mouseUp(inputState);
                return false;
            }
            
            bool mouseDoubleClick(const InputState& inputState) {
                if (active() && static_cast<MousePolicyType&>(*this).doMouseDoubleClick(inputState))
                    return true;
                if (m_next != NULL)
                    return m_next->mouseDoubleClick(inputState);
                return false;
            }
            
            void scroll(const InputState& inputState) {
                if (active())
                    static_cast<MousePolicyType&>(*this).doScroll(inputState);
                if (m_next != NULL)
                    m_next->scroll(inputState);
            }
            
            void mouseMove(const InputState& inputState) {
                if (active())
                    static_cast<MousePolicyType&>(*this).doMouseMove(inputState);
                if (m_next != NULL)
                    m_next->mouseMove(inputState);
            }

            BaseTool* startMouseDrag(const InputState& inputState) {
                if (active() && static_cast<MouseDragPolicyType&>(*this).doStartMouseDrag(inputState)) {
                    m_dragging = true;
                    return this;
                }
                if (m_next != NULL)
                    return m_next->startMouseDrag(inputState);
                return NULL;
            }
            
            bool mouseDrag(const InputState& inputState) {
                assert(active());
                assert(dragging());
                return static_cast<MouseDragPolicyType&>(*this).doMouseDrag(inputState);
            }
            
            void endMouseDrag(const InputState& inputState) {
                assert(active());
                assert(dragging());
                static_cast<MouseDragPolicyType&>(*this).doEndMouseDrag(inputState);
                m_dragging = false;
            }
            
            void cancelMouseDrag(const InputState& inputState) {
                assert(active());
                assert(dragging());
                static_cast<MouseDragPolicyType&>(*this).doCancelMouseDrag(inputState);
                m_dragging = false;
            }
            
            void render(const InputState& inputState, Renderer::RenderContext& renderContext) {
                if (active())
                    static_cast<RenderPolicyType&>(*this).doRender(inputState, renderContext);
                if (m_next != NULL)
                    m_next->render(inputState, renderContext);
            }
        protected:
            virtual void doModifierKeyChange(const InputState& inputState) {}
            
            MapDocumentPtr document() {
                return m_document;
            }
            
            MapDocumentPtr document() const {
                return m_document;
            }
            
            ControllerFacade& controller() {
                return m_controller;
            }
            
            const ControllerFacade& controller() const {
                return m_controller;
            }
            
            bool dragging() const {
                return m_dragging;
            }
        };
    }
}

#endif
