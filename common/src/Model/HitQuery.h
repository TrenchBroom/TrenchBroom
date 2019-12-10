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

#ifndef TrenchBroom_HitQuery
#define TrenchBroom_HitQuery

#include "TrenchBroom.h"
#include "Model/Model_Forward.h"
#include "Model/HitType.h"

#include <list>

namespace TrenchBroom {
    namespace Model {
        class HitQuery {
        private:
            const std::list<Hit>* m_hits;
            const EditorContext* m_editorContext;
            HitFilter* m_include;
            HitFilter* m_exclude;
        public:
            HitQuery(const std::list<Hit>& hits, const EditorContext& editorContext);
            HitQuery(const std::list<Hit>& hits);
            HitQuery(const HitQuery& other);
            ~HitQuery();

            HitQuery& operator=(HitQuery other);
            friend void swap(HitQuery& lhs, HitQuery& rhs);

            HitQuery& pickable();
            HitQuery& type(HitType::Type type);
            HitQuery& occluded(HitType::Type type = HitType::AnyType);
            HitQuery& selected();
            HitQuery& transitivelySelected();
            HitQuery& minDistance(FloatType minDistance);

            bool empty() const;
            const Hit& first() const;
            std::list<Hit> all() const;
        private:
            bool visible(const Hit& hit) const;
        };
    }
}

#endif /* defined(TrenchBroom_HitQuery) */
