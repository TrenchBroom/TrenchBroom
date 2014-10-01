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

#include "ModelHitFilters.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"

namespace TrenchBroom {
    namespace Model {
        bool SelectionHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == Model::Entity::EntityHit)
                return hitToEntity(hit)->selected();
            if (hit.type() == Model::Brush::BrushHit)
                return hitToBrush(hit)->selected() || hitToFace(hit)->selected();
            return false;
        }
        
        ContextHitFilter::ContextHitFilter(const EditorContext& context) :
        m_context(context) {}
        
        bool ContextHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == Entity::EntityHit)
                return m_context.pickable(hitToEntity(hit));
            if (hit.type() == Brush::BrushHit)
                return m_context.pickable(hitToBrush(hit));
            return false;
        }
        
        const Hit& firstHit(const Hits& hits, const Hit::HitType type, const EditorContext& context, const bool ignoreOccluders) {
            return firstHit(hits, type, context, ignoreOccluders, false);
        }
        
        const Hit& firstHit(const Hits& hits, Hit::HitType type, const EditorContext& context, const bool ignoreOccluders, const bool selectedOnly) {
            HitFilterChain hitFilter = chainHitFilter(TypedHitFilter(type), ContextHitFilter(context));
            if (selectedOnly)
                hitFilter = chainHitFilter(SelectionHitFilter(), hitFilter);
            return hits.findFirst(hitFilter, ignoreOccluders);
        }
    }
}
