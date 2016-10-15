/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Hit.h"
#include "HitFilter.h"

#include <algorithm>
#include <limits>

namespace TrenchBroom {
    namespace Model {
        const Hit::HitType Hit::NoType = 0;
        const Hit::HitType Hit::AnyType = 0xFFFFFFFF;
        
        Hit::HitType Hit::freeHitType() {
            static HitType currentType = 1;
            const HitType result = currentType;
            currentType = currentType << 1;
            return result;
        }
        
        const Hit Hit::NoHit = Hit(NoType, 0.0, Vec3::Null, false);
        
        bool Hit::isMatch() const {
            return m_type != NoType;
        }
        
        Hit::HitType Hit::type() const {
            return m_type;
        }
        
        bool Hit::hasType(const HitType typeMask) const {
            return (m_type & typeMask) != 0;
        }
        
        FloatType Hit::distance() const {
            return m_distance;
        }
        
        const Vec3& Hit::hitPoint() const {
            return m_hitPoint;
        }
        
        FloatType Hit::error() const {
            return m_error;
        }
    }
}
