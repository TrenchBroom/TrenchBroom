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

#ifndef __TrenchBroom__MoveTexturesCommand__
#define __TrenchBroom__MoveTexturesCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class MoveTexturesCommand : public Command {
        public:
            static const CommandType Type;
            typedef TrenchBroom::shared_ptr<MoveTexturesCommand> Ptr;
        private:
            View::MapDocumentWPtr m_document;
            Model::BrushFaceList m_faces;
            const Vec3 m_up;
            const Vec3 m_right;
            Vec2f m_offset;
        public:
            static Ptr moveTextures(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& up, const Vec3& right, const Vec2f& offset);
        private:
            MoveTexturesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& up, const Vec3& right, const Vec2f& offset);
            
            bool doPerformDo();
            bool doPerformUndo();
            bool doCollateWith(Command::Ptr command);
            
            void moveTextures(const Vec2f& offset);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveTexturesCommand__) */
