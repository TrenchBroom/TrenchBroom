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

#ifndef __TrenchBroom__CameraAnimation__
#define __TrenchBroom__CameraAnimation__

#include "View/Animation.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace View {
        class EditorView;
    }
    
    namespace View {
        class CameraAnimation : public Animation {
        private:
            View::EditorView& m_view;
            const Vec3f m_startPosition;
            const Vec3f m_startDirection;
            const Vec3f m_startUp;
            const Vec3f m_targetPosition;
            const Vec3f m_targetDirection;
            const Vec3f m_targetUp;
        protected:
            void doUpdate(double progress);
        public:
            CameraAnimation(View::EditorView& view, const Vec3f& targetPosition, const Vec3f& targetDirection, const Vec3f& targetUp, wxLongLong duration);

            Type type() const;
        };
    }
}

#endif /* defined(__TrenchBroom__CameraAnimation__) */
