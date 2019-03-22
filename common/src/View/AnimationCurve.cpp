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

#include "AnimationCurve.h"

namespace TrenchBroom {
    namespace View {
        AnimationCurve::~AnimationCurve() {}

        double AnimationCurve::apply(const double progress) const {
            return doApply(progress);
        }

        double FlatAnimationCurve::doApply(const double progress) const {
            return progress;
        }

        EaseInEaseOutAnimationCurve::EaseInEaseOutAnimationCurve(const wxLongLong& duration) {
            if (duration < 100 + 100)
                m_threshold = 0.5;
            else
                m_threshold = 100.0 / duration.ToDouble();
        }

        double EaseInEaseOutAnimationCurve::doApply(const double progress) const {
            if (progress < m_threshold)
                return progress * progress / m_threshold;
            if (progress > 1.0 - m_threshold) {
                double temp = 1.0 - progress;
                temp = temp * temp / m_threshold;
                return 1.0 - m_threshold + temp;
            }
            return progress;
        }
    }
}
