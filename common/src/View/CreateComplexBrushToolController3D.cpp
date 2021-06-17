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
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/Polyhedron3.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/CreateComplexBrushTool.h"
#include "View/Grid.h"
#include "View/HandleDragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/vector_utils.h>

#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>
#include <vecmath/line.h>

#include <cassert>
#include <algorithm>
#include <memory>

namespace TrenchBroom {
    namespace View {
        namespace {
            class Part {
            protected:
                CreateComplexBrushTool* m_tool;
                Model::Polyhedron3 m_oldPolyhedron;
            protected:
                explicit Part(CreateComplexBrushTool* tool) :
                m_tool{tool} {
                    ensure(m_tool != nullptr, "tool is null");
                }
            public:
                virtual ~Part() = default;
            };

            class DrawFaceDragDelegate : public HandleDragTrackerDelegate {
            private:
                CreateComplexBrushTool& m_tool;
                vm::plane3 m_plane;
                const Model::Polyhedron3 m_oldPolyhedron;
            public:
                DrawFaceDragDelegate(CreateComplexBrushTool& tool, const vm::plane3 plane) :
                m_tool{tool},
                m_plane{plane},
                m_oldPolyhedron{m_tool.polyhedron()} {}

                HandlePositionProposer start(const InputState&, const vm::vec3& initialHandlePosition, const vm::vec3& handleOffset) {
                    using namespace Model::HitFilters;

                    updatePolyhedron(initialHandlePosition, initialHandlePosition);

                    return makeHandlePositionProposer(
                        makeSurfaceHandlePicker(type(Model::BrushNode::BrushHitType), handleOffset), 
                        makeIdentityHandleSnapper());
                }

                DragStatus drag(const InputState&, const DragState& dragState, const vm::vec3& proposedHandlePosition) {
                    updatePolyhedron(dragState.initialHandlePosition, proposedHandlePosition);
                    return DragStatus::Continue;
                }

                void end(const InputState&, const DragState&) {
                }

                void cancel(const DragState&) {
                    m_tool.update(m_oldPolyhedron);
                }
            private:
                void updatePolyhedron(const vm::vec3& initialHandlePosition, const vm::vec3& currentHandlePosition) {
                    const auto& grid = m_tool.grid();

                    const auto axis = vm::find_abs_max_component(m_plane.normal);
                    const auto swizzledPlane = vm::plane3{vm::swizzle(m_plane.anchor(), axis), vm::swizzle(m_plane.normal, axis)};
                    const auto theMin = vm::swizzle(grid.snapDown(vm::min(initialHandlePosition, currentHandlePosition)), axis);
                    const auto theMax = vm::swizzle(grid.snapUp  (vm::max(initialHandlePosition, currentHandlePosition)), axis);

                    const auto     topLeft2 = vm::vec2{theMin.x(), theMin.y()};
                    const auto    topRight2 = vm::vec2{theMax.x(), theMin.y()};
                    const auto  bottomLeft2 = vm::vec2{theMin.x(), theMax.y()};
                    const auto bottomRight2 = vm::vec2{theMax.x(), theMax.y()};

                    const auto newVertices = std::vector<vm::vec3>{
                        vm::unswizzle(vm::vec3{topLeft2,     swizzledPlane.zAt(topLeft2)},     axis),
                        vm::unswizzle(vm::vec3{topRight2,    swizzledPlane.zAt(topRight2)},    axis),
                        vm::unswizzle(vm::vec3{bottomLeft2,  swizzledPlane.zAt(bottomLeft2)},  axis),
                        vm::unswizzle(vm::vec3{bottomRight2, swizzledPlane.zAt(bottomRight2)}, axis)
                    };
                    
                    m_tool.update(Model::Polyhedron3{kdl::vec_concat(newVertices, m_oldPolyhedron.vertexPositions())});
                }
            };

            class DrawFacePart : public Part, public ToolController {
            private:
                vm::plane3 m_plane;
                vm::vec3 m_initialPoint;
            public:
                explicit DrawFacePart(CreateComplexBrushTool* tool) :
                Part{tool} {}
            private:
                Tool* doGetTool() override { return m_tool; }
                const Tool* doGetTool() const override { return m_tool; }

                std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override {
                    using namespace Model::HitFilters;

                    if (inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                        return nullptr;
                    }

                    const auto& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
                    const auto faceHandle = Model::hitToFaceHandle(hit);
                    if (!faceHandle) {
                        return nullptr;
                    }

                    const auto initialHandlePosition = hit.hitPoint();
                    const auto plane = faceHandle->face().boundary();
                    const auto handleOffset = vm::vec3::zero();
                    return createHandleDragTracker(DrawFaceDragDelegate{*m_tool, plane}, inputState, initialHandlePosition, handleOffset);
                }

                bool cancel() override { return false; }
            };

            class DuplicateFaceDragDelegate : public HandleDragTrackerDelegate {
            private:
                CreateComplexBrushTool& m_tool;
                vm::vec3 m_dragDir;
                const Model::Polyhedron3 m_oldPolyhedron;
            public:
                DuplicateFaceDragDelegate(CreateComplexBrushTool& tool, const vm::vec3 dragDir) :
                m_tool{tool},
                m_dragDir{dragDir},
                m_oldPolyhedron{m_tool.polyhedron()} {}

                HandlePositionProposer start(const InputState&, const vm::vec3& initialHandlePosition, const vm::vec3& handleOffset) {
                    using namespace Model::HitFilters;

                    const auto line = vm::line3{initialHandlePosition, m_dragDir};
                    return makeHandlePositionProposer(
                        makeLineHandlePicker(line, handleOffset), 
                        makeRelativeLineHandleSnapper(m_tool.grid(), line));
                }

                DragStatus drag(const InputState&, const DragState& dragState, const vm::vec3& proposedHandlePosition) {
                    auto polyhedron = m_oldPolyhedron;
                    assert(polyhedron.polygon());

                    const auto delta = proposedHandlePosition - dragState.initialHandlePosition;

                    const auto* face = m_oldPolyhedron.faces().front();
                    const auto points = face->vertexPositions() + delta;

                    m_tool.update(Model::Polyhedron3{kdl::vec_concat(points, m_oldPolyhedron.vertexPositions())});

                    return DragStatus::Continue;
                }

                void end(const InputState&, const DragState&) {
                }

                void cancel(const DragState&) {
                    m_tool.update(m_oldPolyhedron);
                }
            };

            class DuplicateFacePart : public Part, public ToolController {
            private:
                vm::vec3 m_dragDir;
            public:
                DuplicateFacePart(CreateComplexBrushTool* tool) :
                Part{tool} {}
            private:
                Tool* doGetTool() override { return m_tool; }
                const Tool* doGetTool() const override { return m_tool; }

                std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override {
                    using namespace Model::HitFilters;

                    if (!inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                        return nullptr;
                    }

                    if (!m_tool->polyhedron().polygon()) {
                        return nullptr;
                    }

                    const auto hit = m_tool->polyhedron().pickFace(inputState.pickRay());
                    if (!hit.isMatch()) {
                        return nullptr;
                    }

                    const vm::vec3 initialHandlePosition = vm::point_at_distance(inputState.pickRay(), hit.distance);
                    const vm::vec3 normal = hit.face->normal();
                    const auto handleOffset = vm::vec3::zero();

                    return createHandleDragTracker(DuplicateFaceDragDelegate{*m_tool, normal}, inputState, initialHandlePosition, handleOffset);
                }
                bool cancel() override { return false; }
            };
        }

        CreateComplexBrushToolController3D::CreateComplexBrushToolController3D(CreateComplexBrushTool* tool) :
        m_tool{tool} {
            ensure(m_tool != nullptr, "tool is null");
            addController(std::make_unique<DrawFacePart>(m_tool));
            addController(std::make_unique<DuplicateFacePart>(m_tool));
        }

        Tool* CreateComplexBrushToolController3D::doGetTool() {
            return m_tool;
        }

        const Tool* CreateComplexBrushToolController3D::doGetTool() const {
            return m_tool;
        }

        bool CreateComplexBrushToolController3D::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft)) {
                return false;
            }
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_No)) {
                return false;
            }

            using namespace Model::HitFilters;
            const Model::Hit& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
            if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                const Grid& grid = m_tool->grid();

                const Model::BrushFace& face = faceHandle->face();
                const vm::vec3 snapped = grid.snap(hit.hitPoint(), face.boundary());

                m_tool->update(Model::Polyhedron3{kdl::vec_concat(std::vector<vm::vec3>({snapped}), m_tool->polyhedron().vertexPositions())});

                return true;
            } else {
                return false;
            }
        }

        bool CreateComplexBrushToolController3D::doMouseDoubleClick(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft)) {
                return false;
            }
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_No)) {
                return false;
            }

            using namespace Model::HitFilters;
            const Model::Hit& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
            if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                const Model::BrushFace& face = faceHandle->face();

                m_tool->update(Model::Polyhedron3{kdl::vec_concat(face.vertexPositions(), m_tool->polyhedron().vertexPositions())});

                return true;
            } else {
                return false;
            }
        }

        bool CreateComplexBrushToolController3D::doShouldHandleMouseDrag(const InputState& inputState) const {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft)) {
                return false;
            }
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare)) {
                return false;
            }
            
            return true;
        }

        void CreateComplexBrushToolController3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);

            const Model::Polyhedron3& polyhedron = m_tool->polyhedron();
            if (!polyhedron.empty()) {
                auto renderService = Renderer::RenderService{renderContext, renderBatch};
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
                        auto pos3f = kdl::vec_transform(pos3, [](const auto& pos) { return vm::vec3f{pos}; });

                        renderService.setForegroundColor(Color{pref(Preferences::HandleColor), 0.5f});
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
            }

            m_tool->update(Model::Polyhedron3());
            return true;
        }
    }
}
