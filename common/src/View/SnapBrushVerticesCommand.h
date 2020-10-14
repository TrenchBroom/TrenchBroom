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

#ifndef TrenchBroom_SnapBrushVerticesCommand
#define TrenchBroom_SnapBrushVerticesCommand

#include "FloatType.h"
#include "Macros.h"
#include "View/SnapshotCommand.h"

namespace TrenchBroom {
    namespace View {
        class SnapBrushVerticesCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
        private:
            FloatType m_snapTo;
        public:
            static std::unique_ptr<SnapBrushVerticesCommand> snap(FloatType snapTo);

            explicit SnapBrushVerticesCommand(FloatType snapTo);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(SnapBrushVerticesCommand)
        };
    }
}

#endif /* defined(TrenchBroom_SnapBrushVerticesCommand) */
