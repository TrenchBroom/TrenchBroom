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

#ifndef TrenchBroom_UVShearTool
#define TrenchBroom_UVShearTool

#include "Model/HitType.h"
#include "View/Tool.h"
#include "View/ToolController.h"

#include <memory>

namespace TrenchBroom {
    namespace View {
        class MapDocument;
        class UVViewHelper;

        class UVShearTool : public ToolControllerBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public Tool {
        private:
            static const Model::HitType::Type XHandleHitType;
            static const Model::HitType::Type YHandleHitType;
        private:
            std::weak_ptr<MapDocument> m_document;
            UVViewHelper& m_helper;

            vm::vec2b m_selector;
            vm::vec3 m_xAxis;
            vm::vec3 m_yAxis;
            vm::vec2f m_initialHit;
            vm::vec2f m_lastHit;
        public:
            UVShearTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            vm::vec2f getHit(const vm::ray3& pickRay) const;

            bool doCancel() override;
        };
    }
}

#endif /* defined(TrenchBroom_UVShearTool) */
