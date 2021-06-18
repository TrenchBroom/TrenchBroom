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

#include "CreateEntityToolController.h"

#include "Ensure.h"
#include "View/CreateEntityTool.h"
#include "View/DropTracker.h"
#include "View/InputState.h"

#include <kdl/string_utils.h>

#include <functional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        CreateEntityToolController::CreateEntityToolController(CreateEntityTool* tool) :
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
        }

        CreateEntityToolController::~CreateEntityToolController() = default;

        Tool& CreateEntityToolController::tool() {
            return *m_tool;
        }

        const Tool& CreateEntityToolController::tool() const {
            return *m_tool;
        }

        std::unique_ptr<DropTracker> CreateEntityToolController::acceptDrop(const InputState& inputState, const std::string& payload) {
            const auto parts = kdl::str_split(payload, ":");
            if (parts.size() != 2 || parts[0] != "entity") {
                return nullptr;
            }

            if (!m_tool->createEntity(parts[1])) {
                return nullptr;
            }

            return createDropTracker(inputState);
        }

        bool CreateEntityToolController::cancel() {
            return false;
        }

        namespace {
            class CreateEntityDropTracker : public DropTracker {
            private:
                CreateEntityTool& m_tool;
                std::function<void(const InputState&, CreateEntityTool& tool)> m_updateEntityPosition;
            public:
                explicit CreateEntityDropTracker(const InputState& inputState, CreateEntityTool& tool, std::function<void(const InputState&, CreateEntityTool& tool)> updateEntityPosition) :
                m_tool{tool},
                m_updateEntityPosition{std::move(updateEntityPosition)} {
                    m_updateEntityPosition(inputState, m_tool);
                }

                bool move(const InputState& inputState) override {
                    m_updateEntityPosition(inputState, m_tool);
                    return true;
                }

                bool drop(const InputState&) override {
                    m_tool.commitEntity();
                    return true;
                }

                void leave(const InputState&) override {
                    m_tool.removeEntity();
                }

            };
        }

        CreateEntityToolController2D::CreateEntityToolController2D(CreateEntityTool* tool) :
        CreateEntityToolController(tool) {}

        std::unique_ptr<DropTracker> CreateEntityToolController2D::createDropTracker(const InputState& inputState) const {
            return std::make_unique<CreateEntityDropTracker>(inputState, *m_tool, [](const auto& is, auto& t) { t.updateEntityPosition2D(is.pickRay()); });
        }

        CreateEntityToolController3D::CreateEntityToolController3D(CreateEntityTool* tool) :
        CreateEntityToolController(tool) {}

        std::unique_ptr<DropTracker> CreateEntityToolController3D::createDropTracker(const InputState& inputState) const {
            return std::make_unique<CreateEntityDropTracker>(inputState, *m_tool, [](const auto& is, auto& t) { t.updateEntityPosition3D(is.pickRay(), is.pickResult()); });
        }
    }
}
