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

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class DragTracker;
        class UVViewHelper;

        class UVOriginTool : public ToolControllerBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, RenderPolicy, NoDropPolicy>, public Tool {
        public:
            static const Model::HitType::Type XHandleHitType;
            static const Model::HitType::Type YHandleHitType;

            static const FloatType MaxPickDistance;
            static const float OriginHandleRadius;
        private:
            UVViewHelper& m_helper;
        public:
            explicit UVOriginTool(UVViewHelper& helper);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doCancel() override;
        };
    }
}

