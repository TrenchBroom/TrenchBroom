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

#ifndef __TrenchBroom__ModelHitFilters__
#define __TrenchBroom__ModelHitFilters__

#include "Hit.h"
#include "HitFilter.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
        
        class SelectionHitFilter : public HitFilter {
        private:
            bool doMatches(const Hit& hit) const;
        };
        
        class ContextHitFilter : public HitFilter {
        private:
            const EditorContext& m_context;
        public:
            ContextHitFilter(const EditorContext& context);
        private:
            bool doMatches(const Hit& hit) const;
        };
        
        const Hit& firstHit(const Hits& hits, Hit::HitType type, const EditorContext& context, bool ignoreOccluders);
        const Hit& firstHit(const Hits& hits, Hit::HitType type, const EditorContext& context, bool ignoreOccluders, bool selectedOnly);
    }
}

#endif /* defined(__TrenchBroom__ModelHitFilters__) */
