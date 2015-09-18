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

#ifndef TrenchBroom_CompareHits
#define TrenchBroom_CompareHits

#include "TrenchBroom.h"
#include "VecMath.h"

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
            ~CombineCompareHits();
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const;
        };
        
        class CompareHitsByType : public CompareHits {
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const;
        };
        
        class CompareHitsByDistance : public CompareHits {
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const;
        };
        
        class CompareHitsBySize : public CompareHits {
        private:
            const Math::Axis::Type m_axis;
            CompareHitsByDistance m_compareByDistance;
        public:
            CompareHitsBySize(Math::Axis::Type axis);
        private:
            int doCompare(const Hit& lhs, const Hit& rhs) const;
            FloatType getSize(const Hit& hit) const;
        };
    }
}

#endif /* defined(TrenchBroom_CompareHits) */
