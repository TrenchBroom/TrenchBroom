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

#ifndef TrenchBroom_HitFilter
#define TrenchBroom_HitFilter

#include "Hit.h"
#include "SharedPointer.h"

namespace TrenchBroom {
    namespace Model {
        class HitFilter {
        private:
            class Always;
            class Never;
        public:
            static HitFilter* always();
            static HitFilter* never();
            
            virtual ~HitFilter();
            bool matches(const Hit& hit) const;
        private:
            virtual bool doMatches(const Hit& hit) const = 0;
        };
        
        class HitFilterChain : public HitFilter {
        private:
            const HitFilter* m_filter;
            const HitFilter* m_next;
        public:
            HitFilterChain(const HitFilter* filter, const HitFilter* next);
            ~HitFilterChain();
        private:
            bool doMatches(const Hit& hit) const;
        };
        
        class TypedHitFilter : public HitFilter {
        private:
            Hit::HitType m_typeMask;
        public:
            TypedHitFilter(Hit::HitType typeMask);
        private:
            bool doMatches(const Hit& hit) const;
        };

        class SelectionHitFilter : public HitFilter {
        private:
            bool doMatches(const Hit& hit) const;
        };
        
        class MinDistanceHitFilter : public HitFilter {
        private:
            FloatType m_minDistance;
        public:
            MinDistanceHitFilter(FloatType minDistance);
        private:
            bool doMatches(const Hit& hit) const;
        };
        
        class EditorContext;

        class ContextHitFilter : public HitFilter {
        private:
            const EditorContext& m_context;
        public:
            ContextHitFilter(const EditorContext& context);
        private:
            bool doMatches(const Hit& hit) const;
        };
    }
}

#endif /* defined(TrenchBroom_HitFilter) */
