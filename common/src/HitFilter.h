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

#ifndef __TrenchBroom__HitFilter__
#define __TrenchBroom__HitFilter__

#include "Hit.h"
#include "SharedPointer.h"

namespace TrenchBroom {
    class HitFilter {
    public:
        virtual ~HitFilter();
        virtual bool matches(const Hit& hit) const = 0;
    };
    
    class HitFilterChain : public HitFilter {
    private:
        typedef TrenchBroom::shared_ptr<HitFilter> FilterPtr;
        FilterPtr m_filter;
        FilterPtr m_next;
    public:
        template <class F, class N>
        HitFilterChain(const F& filter, const N& next) :
        m_filter(new F(filter)),
        m_next(new N(next)) {}
        
        bool matches(const Hit& hit) const;
    };
    
    class TypedHitFilter : public HitFilter {
    private:
        Hit::HitType m_typeMask;
    public:
        TypedHitFilter(Hit::HitType typeMask);
        bool matches(const Hit& hit) const;
    };
    
    template <class F1, class F2>
    HitFilterChain chainHitFilter(const F1& f1, const F2& f2) {
        return HitFilterChain(f1, f2);
    }
    
    template <class F1, class F2, class F3>
    HitFilterChain chainHitFilter(const F1& f1, const F2& f2, const F3& f3) {
        return chainHitFilter(f1, chainHitFilter(f2, f3));
    }
}

#endif /* defined(__TrenchBroom__DefaultHitFilter__) */
