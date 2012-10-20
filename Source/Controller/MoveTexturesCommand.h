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

#ifndef __TrenchBroom__MoveTexturesCommand__
#define __TrenchBroom__MoveTexturesCommand__

#include "Controller/Command.h"

#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class MoveTexturesCommand : public DocumentCommand {
        protected:
            float m_distance;
            Vec3f m_direction;
            
            bool performDo();
            bool performUndo();
            
            MoveTexturesCommand(Model::MapDocument& document, const wxString& name, float distance, const Vec3f& direction);
        public:
            static MoveTexturesCommand* moveTextures(Model::MapDocument& document, const wxString& name, float distance, const Vec3f& direction);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveTexturesCommand__) */
