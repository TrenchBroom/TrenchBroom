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

#ifndef TrenchBroom_AnimationCurve
#define TrenchBroom_AnimationCurve

#include <wx/longlong.h>

namespace TrenchBroom {
    namespace View {
        class AnimationCurve {
        public:
            virtual ~AnimationCurve();
            double apply(double progress) const;
        private:
            virtual double doApply(double progress) const = 0;
        };
        
        class FlatAnimationCurve : public AnimationCurve {
        private:
            double doApply(double progress) const;
        };
        
        class EaseInEaseOutAnimationCurve : public AnimationCurve {
        private:
            double m_threshold;
        public:
            EaseInEaseOutAnimationCurve(wxLongLong duration);
            double doApply(double progress) const;
        };
    }
}


#endif /* defined(TrenchBroom_AnimationCurve) */
