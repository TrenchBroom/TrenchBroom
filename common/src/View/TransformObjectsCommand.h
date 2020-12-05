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

#include "FloatType.h"
#include "Macros.h"
#include "View/SnapshotCommand.h"

#include <vecmath/mat.h>
#include <vecmath/util.h>

#include <string>

namespace TrenchBroom {
    namespace View {
        class TransformObjectsCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
        private:
            enum class Action {
                Translate,
                Rotate,
                Flip,
                Shear,
                Scale
            };

            Action m_action;
            vm::mat4x4 m_transform;
            bool m_lockTextures;
        public:
            static std::unique_ptr<TransformObjectsCommand> translate(const vm::vec3& delta, bool lockTextures);
            static std::unique_ptr<TransformObjectsCommand> rotate(const vm::vec3& center, const vm::vec3& axis, FloatType angle, bool lockTextures);
            static std::unique_ptr<TransformObjectsCommand> scale(const vm::bbox3& oldBBox, const vm::bbox3& newBBox, bool lockTextures);
            static std::unique_ptr<TransformObjectsCommand> scale(const vm::vec3& center, const vm::vec3& scaleFactors, bool lockTextures);
            static std::unique_ptr<TransformObjectsCommand> shearBBox(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta, bool lockTextures);
            static std::unique_ptr<TransformObjectsCommand> flip(const vm::vec3& center, vm::axis::type axis, bool lockTextures);

            TransformObjectsCommand(Action action, const std::string& name, const vm::mat4x4& transform, bool lockTextures);
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(TransformObjectsCommand)
        };
    }
}

#endif /* defined(TrenchBroom_TransformObjectsCommand) */
