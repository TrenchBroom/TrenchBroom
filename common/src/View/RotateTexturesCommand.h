/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_RotateTexturesCommand
#define TrenchBroom_RotateTexturesCommand

#include "SharedPointer.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace View {
        class RotateTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<RotateTexturesCommand> Ptr;
        private:
            float m_angle;
        public:
            static Ptr rotate(float angle);
        private:
            RotateTexturesCommand(float angle);
            
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool rotateTextures(MapDocumentCommandFacade* document, float angle) const;
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_RotateTexturesCommand) */
