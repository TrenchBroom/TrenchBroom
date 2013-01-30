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
            const Vec3f m_up;
            const Vec3f m_right;
            const float m_distance;
            const Direction m_direction;
            
            bool performDo();
            bool performUndo();
            
            MoveTexturesCommand(Model::MapDocument& document, const wxString& name, const Vec3f& up, const Vec3f& right, Direction direction, float distance);
        public:
            static MoveTexturesCommand* moveTextures(Model::MapDocument& document, const wxString& name, const Vec3f& up, const Vec3f& right, Direction direction, float distance);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveTexturesCommand__) */
