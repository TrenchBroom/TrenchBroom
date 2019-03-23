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

#ifndef TrenchBroom_UVScaleTool
#define TrenchBroom_UVScaleTool

#include "Model/Hit.h"
#include "Renderer/GLVertexType.h"
#include "View/Tool.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class PickResult;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class UVViewHelper;

        class UVScaleTool : public ToolControllerBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy>, public Tool {
        private:
            static const Model::Hit::HitType XHandleHit;
            static const Model::Hit::HitType YHandleHit;
        private:
            using EdgeVertex = Renderer::GLVertexTypes::P3::Vertex;

            MapDocumentWPtr m_document;
            UVViewHelper& m_helper;

            vm::vec2i m_handle;
            vm::vec2b m_selector;
            vm::vec2f m_lastHitPoint; // in non-scaled, non-translated texture coordinates
        public:
            UVScaleTool(MapDocumentWPtr document, UVViewHelper& helper);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            vm::vec2i getScaleHandle(const Model::Hit& xHit, const Model::Hit& yHit) const;
            vm::vec2f getHitPoint(const vm::ray3& pickRay) const;

            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            vm::vec2f getScaledTranslatedHandlePos() const;
            vm::vec2f getHandlePos() const;
            vm::vec2f snap(const vm::vec2f& position) const;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            EdgeVertex::List getHandleVertices(const Model::PickResult& pickResult) const;

            bool doCancel() override;
        };
    }
}

#endif /* defined(TrenchBroom_UVScaleTool) */
