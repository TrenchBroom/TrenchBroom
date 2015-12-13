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

#ifndef TrenchBroom_ToolAdapter
#define TrenchBroom_ToolAdapter

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        class Tool;
        
        class PickingPolicy {
        public:
            virtual ~PickingPolicy();
        public:
            virtual void doPick(const InputState& inputState, Model::PickResult& pickResult) = 0;
        };
        
        class NoPickingPolicy : public PickingPolicy {
        public:
            virtual ~NoPickingPolicy();
        public:
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
        };
        
        class KeyPolicy {
        public:
            virtual ~KeyPolicy();
        public:
            virtual void doModifierKeyChange(const InputState& inputState) = 0;
        };
        
        class NoKeyPolicy : public KeyPolicy {
        public:
            virtual ~NoKeyPolicy();
        public:
            void doModifierKeyChange(const InputState& inputState);
        };
        
        class MousePolicy {
        public:
            virtual ~MousePolicy();
        public:
            virtual void doMouseDown(const InputState& inputState);
            virtual void doMouseUp(const InputState& inputState);
            virtual bool doMouseClick(const InputState& inputState);
            virtual bool doMouseDoubleClick(const InputState& inputState);
            virtual void doMouseMove(const InputState& inputState);
            virtual void doMouseScroll(const InputState& inputState);
        };
        
        typedef MousePolicy NoMousePolicy;
        
        class MouseDragPolicy {
        public:
            virtual ~MouseDragPolicy();
        public:
            virtual bool doStartMouseDrag(const InputState& inputState) = 0;
            virtual bool doMouseDrag(const InputState& inputState) = 0;
            virtual void doEndMouseDrag(const InputState& inputState) = 0;
            virtual void doCancelMouseDrag() = 0;
        };
        
        class NoMouseDragPolicy : public MouseDragPolicy {
        public:
            virtual ~NoMouseDragPolicy();
        public:
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
        };
        
        class DelegatingMouseDragPolicy : public MouseDragPolicy {
        private:
            MouseDragPolicy* m_delegate;
        protected:
            DelegatingMouseDragPolicy();
        public:
            virtual ~DelegatingMouseDragPolicy();
        public:
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
        private:
            virtual MouseDragPolicy* doCreateDelegate(const InputState& inputState) = 0;
            virtual void doDeleteDelegate(MouseDragPolicy* delegate) = 0;
            
            virtual void doMouseDragStarted();
            virtual void doMouseDragged();
            virtual void doMouseDragEnded();
            virtual void doMouseDragCancelled();
        };
        
        class PlaneDragPolicy : public MouseDragPolicy {
        private:
            Plane3 m_plane;
            Vec3 m_lastPoint;
            Vec3 m_refPoint;
            bool m_dragging;
        public:
            PlaneDragPolicy();
            virtual ~PlaneDragPolicy();
        public:
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();

            bool dragging() const;
            void resetPlane(const InputState& inputState);
        private: // subclassing interface
            virtual bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
            virtual bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) = 0;
            virtual void doEndPlaneDrag(const InputState& inputState) = 0;
            virtual void doCancelPlaneDrag() = 0;
            virtual void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
        };
        
        class PlaneDragHelper {
        private:
            PlaneDragPolicy* m_policy;
        public:
            PlaneDragHelper(PlaneDragPolicy* policy);
            virtual ~PlaneDragHelper();
            
            bool startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void endPlaneDrag(const InputState& inputState);
            void cancelPlaneDrag();
            void resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        protected:
            bool dragging() const;
            void resetPlane(const InputState& inputState);
        private:
            virtual bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
            virtual bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) = 0;
            virtual void doEndPlaneDrag(const InputState& inputState) = 0;
            virtual void doCancelPlaneDrag() = 0;
            virtual void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) = 0;
            virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
        };

        class LineDragPolicy : public MouseDragPolicy {
        private:
            Line3 m_line;
            FloatType m_lastDist;
            FloatType m_refDist;
            bool m_dragging;
        public:
            LineDragPolicy();
            virtual ~LineDragPolicy();
        public:
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            bool dragging() const;
        private: // subclassing interface
            virtual bool doStartLineDrag(const InputState& inputState, Line3& ray, FloatType& initialDist) = 0;
            virtual bool doLineDrag(const InputState& inputState, FloatType lastDist, FloatType curDist, FloatType& refDist) = 0;
            virtual void doEndLineDrag(const InputState& inputState) = 0;
            virtual void doCancelLineDrag() = 0;
        };
        
        class RenderPolicy {
        public:
            virtual ~RenderPolicy();
        public:
            virtual void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
        
        typedef RenderPolicy NoRenderPolicy;
        
        class DropPolicy {
        public:
            virtual ~DropPolicy();
        public:
            virtual bool doDragEnter(const InputState& inputState, const String& payload) = 0;
            virtual bool doDragMove(const InputState& inputState) = 0;
            virtual void doDragLeave(const InputState& inputState) = 0;
            virtual bool doDragDrop(const InputState& inputState) = 0;
        };
        
        class NoDropPolicy : public DropPolicy {
        public:
            virtual ~NoDropPolicy();
        public:
            bool doDragEnter(const InputState& inputState, const String& payload);
            bool doDragMove(const InputState& inputState);
            void doDragLeave(const InputState& inputState);
            bool doDragDrop(const InputState& inputState);
        };
        
        class ToolAdapter {
        public:
            virtual ~ToolAdapter();
            
            Tool* tool();
            bool toolActive();
            
            virtual void pick(const InputState& inputState, Model::PickResult& pickResult) = 0;
            
            virtual void modifierKeyChange(const InputState& inputState) = 0;

            virtual void mouseDown(const InputState& inputState) = 0;
            virtual void mouseUp(const InputState& inputState) = 0;
            virtual bool mouseClick(const InputState& inputState) = 0;
            virtual bool mouseDoubleClick(const InputState& inputState) = 0;
            virtual void mouseMove(const InputState& inputState) = 0;
            virtual void mouseScroll(const InputState& inputState) = 0;

            virtual bool startMouseDrag(const InputState& inputState) = 0;
            virtual bool mouseDrag(const InputState& inputState) = 0;
            virtual void endMouseDrag(const InputState& inputState) = 0;
            virtual void cancelMouseDrag() = 0;

            virtual void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) = 0;
            virtual void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;

            virtual bool dragEnter(const InputState& inputState, const String& payload) = 0;
            virtual bool dragMove(const InputState& inputState) = 0;
            virtual void dragLeave(const InputState& inputState) = 0;
            virtual bool dragDrop(const InputState& inputState) = 0;
            
            virtual bool cancel() = 0;
        protected:
            void refreshViews();
        private:
            virtual Tool* doGetTool() = 0;
        };
        
        template <class PickingPolicyType, class KeyPolicyType, class MousePolicyType, class MouseDragPolicyType, class RenderPolicyType, class DropPolicyType>
        class ToolAdapterBase : public ToolAdapter, protected PickingPolicyType, protected KeyPolicyType, protected MousePolicyType, protected MouseDragPolicyType, protected RenderPolicyType, protected DropPolicyType {
        private:
            bool m_dragging;
        public:
            ToolAdapterBase() :
            m_dragging(false) {}
            
            virtual ~ToolAdapterBase() {}
            
            void pick(const InputState& inputState, Model::PickResult& pickResult) {
                if (toolActive())
                    static_cast<PickingPolicyType*>(this)->doPick(inputState, pickResult);
            }
            
            void modifierKeyChange(const InputState& inputState) {
                if (toolActive())
                    static_cast<KeyPolicyType*>(this)->doModifierKeyChange(inputState);
            }
            
            void mouseDown(const InputState& inputState) {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseDown(inputState);
            }
            
            void mouseUp(const InputState& inputState) {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseUp(inputState);
            }
            
            bool mouseClick(const InputState& inputState) {
                if (toolActive())
                    return static_cast<MousePolicyType*>(this)->doMouseClick(inputState);
                return false;
            }
            
            bool mouseDoubleClick(const InputState& inputState) {
                if (toolActive())
                    return static_cast<MousePolicyType*>(this)->doMouseDoubleClick(inputState);
                return false;
            }

            void mouseMove(const InputState& inputState) {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseMove(inputState);
            }
            
            void mouseScroll(const InputState& inputState) {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseScroll(inputState);
            }
            
            bool startMouseDrag(const InputState& inputState) {
                m_dragging = toolActive() && static_cast<MouseDragPolicyType*>(this)->doStartMouseDrag(inputState);
                return m_dragging;
            }
            
            bool mouseDrag(const InputState& inputState) {
                assert(dragging() && toolActive());
                return static_cast<MouseDragPolicyType*>(this)->doMouseDrag(inputState);
            }
            
            void endMouseDrag(const InputState& inputState) {
                assert(dragging() && toolActive());
                static_cast<MouseDragPolicyType*>(this)->doEndMouseDrag(inputState);
                m_dragging = false;
            }
            
            void cancelMouseDrag() {
                assert(dragging() && toolActive());
                static_cast<MouseDragPolicyType*>(this)->doCancelMouseDrag();
                m_dragging = false;
            }
            
            void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) {
                if (toolActive())
                    static_cast<RenderPolicyType*>(this)->doSetRenderOptions(inputState, renderContext);
            }
            
            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                if (toolActive())
                    static_cast<RenderPolicyType*>(this)->doRender(inputState, renderContext, renderBatch);
            }
            
            bool dragEnter(const InputState& inputState, const String& payload) {
                if (toolActive())
                    return static_cast<DropPolicyType*>(this)->doDragEnter(inputState, payload);
                return false;
            }
            
            bool dragMove(const InputState& inputState) {
                if (toolActive())
                    return static_cast<DropPolicyType*>(this)->doDragMove(inputState);
                return false;
            }
            
            void dragLeave(const InputState& inputState) {
                if (toolActive())
                    static_cast<DropPolicyType*>(this)->doDragLeave(inputState);
            }
            
            bool dragDrop(const InputState& inputState) {
                if (toolActive())
                    return static_cast<DropPolicyType*>(this)->doDragDrop(inputState);
                return false;
            }
            
            bool cancel() {
                return doCancel();
            }
        protected:
            bool dragging() const {
                return m_dragging;
            }
        private:
            virtual bool doCancel() = 0;
        };
    }
}

#endif /* defined(TrenchBroom_ToolAdapter) */
