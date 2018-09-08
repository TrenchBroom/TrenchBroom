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

#ifndef TrenchBroom_ResizeBrushesCommand
#define TrenchBroom_ResizeBrushesCommand

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/SnapshotCommand.h"

namespace TrenchBroom {
    namespace View {
        class ResizeBrushesCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<ResizeBrushesCommand> Ptr;
        private:
            polygon3::List m_faces;
            polygon3::List m_newFaces;
            vm::vec3 m_delta;
        public:
            static Ptr resize(const polygon3::List& faces, const vm::vec3& delta);
        private:
            ResizeBrushesCommand(const polygon3::List& faces, const vm::vec3& delta);
            
            bool doPerformDo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            
            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* defined(TrenchBroom_ResizeBrushesCommand) */
