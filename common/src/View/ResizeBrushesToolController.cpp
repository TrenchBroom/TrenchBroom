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

#include "ResizeBrushesToolController.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/ResizeBrushesTool.h"

namespace TrenchBroom {
    namespace View {
        ResizeBrushesToolController::ResizeBrushesToolController(ResizeBrushesTool* tool) :
        m_tool{tool},
        m_mode{Mode::Resize} {
            ensure(m_tool != nullptr, "tool is null");
        }

        ResizeBrushesToolController::~ResizeBrushesToolController() = default;

        Tool& ResizeBrushesToolController::tool() {
            return *m_tool;
        }

        const Tool& ResizeBrushesToolController::tool() const {
            return *m_tool;
        }

        void ResizeBrushesToolController::pick(const InputState& inputState, Model::PickResult& pickResult) {
            if (handleInput(inputState)) {
                const Model::Hit hit = doPick(inputState.pickRay(), pickResult);
                if (hit.isMatch()) {
                    pickResult.addHit(hit);
                }
            }
        }

        void ResizeBrushesToolController::modifierKeyChange(const InputState& inputState) {
            if (!anyToolDragging(inputState)) {
                m_tool->updateProposedDragHandles(inputState.pickResult());
            }
        }

        void ResizeBrushesToolController::mouseMove(const InputState& inputState) {
            if (handleInput(inputState) && !anyToolDragging(inputState)) {
                m_tool->updateProposedDragHandles(inputState.pickResult());
            }
        }

        namespace {
            class ResizeToolDragTracker : public DragTracker {
            private:
                using DragFunction = std::function<bool(const InputState&)>;

                ResizeBrushesTool& m_tool;
                DragFunction m_drag;
            public:
                ResizeToolDragTracker(ResizeBrushesTool& tool, DragFunction drag) :
                m_tool{tool},
                m_drag{std::move(drag)} {}

                bool drag(const InputState& inputState) override {
                    return m_drag(inputState);
                }

                void end(const InputState& inputState) override {
                    m_tool.commit();
                    m_tool.updateProposedDragHandles(inputState.pickResult());
                }

                void cancel() override {
                    m_tool.cancel();
                }

                void setRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const override {
                    renderContext.setForceShowSelectionGuide();
                }
            };
        }

        std::unique_ptr<DragTracker> ResizeBrushesToolController::acceptMouseDrag(const InputState& inputState) {
            if (!handleInput(inputState)) {
                return nullptr;
            }
            // NOTE: We check for MBLeft here rather than in handleInput because we want the
            // yellow highlight to render as a preview when Shift is down, before you press MBLeft. 
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return nullptr;
            }

            m_tool->updateProposedDragHandles(inputState.pickResult());
            m_mode = inputState.modifierKeysDown(ModifierKeys::MKAlt) ? Mode::MoveFace : Mode::Resize;
            if (m_mode == Mode::Resize) {
                const auto split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
                if (m_tool->beginResize(inputState.pickResult(), split)) {
                    return std::make_unique<ResizeToolDragTracker>(*m_tool, [&](const InputState& inputState_) {
                        return m_tool->resize(inputState_.pickRay(), inputState_.camera());
                    });
                }
            } else {
                if (m_tool->beginMove(inputState.pickResult())) {
                    return std::make_unique<ResizeToolDragTracker>(*m_tool, [&](const InputState& inputState_) {
                        return m_tool->move(inputState_.pickRay(), inputState_.camera());
                    });
                }
            }

            return nullptr;
        }

        static Renderer::DirectEdgeRenderer buildEdgeRenderer(const std::vector<Model::BrushFaceHandle>& visualHandles) {
            using Vertex = Renderer::GLVertexTypes::P3::Vertex;
            auto vertices = std::vector<Vertex>{};

            for (const auto& dragFaceHandle : visualHandles) {
                const auto& dragFace = dragFaceHandle.face();
                for (const auto* edge : dragFace.edges()) {
                    vertices.emplace_back(vm::vec3f{edge->firstVertex()->position()});
                    vertices.emplace_back(vm::vec3f{edge->secondVertex()->position()});
                }
            }

            return Renderer::DirectEdgeRenderer(Renderer::VertexArray::move(std::move(vertices)), Renderer::PrimType::Lines);
        }

        void ResizeBrushesToolController::render(const InputState&, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            if (m_tool->hasVisualHandles()) {
                Renderer::DirectEdgeRenderer edgeRenderer = buildEdgeRenderer(m_tool->visualHandles());
                edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
            }
        }

        bool ResizeBrushesToolController::cancel() {
            return false;
        }

        bool ResizeBrushesToolController::handleInput(const InputState& inputState) const {
            return (doHandleInput(inputState) && m_tool->applies());
        }

        ResizeBrushesToolController2D::ResizeBrushesToolController2D(ResizeBrushesTool* tool) :
        ResizeBrushesToolController{tool} {}

        Model::Hit ResizeBrushesToolController2D::doPick(const vm::ray3& pickRay, const Model::PickResult& pickResult) {
            return m_tool->pick2D(pickRay, pickResult);
        }

        bool ResizeBrushesToolController2D::doHandleInput(const InputState& inputState) const {
            return (inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd) ||
                    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKAlt));
        }

        ResizeBrushesToolController3D::ResizeBrushesToolController3D(ResizeBrushesTool* tool) :
        ResizeBrushesToolController{tool} {}

        Model::Hit ResizeBrushesToolController3D::doPick(const vm::ray3& pickRay, const Model::PickResult& pickResult) {
            return m_tool->pick3D(pickRay, pickResult);
        }

        bool ResizeBrushesToolController3D::doHandleInput(const InputState& inputState) const {
            return (inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd));
        }
    }
}
