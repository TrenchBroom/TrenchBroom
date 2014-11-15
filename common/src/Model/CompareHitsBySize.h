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

#ifndef __TrenchBroom__CompareHitsBySize__
#define __TrenchBroom__CompareHitsBySize__

#include "Hit.h"

namespace TrenchBroom {
    namespace Model {
        class CompareHitsBySize : public CompareHits {
        private:
            const Math::Axis::Type m_axis;
            CompareHitsByDistance m_compareByDistance;
        public:
            CompareHitsBySize(Math::Axis::Type axis);
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const;
            FloatType getSize(const Hit& hit) const;
        };
        
        Hits hitsBySize(Math::Axis::Type axis);
    }
}

#endif /* defined(__TrenchBroom__CompareHitsBySize__) */
