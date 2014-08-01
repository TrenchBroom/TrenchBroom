/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
    class Hits;
    
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        
        class ActivationPolicy {
        public:
            virtual ~ActivationPolicy();
            
            virtual bool initiallyActive() const;
            virtual bool doActivate(const InputState& inputState) = 0;
            virtual bool doDeactivate(const InputState& inputState) = 0;
        };
        
        class NoActivationPolicy {
        public:
            ~NoActivationPolicy();
            
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
        };
        
        class PickingPolicy {
        public:
            virtual ~PickingPolicy();
            virtual void doPick(const InputState& inputState, Hits& hits) = 0;
        };
        
        class NoPickingPolicy {
        public:
            ~NoPickingPolicy();
            void doPick(const InputState& inputState, Hits& hits);
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
            ~NoMousePolicy();
            
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
            ~NoMouseDragPolicy();
            
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
        
        class PlaneDragHelper {
        public:
            virtual ~PlaneDragHelper();
            
            virtual bool startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
            virtual bool planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) = 0;
            virtual void endPlaneDrag(const InputState& inputState) = 0;
            virtual void cancelPlaneDrag(const InputState& inputState) = 0;
            virtual void resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
            virtual void render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext) = 0;
        };
        
        class DropPolicy {
        public:
            virtual ~DropPolicy();
            virtual bool doDragEnter(const InputState& inputState, const String& payload) = 0;
            virtual bool doDragMove(const InputState& inputState) = 0;
            virtual void doDragLeave(const InputState& inputState) = 0;
            virtual bool doDragDrop(const InputState& inputState) = 0;
        };
        
        class NoDropPolicy {
        public:
            ~NoDropPolicy();
            bool doDragEnter(const InputState& inputState, const String& payload);
            bool doDragMove(const InputState& inputState);
            void doDragLeave(const InputState& inputState);
            bool doDragDrop(const InputState& inputState);
        };

        class RenderPolicy {
        public:
            virtual ~RenderPolicy();
            virtual void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
        };
        
        typedef RenderPolicy NoRenderPolicy;
        
        class Tool {
        private:
            Tool* m_next;
        public:
            Tool();
            virtual ~Tool();

            virtual bool active() const = 0;
            virtual bool activate(const InputState& inputState) = 0;
            virtual void deactivate(const InputState& inputState) = 0;
            
            virtual void pick(const InputState& inputState, Hits& hits) = 0;
            
            virtual void modifierKeyChange(const InputState& inputState) = 0;
            
            virtual bool mouseDown(const InputState& inputState) = 0;
            virtual bool mouseUp(const InputState& inputState) = 0;
            virtual bool mouseDoubleClick(const InputState& inputState) = 0;
            virtual void scroll(const InputState& inputState) = 0;
            virtual void mouseMove(const InputState& inputState) = 0;
            
            virtual Tool* startMouseDrag(const InputState& inputState) = 0;
            virtual bool mouseDrag(const InputState& inputState) = 0;
            virtual void endMouseDrag(const InputState& inputState) = 0;
            virtual void cancelMouseDrag(const InputState& inputState) = 0;
            
            virtual Tool* dragEnter(const InputState& inputState, const String& payload) = 0;
            virtual bool dragMove(const InputState& inputState) = 0;
            virtual void dragLeave(const InputState& inputState) = 0;
            virtual bool dragDrop(const InputState& inputState) = 0;
            
            virtual void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const = 0;
            virtual void renderChain(const InputState& inputState, Renderer::RenderContext& renderContext) = 0;
            virtual void renderOnly(const InputState& inputState, Renderer::RenderContext& renderContext) = 0;
            
            Tool* next() const;
            void appendTool(Tool* tool);
        };
        
        template <class ActivationPolicyType, class PickingPolicyType, class MousePolicyType, class MouseDragPolicyType, class DropPolicyType, class RenderPolicyType>
        class ToolImpl : public Tool, protected ActivationPolicyType, protected PickingPolicyType, protected MousePolicyType, protected DropPolicyType, protected MouseDragPolicyType, protected RenderPolicyType {
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            bool m_dragging;
            bool m_active;
        public:
            ToolImpl(MapDocumentWPtr document, ControllerWPtr controller) :
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
                assert(!active());
                if (static_cast<ActivationPolicyType&>(*this).doActivate(inputState))
                    m_active = true;
                return m_active;
            }
            
            void deactivate(const InputState& inputState) {
                assert(active());
                if (static_cast<ActivationPolicyType&>(*this).doDeactivate(inputState))
                    m_active = false;
            }
            
            void pick(const InputState& inputState, Hits& hits) {
                if (active())
                    static_cast<PickingPolicyType&>(*this).doPick(inputState, hits);
                if (next() != NULL)
                    next()->pick(inputState, hits);
            }

            void modifierKeyChange(const InputState& inputState) {
                if (active())
                    doModifierKeyChange(inputState);
                if (next() != NULL)
                    next()->modifierKeyChange(inputState);
            }
            
            bool mouseDown(const InputState& inputState) {
                if (active() && static_cast<MousePolicyType&>(*this).doMouseDown(inputState))
                    return true;
                if (next() != NULL)
                    return next()->mouseDown(inputState);
                return false;
            }
            
            bool mouseUp(const InputState& inputState) {
                if (active() && static_cast<MousePolicyType&>(*this).doMouseUp(inputState))
                    return true;
                if (next() != NULL)
                    return next()->mouseUp(inputState);
                return false;
            }
            
            bool mouseDoubleClick(const InputState& inputState) {
                if (active() && static_cast<MousePolicyType&>(*this).doMouseDoubleClick(inputState))
                    return true;
                if (next() != NULL)
                    return next()->mouseDoubleClick(inputState);
                return false;
            }
            
            void scroll(const InputState& inputState) {
                if (active())
                    static_cast<MousePolicyType&>(*this).doScroll(inputState);
                if (next() != NULL)
                    next()->scroll(inputState);
            }
            
            void mouseMove(const InputState& inputState) {
                if (active())
                    static_cast<MousePolicyType&>(*this).doMouseMove(inputState);
                if (next() != NULL)
                    next()->mouseMove(inputState);
            }

            Tool* startMouseDrag(const InputState& inputState) {
                if (active() && static_cast<MouseDragPolicyType&>(*this).doStartMouseDrag(inputState)) {
                    m_dragging = true;
                    return this;
                }
                if (next() != NULL)
                    return next()->startMouseDrag(inputState);
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
                m_dragging = false;
                static_cast<MouseDragPolicyType&>(*this).doEndMouseDrag(inputState);
            }
            
            void cancelMouseDrag(const InputState& inputState) {
                if (dragging()) {
                    m_dragging = false;
                    static_cast<MouseDragPolicyType&>(*this).doCancelMouseDrag(inputState);
                }
            }
            
            Tool* dragEnter(const InputState& inputState, const String& payload) {
                if (active() && static_cast<DropPolicyType&>(*this).doDragEnter(inputState, payload))
                    return this;
                if (next() != NULL)
                    return next()->dragEnter(inputState, payload);
                return NULL;
            }
            
            bool dragMove(const InputState& inputState) {
                assert(active());
                return static_cast<DropPolicyType&>(*this).doDragMove(inputState);
            }
            
            void dragLeave(const InputState& inputState) {
                assert(active());
                static_cast<DropPolicyType&>(*this).doDragLeave(inputState);
            }
            
            bool dragDrop(const InputState& inputState) {
                assert(active());
                return static_cast<DropPolicyType&>(*this).doDragDrop(inputState);
            }

            void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
                if (active())
                    static_cast<const RenderPolicyType&>(*this).doSetRenderOptions(inputState, renderContext);
                if (next() != NULL)
                    next()->setRenderOptions(inputState, renderContext);
            }

            void renderChain(const InputState& inputState, Renderer::RenderContext& renderContext) {
                if (active())
                    static_cast<RenderPolicyType&>(*this).doRender(inputState, renderContext);
                if (next() != NULL)
                    next()->renderChain(inputState, renderContext);
            }

            void renderOnly(const InputState& inputState, Renderer::RenderContext& renderContext) {
                if (active())
                    static_cast<RenderPolicyType&>(*this).doRender(inputState, renderContext);
            }
        protected:
            virtual void doModifierKeyChange(const InputState& inputState) {}
            
            MapDocumentSPtr document() {
                return lock(m_document);
            }
            
            MapDocumentSPtr document() const {
                return lock(m_document);
            }
            
            ControllerSPtr controller() {
                return lock(m_controller);
            }
            
            const ControllerSPtr controller() const {
                return lock(m_controller);
            }
            
            bool dragging() const {
                return m_dragging;
            }
        };
    }
}

#endif
