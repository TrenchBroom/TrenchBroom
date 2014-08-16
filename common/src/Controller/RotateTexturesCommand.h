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

#ifndef __TrenchBroom__RotateTexturesCommand__
#define __TrenchBroom__RotateTexturesCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class RotateTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<RotateTexturesCommand> Ptr;
        private:
            Model::BrushFaceList m_faces;
            float m_angle;
        public:
            static Ptr rotateTextures(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, float angle);
        private:
            RotateTexturesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, float angle);
            
            bool doPerformDo();
            bool doPerformUndo();

            bool doIsRepeatable(View::MapDocumentSPtr document) const;
            Command* doRepeat(View::MapDocumentSPtr document) const;

            bool doCollateWith(Command::Ptr command);
            
            void rotateTextures(float angle);
            
            static String makeName(const Model::BrushFaceList& faces);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateTexturesCommand__) */
