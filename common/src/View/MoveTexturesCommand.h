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

#ifndef TrenchBroom_MoveTexturesCommand
#define TrenchBroom_MoveTexturesCommand

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace View {
        class MoveTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<MoveTexturesCommand> Ptr;
        private:
            Vec3f m_cameraUp;
            Vec3f m_cameraRight;
            Vec2f m_delta;
        public:
            static Ptr move(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta);
        private:
            MoveTexturesCommand(const Vec3f& cameraUp, const Vec3f& cameraRight, const Vec2f& delta);

            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            void moveTextures(MapDocumentCommandFacade* document, const Vec2f& delta) const;
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_MoveTexturesCommand) */
