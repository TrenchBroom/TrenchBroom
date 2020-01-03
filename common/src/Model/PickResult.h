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

#ifndef TrenchBroom_PickResult
#define TrenchBroom_PickResult

#include "Macros.h"
#include "Model/Hit.h"

#include <vecmath/util.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class CompareHits;
        class EditorContext;
        class HitQuery;

        class PickResult {
        private:
            const EditorContext* m_editorContext;
            std::vector<Hit> m_hits;
            std::shared_ptr<CompareHits> m_compare;
            class CompareWrapper;
        public:
            PickResult(const EditorContext& editorContext, std::shared_ptr<CompareHits> compare);
            PickResult();

            defineCopyAndMove(PickResult)

            ~PickResult();

            static PickResult byDistance(const EditorContext& editorContext);
            static PickResult bySize(const EditorContext& editorContext, vm::axis::type axis);

            bool empty() const;
            size_t size() const;

            void addHit(const Hit& hit);

            const std::vector<Hit>& all() const;
            HitQuery query() const;

            void clear();
        };
    }
}

#endif /* defined(TrenchBroom_PickResult) */
