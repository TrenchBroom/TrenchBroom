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

#pragma once

#include "FloatType.h"
#include "ToolChain.h"
#include "Model/HitType.h"
#include "View/InputState.h"

#include <vecmath/vec.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Hit;
        class HitQuery;
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
            ~NoPickingPolicy() override;
        public:
            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;
        };

        class KeyPolicy {
        public:
            virtual ~KeyPolicy();
        public:
            virtual void doModifierKeyChange(const InputState& inputState) = 0;
        };

        class NoKeyPolicy : public KeyPolicy {
        public:
            ~NoKeyPolicy() override;
        public:
            void doModifierKeyChange(const InputState& inputState) override;
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

        using NoMousePolicy = MousePolicy;

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
            ~NoMouseDragPolicy() override;
        public:
            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;
        };

        class DragRestricter {
        public:
            virtual ~DragRestricter();
            bool hitPoint(const InputState& inputState, vm::vec3& point) const;
        private:
            virtual bool doComputeHitPoint(const InputState& inputState, vm::vec3& point) const = 0;
        };

        class PlaneDragRestricter : public DragRestricter {
        private:
            const vm::plane3 m_plane;
        public:
            explicit PlaneDragRestricter(const vm::plane3& plane);
        private:
            bool doComputeHitPoint(const InputState& inputState, vm::vec3& point) const override;
        };

        class CircleDragRestricter : public DragRestricter {
        private:
            const vm::vec3 m_center;
            const vm::vec3 m_normal;
            const FloatType m_radius;
        public:
            CircleDragRestricter(const vm::vec3& center, const vm::vec3& normal, FloatType radius);
        private:
            bool doComputeHitPoint(const InputState& inputState, vm::vec3& point) const override;
        };

        class LineDragRestricter : public DragRestricter {
        private:
            const vm::line3 m_line;
        public:
            explicit LineDragRestricter(const vm::line3& line);
        private:
            bool doComputeHitPoint(const InputState& inputState, vm::vec3& point) const override;
        };

        class SurfaceDragHelper {
        private:
            bool m_hitTypeSet;
            bool m_occludedTypeSet;
            bool m_minDistanceSet;

            bool m_pickable;
            bool m_selected;
            Model::HitType::Type m_hitTypeValue;
            Model::HitType::Type m_occludedTypeValue;
            FloatType m_minDistanceValue;
        public:
            SurfaceDragHelper();
            virtual ~SurfaceDragHelper();

            void setPickable(bool pickable);
            void setSelected(bool selected);
            void setType(Model::HitType::Type type);
            void setOccluded(Model::HitType::Type type);
            void setMinDistance(FloatType minDistance);
        protected:
            Model::HitQuery query(const InputState& inputState) const;
        };

        class SurfaceDragRestricter : public SurfaceDragHelper, public DragRestricter {
        private:
            bool doComputeHitPoint(const InputState& inputState, vm::vec3& point) const override;
        };

        class DragSnapper {
        public:
            virtual ~DragSnapper();

            bool snap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const;
        private:
            virtual bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const = 0;
        };

        class MultiDragSnapper : public DragSnapper {
        private:
            using List = std::vector<std::unique_ptr<DragSnapper>>;
            List m_delegates;
        public:
            template <typename... T>
            explicit MultiDragSnapper(T... delegates) {
                addDelegates(delegates...);
            }
        private:
            template <typename F, typename... R>
            void addDelegates(F first, R... rest) {
                m_delegates.push_back(std::unique_ptr<DragSnapper>(first));
                addDelegates(rest...);
            }

            void addDelegates();
        private:
            bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const override;
        };

        class NoDragSnapper : public DragSnapper {
        private:
            bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const override;
        };

        class Grid;

        class AbsoluteDragSnapper : public DragSnapper {
        private:
            const Grid& m_grid;
            vm::vec3 m_offset;
        public:
            explicit AbsoluteDragSnapper(const Grid& grid, const vm::vec3& offset = vm::vec3::zero());
        private:
            bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const override;
        };

        class DeltaDragSnapper : public DragSnapper {
        private:
            const Grid& m_grid;
        public:
            explicit DeltaDragSnapper(const Grid& grid);
        private:
            bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const override;
        };

        /**
         * Snaps at least one axis to grid, while staying on the given line.
         */
        class LineDragSnapper : public DragSnapper {
        private:
            const Grid& m_grid;
            vm::line3 m_line;
        public:
            LineDragSnapper(const Grid& grid, const vm::line3& line);
        private:
            bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const override;
        };

        class CircleDragSnapper : public DragSnapper {
        private:
            const Grid& m_grid;
            const FloatType m_snapAngle;
            const vm::vec3 m_start;
            const vm::vec3 m_center;
            const vm::vec3 m_normal;
            const FloatType m_radius;
        public:
            CircleDragSnapper(const Grid& grid, FloatType snapAngle, const vm::vec3& start, const vm::vec3& center, const vm::vec3& normal, FloatType radius);
        private:
            bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const override;
        };

        class SurfaceDragSnapper : public SurfaceDragHelper, public DragSnapper {
        private:
            const Grid& m_grid;
        public:
            explicit SurfaceDragSnapper(const Grid& grid);
        private:
            bool doSnap(const InputState& inputState, const vm::vec3& initialPoint, const vm::vec3& lastPoint, vm::vec3& curPoint) const override;
        private:
            virtual vm::plane3 doGetPlane(const InputState& inputState, const Model::Hit& hit) const = 0;
        };

        class RestrictedDragPolicy : public MouseDragPolicy {
        private:
            DragRestricter* m_restricter;
            DragSnapper* m_snapper;

            vm::vec3 m_initialHandlePosition;
            vm::vec3 m_currentHandlePosition;

            vm::vec3 m_initialMousePosition;
            vm::vec3 m_currentMousePosition;
        protected:
            struct DragInfo {
                DragRestricter* restricter;
                DragSnapper* snapper;

                vm::vec3 initialHandlePosition;
                bool computeInitialHandlePosition;

                DragInfo();
                DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper);
                DragInfo(DragRestricter* i_restricter, DragSnapper* i_snapper, const vm::vec3& i_initialHandlePosition);

                bool skip() const;
            };

        public:
            RestrictedDragPolicy();
            ~RestrictedDragPolicy() override;

            typedef enum {
                DR_Continue,
                DR_Deny,
                DR_Cancel
            } DragResult;

        private:
            bool dragging() const;
            void deleteRestricter();
            void deleteSnapper();
        public:
            const vm::vec3& initialHandlePosition() const;
            const vm::vec3& currentHandlePosition() const;
            const vm::vec3& initialMousePosition() const;
            const vm::vec3& currentMousePosition() const;

            bool hitPoint(const InputState& inputState, vm::vec3& result) const;
        public:
            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            void setRestricter(const InputState& inputState, DragRestricter* restricter, bool resetInitialPoint);
            void setSnapper(const InputState& inputState, DragSnapper* snapper, bool resetCurrentHandlePosition);

            bool snapPoint(const InputState& inputState, vm::vec3& point) const;
        private:
            void resetInitialPoint(const InputState& inputState);
        private: // subclassing interface
            virtual DragInfo doStartDrag(const InputState& inputState) = 0;
            virtual DragResult doDrag(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) = 0;
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

        using NoRenderPolicy = RenderPolicy;

        class DropPolicy {
        public:
            virtual ~DropPolicy();
        public:
            virtual bool doDragEnter(const InputState& inputState, const std::string& payload) = 0;
            virtual bool doDragMove(const InputState& inputState) = 0;
            virtual void doDragLeave(const InputState& inputState) = 0;
            virtual bool doDragDrop(const InputState& inputState) = 0;
        };

        class NoDropPolicy : public DropPolicy {
        public:
            ~NoDropPolicy() override;
        public:
            bool doDragEnter(const InputState& inputState, const std::string& payload) override;
            bool doDragMove(const InputState& inputState) override;
            void doDragLeave(const InputState& inputState) override;
            bool doDragDrop(const InputState& inputState) override;
        };

        class ToolController {
        public:
            virtual ~ToolController();

            Tool* tool();
            const Tool* tool() const;
            bool toolActive() const;

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

            virtual bool dragEnter(const InputState& inputState, const std::string& payload) = 0;
            virtual bool dragMove(const InputState& inputState) = 0;
            virtual void dragLeave(const InputState& inputState) = 0;
            virtual bool dragDrop(const InputState& inputState) = 0;

            virtual bool cancel() = 0;
        protected:
            void refreshViews();
        private:
            virtual Tool* doGetTool() = 0;
            virtual const Tool* doGetTool() const = 0;
        };

        template <class PickingPolicyType, class KeyPolicyType, class MousePolicyType, class MouseDragPolicyType, class RenderPolicyType, class DropPolicyType>
        class ToolControllerBase : public ToolController, protected PickingPolicyType, protected KeyPolicyType, protected MousePolicyType, protected MouseDragPolicyType, protected RenderPolicyType, protected DropPolicyType {
        private:
            bool m_dragging;
        public:
            ToolControllerBase() :
            m_dragging(false) {}

            ~ToolControllerBase() override = default;

            void pick(const InputState& inputState, Model::PickResult& pickResult) override {
                if (toolActive())
                    static_cast<PickingPolicyType*>(this)->doPick(inputState, pickResult);
            }

            void modifierKeyChange(const InputState& inputState) override {
                if (toolActive())
                    static_cast<KeyPolicyType*>(this)->doModifierKeyChange(inputState);
            }

            void mouseDown(const InputState& inputState) override {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseDown(inputState);
            }

            void mouseUp(const InputState& inputState) override {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseUp(inputState);
            }

            bool mouseClick(const InputState& inputState) override {
                if (toolActive())
                    return static_cast<MousePolicyType*>(this)->doMouseClick(inputState);
                return false;
            }

            bool mouseDoubleClick(const InputState& inputState) override {
                if (toolActive())
                    return static_cast<MousePolicyType*>(this)->doMouseDoubleClick(inputState);
                return false;
            }

            void mouseMove(const InputState& inputState) override {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseMove(inputState);
            }

            void mouseScroll(const InputState& inputState) override {
                if (toolActive())
                    static_cast<MousePolicyType*>(this)->doMouseScroll(inputState);
            }

            bool startMouseDrag(const InputState& inputState) override {
                m_dragging = (toolActive() && static_cast<MouseDragPolicyType*>(this)->doStartMouseDrag(inputState));
                return m_dragging;
            }

            bool mouseDrag(const InputState& inputState) override {
                assert(thisToolDragging() && toolActive());
                return static_cast<MouseDragPolicyType*>(this)->doMouseDrag(inputState);
            }

            void endMouseDrag(const InputState& inputState) override {
                assert(thisToolDragging() && toolActive());
                static_cast<MouseDragPolicyType*>(this)->doEndMouseDrag(inputState);
                m_dragging = false;
            }

            void cancelMouseDrag() override {
                assert(thisToolDragging() && toolActive());
                static_cast<MouseDragPolicyType*>(this)->doCancelMouseDrag();
                m_dragging = false;
            }

            bool thisToolDragging() const override {
                return m_dragging;
            }

            bool anyToolDragging(const InputState& inputState) const override {
                return inputState.anyToolDragging();
            }

            void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) override {
                if (toolActive())
                    static_cast<RenderPolicyType*>(this)->doSetRenderOptions(inputState, renderContext);
            }

            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                if (toolActive())
                    static_cast<RenderPolicyType*>(this)->doRender(inputState, renderContext, renderBatch);
            }

            bool dragEnter(const InputState& inputState, const std::string& payload) override {
                if (toolActive())
                    return static_cast<DropPolicyType*>(this)->doDragEnter(inputState, payload);
                return false;
            }

            bool dragMove(const InputState& inputState) override {
                if (toolActive())
                    return static_cast<DropPolicyType*>(this)->doDragMove(inputState);
                return false;
            }

            void dragLeave(const InputState& inputState) override {
                if (toolActive())
                    static_cast<DropPolicyType*>(this)->doDragLeave(inputState);
            }

            bool dragDrop(const InputState& inputState) override {
                if (toolActive())
                    return static_cast<DropPolicyType*>(this)->doDragDrop(inputState);
                return false;
            }

            bool cancel() override {
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
            ~ToolControllerGroup() override;
        protected:
            void addController(ToolController* controller);
        protected:
            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void doModifierKeyChange(const InputState& inputState) override;

            void doMouseDown(const InputState& inputState) override;
            void doMouseUp(const InputState& inputState) override;
            bool doMouseClick(const InputState& inputState) override;
            bool doMouseDoubleClick(const InputState& inputState) override;
            void doMouseMove(const InputState& inputState) override;
            void doMouseScroll(const InputState& inputState) override;

            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doDragEnter(const InputState& inputState, const std::string& payload) override;
            bool doDragMove(const InputState& inputState) override;
            void doDragLeave(const InputState& inputState) override;
            bool doDragDrop(const InputState& inputState) override;

            bool doCancel() override;
        private: // subclassing interface
            virtual bool doShouldHandleMouseDrag(const InputState& inputState) const;
            virtual void doMouseDragStarted(const InputState& inputState);
            virtual void doMouseDragged(const InputState& inputState);
            virtual void doMouseDragEnded(const InputState& inputState);
            virtual void doMouseDragCancelled();

            virtual bool doShouldHandleDrop(const InputState& inputState, const std::string& payload) const;
        };
    }
}


