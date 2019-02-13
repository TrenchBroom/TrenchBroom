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

#include "TrenchBroom.h"
#include "SharedPointer.h"
#include "View/SnapshotCommand.h"

namespace TrenchBroom {
    namespace View {
        class SnapBrushVerticesCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SnapBrushVerticesCommand>;
        private:
            FloatType m_snapTo;
        public:
            static SnapBrushVerticesCommand::Ptr snap(FloatType snapTo);
        private:
            explicit SnapBrushVerticesCommand(FloatType snapTo);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* defined(TrenchBroom_SnapBrushVerticesCommand) */
