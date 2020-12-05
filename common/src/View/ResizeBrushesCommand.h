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

#include "Macros.h"
#include "FloatType.h"
#include "View/SnapshotCommand.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class ResizeBrushesCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
        private:
            std::vector<vm::polygon3> m_faces;
            std::vector<vm::polygon3> m_newFaces;
            vm::vec3 m_delta;
        public:
            static std::unique_ptr<ResizeBrushesCommand> resize(const std::vector<vm::polygon3>& faces, const vm::vec3& delta);

            ResizeBrushesCommand(const std::vector<vm::polygon3>& faces, const vm::vec3& delta);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(ResizeBrushesCommand)
        };
    }
}

