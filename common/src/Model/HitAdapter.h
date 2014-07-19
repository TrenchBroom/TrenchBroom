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

#ifndef TrenchBroom_HitAdapter_h
#define TrenchBroom_HitAdapter_h

#include "HitFilter.h"
#include "Hit.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class BrushFace;
        class Entity;
        class ModelFilter;
        class Object;
        
        Object* hitAsObject(const Hit& hit);
        Entity* hitAsEntity(const Hit& hit);
        Brush* hitAsBrush(const Hit& hit);
        BrushFace* hitAsFace(const Hit& hit);
        
        
        class SelectionHitFilter : public HitFilter {
        public:
            bool matches(const Hit& hit) const;
        };
        
        class ModelFilter;
        class DefaultHitFilter : public HitFilter {
        private:
            const ModelFilter& m_filter;
        public:
            DefaultHitFilter(const ModelFilter& filter);
            bool matches(const Hit& hit) const;
        };

        const Hit& findFirstHit(const Hits& hits, Hit::HitType type, const ModelFilter& filter, bool ignoreOccluders);
        const Hit& findFirstHit(const Hits& hits, Hit::HitType type, const ModelFilter& filter, bool ignoreOccluders, bool selectedOnly);
    }
}

#endif
