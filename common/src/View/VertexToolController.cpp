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

#include "VertexToolController.h"

#include "Renderer/RenderContext.h"
#include "View/VertexTool.h"
#include "View/VertexHandleManager.h"

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace View {

        class VertexToolController::VertexDragSnapper : public DragSnapper {
        private:
            typedef VertexHandleManagerBaseT<Vec3> HandleManager;
            const HandleManager& m_handles;
            const Vec3 m_offset;
        public:
            VertexDragSnapper(const HandleManager& handles, const Vec3& offset) :
            m_handles(handles),
            m_offset(offset) {}
        private:
            bool doSnap(const InputState& inputState, const Vec3& initialPoint, const Vec3& lastPoint, Vec3& curPoint) const override {
                const Ray3& pickRay = inputState.pickRay();
                const auto handles = m_handles.allHandles();
                
                bool anyHit = false;
                FloatType bestError = std::numeric_limits<FloatType>::max();
                Vec3 bestHandle;
                for (const Vec3& handle : handles) {
                    const FloatType dist = pickRay.intersectWithSphere(handle, pref(Preferences::HandleRadius));
                    if (!Math::isnan(dist)) {
                        const FloatType curError = pickRay.squaredDistanceToPoint(handle).distance;
                        if (curError < bestError) {
                            bestHandle = handle;
                            bestError = curError;
                        }
                        anyHit = true;
                    }
                }
                
                if (anyHit) {
                    curPoint = bestHandle - m_offset;
                }
                return anyHit;
            }
        };

        class VertexToolController::VertexPartBase {
        public:
            virtual ~VertexPartBase() {}
        protected:
            const Model::Hit& findHandleHit(const InputState& inputState) const {
                const Model::Hit& vertexHit = inputState.pickResult().query().type(VertexHandleManager::HandleHit).occluded().first();
                if (vertexHit.isMatch())
                    return vertexHit;
                const Model::Hit& edgeHit = inputState.pickResult().query().type(EdgeHandleManager::HandleHit).first();
                if (edgeHit.isMatch())
                    return edgeHit;
                return inputState.pickResult().query().type(FaceHandleManager::HandleHit).first();
            }
        };
        
        class VertexToolController::SelectVertexPart : public SelectPartBase<Vec3>, private VertexPartBase {
        public:
            SelectVertexPart(VertexTool* tool) :
            SelectPartBase(tool, VertexHandleManager::HandleHit) {}
        private:
            const Model::Hit& doFindDraggableHandle(const InputState& inputState) const override {
                return findHandleHit(inputState);
            }
            
            bool equalHandles(const Vec3& lhs, const Vec3& rhs) const override {
                return lhs.squaredDistanceTo(rhs) < MaxHandleDistance * MaxHandleDistance;
            }
        };

        class VertexToolController::MoveVertexPart : public MovePartBase, private VertexPartBase {
        public:
            MoveVertexPart(VertexTool* tool) :
            MovePartBase(tool, VertexHandleManager::HandleHit) {}
        private:
            typedef enum {
                ST_Relative,
                ST_Absolute
            } SnapType;
            
            SnapType m_lastSnapType;
            Vec3 m_handleOffset;
        private:
            void doModifierKeyChange(const InputState& inputState) override {
                MoveToolController::doModifierKeyChange(inputState);
                
                if (Super::thisToolDragging()) {
                    const SnapType currentSnapType = snapType(inputState);
                    if (currentSnapType != m_lastSnapType) {
                        setSnapper(inputState, doCreateDragSnapper(inputState), false);
                        m_lastSnapType = currentSnapType;
                    }
                }
            }

            MoveInfo doStartMove(const InputState& inputState) override {
                const MoveInfo info = MovePartBase::doStartMove(inputState);
                if (info.move) {
                    m_lastSnapType = snapType(inputState);
                    const Model::Hit& hit = findDraggableHandle(inputState);
                    const Vec3& handlePos = hit.target<Vec3>();
                    m_handleOffset = handlePos - hit.hitPoint();
                }
                return info;
            }
            
            DragSnapper* doCreateDragSnapper(const InputState& inputState) const override {
                return new MultiDragSnapper(createDistanceSnapper(inputState),
                                            createVertexSnapper(inputState));
            }
            
            DragSnapper* createDistanceSnapper(const InputState& inputState) const {
                switch (snapType(inputState)) {
                    case ST_Absolute:
                        return new AbsoluteDragSnapper(m_tool->grid(), m_handleOffset);
                    case ST_Relative:
                        return new DeltaDragSnapper(m_tool->grid());
                        switchDefault();
                }
            }
            
            DragSnapper* createVertexSnapper(const InputState& inputState) const {
                return new VertexDragSnapper(m_tool->handleManager(), m_handleOffset);
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                MovePartBase::doRender(inputState, renderContext, renderBatch);
                
                if (!thisToolDragging()) {
                    const Model::Hit& hit = findDraggableHandle(inputState);
                    if (hit.hasType(EdgeHandleManager::HandleHit | FaceHandleManager::HandleHit)) {
                        const Vec3 handle = m_tool->getHandlePosition(hit);
                        if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                            m_tool->renderHandle(renderContext, renderBatch, handle, pref(Preferences::SelectedHandleColor));
                        else
                            m_tool->renderHandle(renderContext, renderBatch, handle);
                        m_tool->renderHighlight(renderContext, renderBatch, handle);
                    }
                }
            }
        private:
            SnapType snapType(const InputState& inputState) const {
                if (inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd))
                    return ST_Absolute;
                return ST_Relative;
            }
        private:
            const Model::Hit& doFindDraggableHandle(const InputState& inputState) const override {
                return findHandleHit(inputState);
            }
        };
        
        VertexToolController::VertexToolController(VertexTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveVertexPart(tool));
            addController(new SelectVertexPart(tool));
        }
    }
}
