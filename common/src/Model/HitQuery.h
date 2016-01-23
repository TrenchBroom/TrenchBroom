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

#ifndef TrenchBroom_HitQuery
#define TrenchBroom_HitQuery

#include "Model/Hit.h"
#include "Model/HitFilter.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
        
        class HitQuery {
        private:
            const Hit::List& m_hits;
            const EditorContext* m_editorContext;
            HitFilter* m_include;
            HitFilter* m_exclude;
        public:
            HitQuery(const Hit::List& hits, const EditorContext& editorContext);
            HitQuery(const Hit::List& hits);
            ~HitQuery();
            
            HitQuery& pickable();
            HitQuery& type(Hit::HitType type);
            HitQuery& occluded(Hit::HitType type = Hit::AnyType);
            HitQuery& selected();
            HitQuery& minDistance(FloatType minDistance);
            
            const Hit& first() const;
            Hit::List all() const;
        private:
            bool visible(const Hit& hit) const;
        };
    }
}

#endif /* defined(TrenchBroom_HitQuery) */
