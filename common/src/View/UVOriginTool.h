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

#include "Model/HitType.h"
#include "Renderer/GLVertexType.h"
#include "View/Tool.h"
#include "View/ToolController.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class UVViewHelper;

        class UVOriginTool : public ToolControllerBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy>, public Tool {
        public:
            static const Model::HitType::Type XHandleHitType;
            static const Model::HitType::Type YHandleHitType;
        private:
            static const FloatType MaxPickDistance;
            static const float OriginHandleRadius;

            using EdgeVertex = Renderer::GLVertexTypes::P3C4::Vertex;

            UVViewHelper& m_helper;

            vm::vec2f m_lastPoint;
            vm::vec2f m_selector;
        public:
            explicit UVOriginTool(UVViewHelper& helper);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void computeOriginHandles(vm::line3& xHandle, vm::line3& yHandle) const;

            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;

            vm::vec2f computeHitPoint(const vm::ray3& ray) const;
            vm::vec2f snapDelta(const vm::vec2f& delta) const;

            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            void renderLineHandles(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            std::vector<EdgeVertex> getHandleVertices(const InputState& inputState) const;

            class RenderOrigin;
            void renderOriginHandle(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            bool renderHighlight(const InputState& inputState) const;

            bool doCancel() override;
        };
    }
}

