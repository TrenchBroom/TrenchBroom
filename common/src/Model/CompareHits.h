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

#ifndef TrenchBroom_CompareHits
#define TrenchBroom_CompareHits

#include "TrenchBroom.h"

#include <vecmath/util.h>

namespace TrenchBroom {
    namespace Model {
        class Hit;
        
        class CompareHits {
        public:
            virtual ~CompareHits();
            int compare(const Hit& lhs, const Hit& rhs) const;
        private:
            virtual int doCompare(const Hit& lhs, const Hit& rhs) const = 0;
        };

        class CombineCompareHits : public CompareHits {
        private:
            CompareHits* m_first;
            CompareHits* m_second;
        public:
            CombineCompareHits(CompareHits* first, CompareHits* second);
            ~CombineCompareHits() override;
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const override;
        };
        
        class CompareHitsByType : public CompareHits {
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const override;
        };
        
        class CompareHitsByDistance : public CompareHits {
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const override;
        };
        
        class CompareHitsBySize : public CompareHits {
        private:
            const vm::axis::type m_axis;
            CompareHitsByDistance m_compareByDistance;
        public:
            CompareHitsBySize(vm::axis::type axis);
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const override;
            FloatType getSize(const Hit& hit) const;
        };
    }
}

#endif /* defined(TrenchBroom_CompareHits) */
