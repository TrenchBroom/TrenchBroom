/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "DefaultHitFilter.h"

#include "Model/Filter.h"
#include "Model/Entity.h"
#include "Model/Brush.h"
#include "Model/BrushFAce.h"
#include "Model/HitAdapter.h"

namespace TrenchBroom {
    namespace Model {
        DefaultHitFilter::DefaultHitFilter(Filter& filter) :
        m_filter(filter) {}
        
        bool DefaultHitFilter::matches(const Hit& hit) const {
            if (hit.type() == Entity::EntityHit)
                return m_filter.pickable(hitAsEntity(hit));
            if (hit.type() == Brush::BrushHit)
                return m_filter.pickable(hitAsBrush(hit));
            return false;
        }
        
    }
}
