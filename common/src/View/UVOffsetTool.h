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

#include "View/Tool.h"
#include "View/ToolController.h"

#include <memory>

namespace TrenchBroom {
    namespace View {
        class MapDocument;
        class UVViewHelper;

        class UVOffsetTool : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public Tool {
        private:
            std::weak_ptr<MapDocument> m_document;
            const UVViewHelper& m_helper;
            vm::vec2f m_lastPoint;
        public:
            UVOffsetTool(std::weak_ptr<MapDocument> document, const UVViewHelper& helper);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            vm::vec2f computeHitPoint(const vm::ray3& ray) const;
            vm::vec2f snapDelta(const vm::vec2f& delta) const;

            bool doCancel() override;
        };
    }
}

