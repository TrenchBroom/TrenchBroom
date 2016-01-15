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

#ifndef TrenchBroom_ShearTexturesCommand
#define TrenchBroom_ShearTexturesCommand

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace View {
        class ShearTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<ShearTexturesCommand> Ptr;
        private:
            Vec2f m_factors;
        public:
            static Ptr shear(const Vec2f& factors);
        private:
            ShearTexturesCommand(const Vec2f& factors);
            
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool shearTextures(MapDocumentCommandFacade* document, const Vec2f& factors);
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_ShearTexturesCommand) */
