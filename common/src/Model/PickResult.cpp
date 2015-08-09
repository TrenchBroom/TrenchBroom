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

#include "PickResult.h"

#include "Model/CompareHits.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult::CompareWrapper {
        private:
            const CompareHits* m_compare;
        public:
            CompareWrapper(const CompareHits* compare) : m_compare(compare) {}
            bool operator()(const Hit& lhs, const Hit& rhs) const { return m_compare->compare(lhs, rhs) < 0; }
        };
        
        PickResult::PickResult() :
        m_editorContext(NULL),
        m_compare(new CompareHitsByDistance()) {}

        PickResult PickResult::byDistance(const EditorContext& editorContext) {
            CompareHits* compare = new CombineCompareHits(new CompareHitsByDistance(),
                                                          new CompareHitsByType());
            return PickResult(editorContext, compare);
        }

        PickResult PickResult::bySize(const EditorContext& editorContext, const Math::Axis::Type axis) {
            return PickResult(editorContext, new CompareHitsBySize(axis));
        }

        bool PickResult::empty() const {
            return m_hits.empty();
        }
        
        size_t PickResult::size() const {
            return m_hits.size();
        }
        
        void PickResult::addHit(const Hit& hit) {
            assert(m_compare != NULL);
            Hit::List::iterator pos = std::upper_bound(m_hits.begin(), m_hits.end(), hit, CompareWrapper(m_compare.get()));
            m_hits.insert(pos, hit);
        }
        
        const Hit::List& PickResult::all() const {
            return m_hits;
        }
        
        HitQuery PickResult::query() const {
            if (m_editorContext != NULL)
                return HitQuery(m_hits, *m_editorContext);
            return HitQuery(m_hits);
        }
    }
}
