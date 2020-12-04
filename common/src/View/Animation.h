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

#pragma once

#include <map>
#include <memory>
#include <vector>

#include <QObject>
#include <QElapsedTimer>

class QTimer;

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
            double doApply(double progress) const override;
        };

        class EaseInEaseOutAnimationCurve : public AnimationCurve {
        private:
            double m_threshold;
        public:
            EaseInEaseOutAnimationCurve(double duration);
            double doApply(double progress) const override;
        };

        class Animation {
        public:
            using Type = int;
            static const Type NoType = -1;

            enum class Curve {
                Flat,
                EaseInEaseOut
            };
        private:
            const Type m_type;
            std::unique_ptr<AnimationCurve> m_curve;

            const double m_duration;
            double m_elapsed;
            double m_progress;
        public:
            static Type freeType();

            Animation(Type type, Curve curve, double duration);
            virtual ~Animation();

            Type type() const;
            /**
             * Advances the animation by the given number of milliseconds.
             * @return true if the animation is finished.
             */
            bool step(double deltaMilliseconds);
            void update();
        private:
            static std::unique_ptr<AnimationCurve> createAnimationCurve(Curve curve, double duration);
            virtual void doUpdate(double progress) = 0;
        };

        class AnimationManager : public QObject {
            Q_OBJECT
        private:
            static const int AnimationUpdateRateHz;
        private:
            /**
             * To measure how much time to run the animation for in onTimerTick()
             */
            QElapsedTimer m_elapsedTimer;
            QTimer* m_timer;

            std::map<Animation::Type, std::vector<std::unique_ptr<Animation>>> m_animations;
        public:
            explicit AnimationManager(QObject* parent);
            void runAnimation(std::unique_ptr<Animation> animation, bool replace);

        private:
            void onTimerTick();
        };
    }
}


