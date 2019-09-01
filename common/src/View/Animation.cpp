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

#include "Animation.h"
#include "Macros.h"
#include "View/AnimationCurve.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace View {
        // Animation

        Animation::Type Animation::freeType() {
            static Type type = 0;
            return type++;
        }

        Animation::Animation(const Type type, const Curve curve, const double duration) :
        m_type(type),
        m_curve(createAnimationCurve(curve, duration)),
        m_duration(duration),
        m_elapsed(0),
        m_progress(0.0) {
            assert(m_duration > 0);
        }

        Animation::~Animation() = default;

        Animation::Type Animation::type() const {
            return m_type;
        }

        bool Animation::step(const double delta) {
            m_elapsed = std::min(m_elapsed + delta, m_duration);
            m_progress = m_elapsed / m_duration;
            return m_elapsed >= m_duration;
        }

        void Animation::update() {
            doUpdate(m_progress);
        }

        std::unique_ptr<AnimationCurve> Animation::createAnimationCurve(const Curve curve, const double duration) {
            switch (curve) {
                case Curve_EaseInEaseOut:
                    return std::make_unique<EaseInEaseOutAnimationCurve>(duration);
                case Curve_Flat:
                    return std::make_unique<FlatAnimationCurve>();
            }
            return { nullptr };
        }

        // AnimationManager

        const int AnimationManager::AnimationUpdateRateHz = 60;

        AnimationManager::AnimationManager(QObject* parent) :
        QObject(parent),
        m_timer(new QTimer(this)) {
            connect(m_timer, &QTimer::timeout, this, &AnimationManager::onTimerTick);
        }

        void AnimationManager::runAnimation(Animation* animation, const bool replace) {
            ensure(animation != nullptr, "animation is null");

            Animation::List& list = m_animations[animation->type()];
            if (replace) {
                list.clear();
            }
            list.push_back(Animation::Ptr(animation));

            // start the ticks if needed
            if (!m_timer->isActive()) {
                assert(!m_elapsedTimer.isValid());
                m_elapsedTimer.start();

                m_timer->start(1000 / AnimationUpdateRateHz);
            }
        }

        void AnimationManager::onTimerTick() {
            assert(m_elapsedTimer.isValid());
            const double msElapsed = static_cast<double>(m_elapsedTimer.restart());

            // advance the animation times
            Animation::List updateAnimations;
            if (!m_animations.empty()) {
                auto mapIt = std::begin(m_animations);
                while (mapIt != std::end(m_animations)) {
                    Animation::List& list = mapIt->second;
                    auto listIt = std::begin(list);
                    while (listIt != std::end(list)) {
                        Animation::Ptr animation = *listIt;
                        if (animation->step(msElapsed)) {
                            listIt = list.erase(listIt);
                        }
                        animation->update();
                        updateAnimations.push_back(animation);
                        if (listIt != std::end(list)) {
                            ++listIt;
                        }
                    }

                    if (list.empty()) {
                        m_animations.erase(mapIt++);
                    } else {
                        ++mapIt;
                    }
                }
            }

            // stop the animations if all are finished
            if (m_animations.empty()) {
                m_elapsedTimer.invalidate();
                m_timer->stop();
            }
        }
    }
}
