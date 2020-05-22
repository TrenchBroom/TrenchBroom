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

#include "CreateComplexBrushToolController3D.h"

#include "FloatType.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/Polyhedron3.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/CreateComplexBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/vector_utils.h>

#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>
#include <vecmath/line.h>

#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace View {
        class CreateComplexBrushToolController3D::Part {
        protected:
            CreateComplexBrushTool* m_tool;
            Model::Polyhedron3 m_oldPolyhedron;
        protected:
            explicit Part(CreateComplexBrushTool* tool) :
            m_tool(tool),
            m_oldPolyhedron() {
                ensure(m_tool != nullptr, "tool is null");
            }
        public:
            virtual ~Part() = default;
        };

        class CreateComplexBrushToolController3D::DrawFacePart : public Part, public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy> {
        private:
            vm::plane3 m_plane;
            vm::vec3 m_initialPoint;
        public:
            explicit DrawFacePart(CreateComplexBrushTool* tool) :
            Part(tool) {}
        private:
            Tool* doGetTool() override { return m_tool; }
            const Tool* doGetTool() const override { return m_tool; }

            DragInfo doStartDrag(const InputState& inputState) override {
                if (inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                    return DragInfo();
                }

                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
                if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                    m_oldPolyhedron = m_tool->polyhedron();

                    const Model::BrushFace& face = faceHandle->face();
                    m_plane = face.boundary();
                    m_initialPoint = hit.hitPoint();
                    updatePolyhedron(m_initialPoint);

                    auto* restricter = new SurfaceDragRestricter();
                    restricter->setPickable(true);
                    restricter->setType(Model::BrushNode::BrushHitType);
                    restricter->setOccluded(true);
                    return DragInfo(restricter, new NoDragSnapper(), m_initialPoint);
                } else {
                    return DragInfo();
                }
            }

            DragResult doDrag(const InputState&, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) override {
                updatePolyhedron(nextHandlePosition);
                return DR_Continue;
            }

            void doEndDrag(const InputState&) override {
            }

            void doCancelDrag() override {
                m_tool->update(m_oldPolyhedron);
            }

            bool doCancel() override { return false; }
        private:
            void updatePolyhedron(const vm::vec3& current) {
                const auto& grid = m_tool->grid();

                const auto axis = vm::find_abs_max_component(m_plane.normal);
                const vm::plane3 swizzledPlane(swizzle(m_plane.anchor(), axis), swizzle(m_plane.normal, axis));
                const auto theMin = swizzle(grid.snapDown(min(m_initialPoint, current)), axis);
                const auto theMax = swizzle(grid.snapUp  (max(m_initialPoint, current)), axis);

                const vm::vec2     topLeft2(theMin.x(), theMin.y());
                const vm::vec2    topRight2(theMax.x(), theMin.y());
                const vm::vec2  bottomLeft2(theMin.x(), theMax.y());
                const vm::vec2 bottomRight2(theMax.x(), theMax.y());

                const std::vector<vm::vec3> newVertices({
                    unswizzle(vm::vec3(topLeft2,     swizzledPlane.zAt(topLeft2)),     axis),
                    unswizzle(vm::vec3(topRight2,    swizzledPlane.zAt(topRight2)),    axis),
                    unswizzle(vm::vec3(bottomLeft2,  swizzledPlane.zAt(bottomLeft2)),  axis),
                    unswizzle(vm::vec3(bottomRight2, swizzledPlane.zAt(bottomRight2)), axis)
                });
                
                m_tool->update(Model::Polyhedron3(kdl::vec_concat(newVertices, m_oldPolyhedron.vertexPositions())));
            }
        };

        class CreateComplexBrushToolController3D::DuplicateFacePart : public Part, public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy> {
        private:
            vm::vec3 m_dragDir;
        public:
            DuplicateFacePart(CreateComplexBrushTool* tool) :
            Part(tool) {}
        private:
            Tool* doGetTool() override { return m_tool; }
            const Tool* doGetTool() const override { return m_tool; }

            DragInfo doStartDrag(const InputState& inputState) override {
                if (!inputState.modifierKeysDown(ModifierKeys::MKShift))
                    return DragInfo();

                if (!m_tool->polyhedron().polygon())
                    return DragInfo();

                m_oldPolyhedron = m_tool->polyhedron();

                const Model::Polyhedron3::FaceHit hit = m_oldPolyhedron.pickFace(inputState.pickRay());
                if (!hit.isMatch())
                    return DragInfo();

                const vm::vec3 origin    = vm::point_at_distance(inputState.pickRay(), hit.distance);
                const vm::vec3 direction = hit.face->normal();

                const vm::line3 line(origin, direction);
                m_dragDir = line.direction;

                return DragInfo(new LineDragRestricter(line), new NoDragSnapper(), origin);
            }

            DragResult doDrag(const InputState&, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) override {
                auto polyhedron = m_oldPolyhedron;
                assert(polyhedron.polygon());

                const auto& grid = m_tool->grid();

                const auto rayDelta        = nextHandlePosition - initialHandlePosition();
                const auto rayAxis         = vm::get_abs_max_component_axis(m_dragDir);
                const auto axisDistance    = vm::dot(rayDelta, rayAxis);
                const auto snappedDistance = grid.snap(axisDistance);
                const auto snappedRayDist  = vm::dot(m_dragDir, rayAxis * snappedDistance);
                const auto snappedRayDelta = snappedRayDist * m_dragDir;

                const auto* face = m_oldPolyhedron.faces().front();
                const auto points = face->vertexPositions() + snappedRayDelta;

                m_tool->update(Model::Polyhedron3(kdl::vec_concat(points, m_oldPolyhedron.vertexPositions())));

                return DR_Continue;
            }

            void doEndDrag(const InputState&) override {
            }

            void doCancelDrag() override {
                m_tool->update(m_oldPolyhedron);
            }

            bool doCancel() override { return false; }
        };

        CreateComplexBrushToolController3D::CreateComplexBrushToolController3D(CreateComplexBrushTool* tool) :
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
            addController(new DrawFacePart(m_tool));
            addController(new DuplicateFacePart(m_tool));
        }

        Tool* CreateComplexBrushToolController3D::doGetTool() {
            return m_tool;
        }

        const Tool* CreateComplexBrushToolController3D::doGetTool() const {
            return m_tool;
        }

        bool CreateComplexBrushToolController3D::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_No))
                return false;

            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
            if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                const Grid& grid = m_tool->grid();

                const Model::BrushFace& face = faceHandle->face();
                const vm::vec3 snapped = grid.snap(hit.hitPoint(), face.boundary());

                m_tool->update(Model::Polyhedron3(kdl::vec_concat(std::vector<vm::vec3>({snapped}), m_tool->polyhedron().vertexPositions())));

                return true;
            } else {
                return false;
            }
        }

        bool CreateComplexBrushToolController3D::doMouseDoubleClick(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_No))
                return false;

            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
            if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                const Model::BrushFace& face = faceHandle->face();

                m_tool->update(Model::Polyhedron3(kdl::vec_concat(face.vertexPositions(), m_tool->polyhedron().vertexPositions())));

                return true;
            } else {
                return false;
            }
        }

        bool CreateComplexBrushToolController3D::doShouldHandleMouseDrag(const InputState& inputState) const {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft)) {
                return false;
            } else if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare)) {
                return false;
            } else {
                return true;
            }
        }

        void CreateComplexBrushToolController3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);

            const Model::Polyhedron3& polyhedron = m_tool->polyhedron();
            if (!polyhedron.empty()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                renderService.setLineWidth(2.0f);

                for (const auto* edge : polyhedron.edges()) {
                    renderService.renderLine(vm::vec3f(edge->firstVertex()->position()), vm::vec3f(edge->secondVertex()->position()));
                }

                for (const auto* vertex : polyhedron.vertices()) {
                    renderService.renderHandle(vm::vec3f(vertex->position()));
                }

                if (polyhedron.polygon() && inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                    const auto hit = polyhedron.pickFace(inputState.pickRay());
                    if (hit.isMatch()) {
                        const auto* face = polyhedron.faces().front();
                        const auto pos3 = face->vertexPositions();
                        std::vector<vm::vec3f> pos3f(pos3.size());
                        for (size_t i = 0; i < pos3.size(); ++i) {
                            pos3f[i] = vm::vec3f(pos3[i]);
                        }

                        renderService.setForegroundColor(Color(pref(Preferences::HandleColor), 0.5f));
                        renderService.renderFilledPolygon(pos3f);

                        std::reverse(std::begin(pos3f), std::end(pos3f));
                        renderService.renderFilledPolygon(pos3f);
                    }
                }
            }
        }

        bool CreateComplexBrushToolController3D::doCancel() {
            const Model::Polyhedron3& polyhedron = m_tool->polyhedron();
            if (polyhedron.empty()) {
                return false;
            } else {
                m_tool->update(Model::Polyhedron3());
                return true;
            }
        }
    }
}
