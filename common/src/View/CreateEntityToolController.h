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

#include "View/ToolController.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        class CreateEntityTool;

        class CreateEntityToolController : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, NoRenderPolicy, DropPolicy> {
        protected:
            CreateEntityTool* m_tool;
        protected:
            CreateEntityToolController(CreateEntityTool* tool);
        public:
            virtual ~CreateEntityToolController() override;
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            bool doDragEnter(const InputState& inputState, const std::string& payload) override;
            bool doDragMove(const InputState& inputState) override;
            void doDragLeave(const InputState& inputState) override;
            bool doDragDrop(const InputState& inputState) override;
            void updateEntityPosition(const InputState& inputState);

            bool doCancel() override;
        private:
            virtual void doUpdateEntityPosition(const InputState& inputState) = 0;
        };

        class CreateEntityToolController2D : public CreateEntityToolController {
        public:
            CreateEntityToolController2D(CreateEntityTool* tool);
        private:
            void doUpdateEntityPosition(const InputState& inputState) override;
        };

        class CreateEntityToolController3D : public CreateEntityToolController {
        public:
            CreateEntityToolController3D(CreateEntityTool* tool);
        private:
            void doUpdateEntityPosition(const InputState& inputState) override;
        };
    }
}

