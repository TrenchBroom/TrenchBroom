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

#include "HitAdapter.h"

#include "Hit.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/ModelFilter.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        Object* hitAsObject(const Hit& hit) {
            if (hit.type() == Entity::EntityHit)
                return hit.target<Object*>();
            if (hit.type() == Brush::BrushHit) {
                BrushFace* face = hit.target<BrushFace*>();
                return face->parent();
            }
            return NULL;
        }
        
        Entity* hitAsEntity(const Hit& hit) {
            if (hit.type() == Entity::EntityHit)
                return hit.target<Entity*>();
            return NULL;
        }
        
        Brush* hitAsBrush(const Hit& hit) {
            if (hit.type() == Brush::BrushHit)
                return hit.target<BrushFace*>()->parent();
            return NULL;
        }
        
        BrushFace* hitAsFace(const Hit& hit) {
            if (hit.type() == Brush::BrushHit)
                return hit.target<BrushFace*>();
            return NULL;
        }

        bool SelectionHitFilter::matches(const Hit& hit) const {
            if (hit.type() == Model::Entity::EntityHit)
                return hitAsEntity(hit)->selected();
            if (hit.type() == Model::Brush::BrushHit)
                return hitAsBrush(hit)->selected() || hitAsFace(hit)->selected();
            return false;
        }
        
        DefaultHitFilter::DefaultHitFilter(const ModelFilter& filter) :
        m_filter(filter) {}
        
        bool DefaultHitFilter::matches(const Hit& hit) const {
            if (hit.type() == Entity::EntityHit)
                return m_filter.pickable(hitAsEntity(hit));
            if (hit.type() == Brush::BrushHit)
                return m_filter.pickable(hitAsBrush(hit));
            return false;
        }

        const Hit& findFirstHit(const Hits& hits, Hit::HitType type, const ModelFilter& filter, bool ignoreOccluders) {
            return hits.findFirst(chainHitFilter(TypedHitFilter(type), DefaultHitFilter(filter)), ignoreOccluders);
        }
    }
}
