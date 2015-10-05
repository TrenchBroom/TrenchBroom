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

#ifndef TrenchBroom_MovementRestriction
#define TrenchBroom_MovementRestriction

#include "TrenchBroom.h"
#include "VecMath.h"

class wxKeyEvent;

namespace TrenchBroom {
    namespace View {
        class MovementRestriction {
        private:
            typedef enum {
                Restriction_None = 0,
                Restriction_X    = 1,
                Restriction_Y    = 2,
                Restriction_Z    = 3
            } Restriction;
            
            Restriction m_restriction;
            bool m_verticalRestriction;
        public:
            MovementRestriction();
            
            void toggleRestriction();
            void toggleVerticalRestriction(bool verticalRestriction);
            void toggleRestriction(Math::Axis::Type axis) ;
            
            bool isRestricted(Math::Axis::Type axis) const;
            Vec3 apply(const Vec3& v) const;
            
        private:
            const Vec3& xAxis() const;
            const Vec3& yAxis() const;
            const Vec3& zAxis() const;
            bool canMoveAlong(Math::Axis::Type axis) const;
        };
    }
}

#endif /* defined(TrenchBroom_MovementRestriction) */
