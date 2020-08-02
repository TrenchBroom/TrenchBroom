/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Ensure.h"
#include "Model/CompareHits.h"
#include "Model/Hit.h"
#include "Model/HitQuery.h"

#include <vecmath/scalar.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class PickResult::CompareWrapper {
        private:
            const CompareHits* m_compare;
        public:
            CompareWrapper(const CompareHits* compare) : m_compare(compare) {}
            bool operator()(const Hit& lhs, const Hit& rhs) const { return m_compare->compare(lhs, rhs) < 0; }
        };

        PickResult::PickResult(const EditorContext& editorContext, std::shared_ptr<CompareHits> compare) :
        m_editorContext(&editorContext),
        m_compare(std::move(compare)) {}

        PickResult::PickResult() :
        m_editorContext(nullptr),
        m_compare(std::make_shared<CompareHitsByDistance>()) {}

        PickResult::~PickResult() = default;

        PickResult PickResult::byDistance(const EditorContext& editorContext) {
            return PickResult(editorContext, std::make_shared<CombineCompareHits>(
                std::make_unique<CompareHitsByDistance>(),
                std::make_unique<CompareHitsByType>()));
        }

        PickResult PickResult::bySize(const EditorContext& editorContext, const vm::axis::type axis) {
            return PickResult(editorContext, std::make_shared<CompareHitsBySize>(axis));
        }

        bool PickResult::empty() const {
            return m_hits.empty();
        }

        size_t PickResult::size() const {
            return m_hits.size();
        }

        void PickResult::addHit(const Hit& hit) {
            assert(!vm::is_nan(hit.distance()));
            assert(!vm::is_nan(hit.hitPoint()));
            ensure(m_compare.get() != nullptr, "compare is null");
            auto pos = std::upper_bound(std::begin(m_hits), std::end(m_hits), hit, CompareWrapper(m_compare.get()));
            m_hits.insert(pos, hit);
        }

        const std::vector<Hit>& PickResult::all() const {
            return m_hits;
        }

        HitQuery PickResult::query() const {
            if (m_editorContext != nullptr)
                return HitQuery(m_hits, *m_editorContext);
            return HitQuery(m_hits);
        }

        void PickResult::clear() {
            m_hits.clear();
        }
    }
}
