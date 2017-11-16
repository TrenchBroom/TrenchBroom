/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_ToolController
#define TrenchBroom_ToolController

#include "TrenchBroom.h"
#include "VecMath.h"
#include "ToolChain.h"
#include "Model/Hit.h"
#include "Model/HitQuery.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
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
        
        class DragRestricter {
        public:
            virtual ~DragRestricter();
            bool hitPoint(const InputState& inputState, Vec3& point) const;
        private:
            virtual bool doComputeHitPoint(const InputState& inputState, Vec3& point) const = 0;
        };

        class PlaneDragRestricter : public DragRestricter {
        private:
            const Plane3 m_plane;
        public:
            PlaneDragRestricter(const Plane3& plane);
        private:
            bool doComputeHitPoint(const InputState& inputState, Vec3& point) const;
        };
        
        class CircleDragRestricter : public DragRestricter {
        private:
            const Vec3 m_center;
            const Vec3 m_normal;
            const FloatType m_radius;
        public:
            CircleDragRestricter(const Vec3& center, const Vec3& normal, FloatType radius);
        private:
            bool doComputeHitPoint(const InputState& inputState, Vec3& point) const;
        };
        
        class LineDragRestricter : public DragRestricter {
        private:
            const Line3 m_line;
        public:
            LineDragRestricter(const Line3& line);
        private:
            bool doComputeHitPoint(const InputState& inputState, Vec3& point) const;
        };
        
        class SurfaceDragHelper {
        private:
            bool m_hitTypeSet;
            bool m_occludedTypeSet;
            bool m_minDistanceSet;
            
            bool m_pickable;
            bool m_selected;
            Model::Hit::HitType m_hitTypeValue;
            Model::Hit::HitType m_occludedTypeValue;
            FloatType m_minDistanceValue;
        public:
            SurfaceDragHelper();
            virtual ~SurfaceDragHelper();
            
            void setPickable(bool pickable);
            void setSelected(bool selected);
            void setType(Model::Hit::HitType type);
            void setOccluded(Model::Hit::HitType type);
            void setMinDistance(FloatType minDistance);
        protected:
            Model::HitQuery query(const InputState& inputState) const;
        };
        
        class SurfaceDragRestricter : public SurfaceDragHelper, public DragRestricter {
        private:
            bool doComputeHitPoint(const InputState& inputState, Vec3& point) const;
        };
        
        class DragSnapper {
        public:
            virtual ~DragSnapper();
            
            bool snap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const;
        private:
            virtual bool doSnap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const = 0;
        };
        
        class NoDragSnapper : public DragSnapper {
        private:
            bool doSnap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const;
        };
        
        class Grid;
        
        class AbsoluteDragSnapper : public DragSnapper {
        private:
            const Grid& m_grid;
            Vec3 m_offset;
        public:
            AbsoluteDragSnapper(const Grid& grid, const Vec3& offset = Vec3::Null);
        private:
            bool doSnap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const;
        };
        
        class DeltaDragSnapper : public DragSnapper {
        private:
            const Grid& m_grid;
        public:
            DeltaDragSnapper(const Grid& grid);
        private:
            bool doSnap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const;
        };
        
        class CircleDragSnapper : public DragSnapper {
        private:
            const Grid& m_grid;
            const Vec3 m_start;
            const Vec3 m_center;
            const Vec3 m_normal;
            const FloatType m_radius;
        public:
            CircleDragSnapper(const Grid& grid, const Vec3& start, const Vec3& center, const Vec3& normal, FloatType radius);
        private:
            bool doSnap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const;
        };
        
        class SurfaceDragSnapper : public SurfaceDragHelper, public DragSnapper {
        private:
            const Grid& m_grid;
        public:
            SurfaceDragSnapper(const Grid& grid);
        private:
            bool doSnap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const;
        private:
            virtual Plane3 doGetPlane(const InputState& inputState, const Model::Hit& hit) const = 0;
        };
        
        class RestrictedDragPolicy : public MouseDragPolicy {
        private:
            DragRestricter* m_restricter;
            DragSnapper* m_snapper;
            
            Vec3 m_initialHandlePosition;
            Vec3 m_currentHandlePosition;
            
            Vec3 m_initialMousePosition;
            Vec3 m_currentMousePosition;
        protected:
            struct DragInfo {
                DragRestricter* restricter;
                DragSnapper* snapper;
                
                Vec3 initialHandlePosition;
                bool computeInitialHandlePosition;
                
                DragInfo();
                DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper);
                DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper, const Vec3& i_initialHandlePosition);

                bool skip() const;
            };
            
            typedef enum {
                DR_Continue,
                DR_Deny,
                DR_Cancel
            } DragResult;
        public:
            RestrictedDragPolicy();
            virtual ~RestrictedDragPolicy();
        private:
            bool dragging() const;
            void deleteRestricter();
            void deleteSnapper();
        protected:
            const Vec3& initialHandlePosition() const;
            const Vec3& currentHandlePosition() const;
            const Vec3& initialMousePosition() const;
            const Vec3& currentMousePosition() const;
            
            bool hitPoint(const InputState& inputState, Vec3& result) const;
        public:
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();

            void setRestricter(const InputState& inputState, DragRestricter* restricter, bool resetInitialPoint);
            void setSnapper(const InputState& inputState, DragSnapper* snapper, bool resetCurrentHandlePosition);
            
            bool snapPoint(const InputState& inputState, Vec3& point) const;
        private: // subclassing interface
            virtual DragInfo doStartDrag(const InputState& inputState) = 0;
            virtual DragResult doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) = 0;
            virtual void doEndDrag(const InputState& inputState) = 0;
            virtual void doCancelDrag() = 0;
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
        
        class ToolController {
        public:
            virtual ~ToolController();
            
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
            virtual bool thisToolDragging() const = 0;
            virtual bool anyToolDragging(const InputState& inputState) const = 0;

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
        class ToolControllerBase : public ToolController, protected PickingPolicyType, protected KeyPolicyType, protected MousePolicyType, protected MouseDragPolicyType, protected RenderPolicyType, protected DropPolicyType {
        private:
            bool m_dragging;
        public:
            ToolControllerBase() :
            m_dragging(false) {}
            
            virtual ~ToolControllerBase() {}
            
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
                m_dragging = (toolActive() && static_cast<MouseDragPolicyType*>(this)->doStartMouseDrag(inputState));
                return m_dragging;
            }
            
            bool mouseDrag(const InputState& inputState) {
                assert(thisToolDragging() && toolActive());
                return static_cast<MouseDragPolicyType*>(this)->doMouseDrag(inputState);
            }
            
            void endMouseDrag(const InputState& inputState) {
                assert(thisToolDragging() && toolActive());
                static_cast<MouseDragPolicyType*>(this)->doEndMouseDrag(inputState);
                m_dragging = false;
            }
            
            void cancelMouseDrag() {
                assert(thisToolDragging() && toolActive());
                static_cast<MouseDragPolicyType*>(this)->doCancelMouseDrag();
                m_dragging = false;
            }
            
            bool thisToolDragging() const {
                return m_dragging;
            }

            bool anyToolDragging(const InputState& inputState) const {
                return inputState.anyToolDragging();
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
        private:
            virtual bool doCancel() = 0;
        };
        
        class ToolControllerGroup : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, MouseDragPolicy, RenderPolicy, DropPolicy> {
        private:
            ToolChain m_chain;
            ToolController* m_dragReceiver;
            ToolController* m_dropReceiver;
        public:
            ToolControllerGroup();
            virtual ~ToolControllerGroup();
        protected:
            void addController(ToolController* controller);
        protected:
            virtual void doPick(const InputState& inputState, Model::PickResult& pickResult);
            
            virtual void doModifierKeyChange(const InputState& inputState);

            virtual void doMouseDown(const InputState& inputState);
            virtual void doMouseUp(const InputState& inputState);
            virtual bool doMouseClick(const InputState& inputState);
            virtual bool doMouseDoubleClick(const InputState& inputState);
            virtual void doMouseMove(const InputState& inputState);
            virtual void doMouseScroll(const InputState& inputState);

            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            virtual void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doDragEnter(const InputState& inputState, const String& payload);
            bool doDragMove(const InputState& inputState);
            void doDragLeave(const InputState& inputState);
            bool doDragDrop(const InputState& inputState);
            
            virtual bool doCancel();
        private: // subclassing interface
            virtual bool doShouldHandleMouseDrag(const InputState& inputState) const;
            virtual void doMouseDragStarted(const InputState& inputState);
            virtual void doMouseDragged(const InputState& inputState);
            virtual void doMouseDragEnded(const InputState& inputState);
            virtual void doMouseDragCancelled();
            
            virtual bool doShouldHandleDrop(const InputState& inputState, const String& payload) const;
        };
    }
}

#endif /* defined(TrenchBroom_ToolController) */
