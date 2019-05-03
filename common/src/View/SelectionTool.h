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

#ifndef TrenchBroom_SelectionTool
#define TrenchBroom_SelectionTool

#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }

    namespace View {
        class InputState;

        /**
         * Implements the Group picking logic: if `node` is inside a (possibly nested chain of)
         * closed group(s), the outermost closed group is returned. Otherwise, `node` itself is returned.
         *
         * This is used to implement the UI where clicking on a brush inside a group selects the group.
         */
        Model::Node* outermostClosedGroupOrNode(Model::Node* node);

        /**
         * Applies the group picking logic of outermostClosedGroupOrNode() to a list of hits.
         * The order of the hits is preserved, but if multiple hits map to the same group, that group
         * will only be listed once in the output.
         */
        Model::NodeList hitsToNodesWithGroupPicking(const Model::Hit::List& hits);

        class SelectionTool : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy>, public Tool {
        private:
            MapDocumentWPtr m_document;
        public:
            SelectionTool(MapDocumentWPtr document);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            bool doMouseClick(const InputState& inputState) override;
            bool doMouseDoubleClick(const InputState& inputState) override;

            bool handleClick(const InputState& inputState) const;
            bool isFaceClick(const InputState& inputState) const;
            bool isMultiClick(const InputState& inputState) const;

            const Model::Hit& firstHit(const InputState& inputState, Model::Hit::HitType type) const;

            Model::NodeList collectSelectableChildren(const Model::EditorContext& editorContext, const Model::Node* node) const;

            void doMouseScroll(const InputState& inputState) override;
            void adjustGrid(const InputState& inputState);
            void drillSelection(const InputState& inputState);

            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;

            bool doCancel() override;
        };
    }
}

#endif /* defined(TrenchBroom_SelectionTool) */
