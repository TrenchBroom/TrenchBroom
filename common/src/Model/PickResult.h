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

#ifndef TrenchBroom_PickResult
#define TrenchBroom_PickResult

#include "MathUtils.h"
#include "Model/CompareHits.h"
#include "Model/Hit.h"
#include "Model/HitQuery.h"

namespace TrenchBroom {
    namespace Model {
        class CompareHits;
        class HitFilter;
        class EditorContext;

        class PickResult {
        public:
            typedef std::tr1::shared_ptr<CompareHits> ComparePtr;
        private:
            const EditorContext* m_editorContext;
            Hit::List m_hits;
            ComparePtr m_compare;
            class CompareWrapper;
        public:
            PickResult(const EditorContext& editorContext, CompareHits* compare) :
            m_editorContext(&editorContext),
            m_compare(compare) {}

            PickResult();

            static PickResult byDistance(const EditorContext& editorContext);
            static PickResult bySize(const EditorContext& editorContext, Math::Axis::Type axis);

            bool empty() const;
            size_t size() const;

            void addHit(const Hit& hit);

            const Hit::List& all() const;
            HitQuery query() const;
        };
    }
}

#endif /* defined(TrenchBroom_PickResult) */
