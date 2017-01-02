/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/VertexCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class VertexHandleManager;
        
        class SnapBrushVerticesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<SnapBrushVerticesCommand> Ptr;
        private:
            size_t m_snapTo;
            Model::Snapshot* m_snapshot;
        public:
            static SnapBrushVerticesCommand::Ptr snap(size_t snapTo);
        private:
            SnapBrushVerticesCommand(size_t snapTo);
        public:
            ~SnapBrushVerticesCommand();
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;

            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_SnapBrushVerticesCommand) */
