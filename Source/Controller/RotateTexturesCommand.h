/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__RotateTexturesCommand__
#define __TrenchBroom__RotateTexturesCommand__

#include "Controller/Command.h"
#include "Model/FaceTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class RotateTexturesCommand : public DocumentCommand {
        protected:
            Model::FaceList m_faces;
            float m_angle;
            
            bool performDo();
            bool performUndo();
            
            RotateTexturesCommand(Model::MapDocument& document, const Model::FaceList& faces, const wxString& name, float angle);
        public:
            static RotateTexturesCommand* rotateClockwise(Model::MapDocument& document, const Model::FaceList& faces, float angle);
            static RotateTexturesCommand* rotateCounterClockwise(Model::MapDocument& document, const Model::FaceList& faces, float angle);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateTexturesCommand__) */
