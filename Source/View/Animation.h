/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Animation__
#define __TrenchBroom__Animation__

#include "Utility/ExecutableEvent.h"
#include "Utility/SharedPointer.h"

#include <map>
#include <vector>

#include <wx/event.h>
#include <wx/longlong.h>
#include <wx/thread.h>

namespace TrenchBroom {
    namespace View {
        class AnimationCurve {
        public:
            virtual ~AnimationCurve() {}
            virtual double apply(double progress) const = 0;
        };

        class FlatAnimationCurve : public AnimationCurve {
        public:
            inline double apply(double progress) const {
                return progress;
            }
        };

        class EaseInEaseOutAnimationCurve : public AnimationCurve {
        private:
            double m_threshold;
        public:
            EaseInEaseOutAnimationCurve(wxLongLong duration) {
                if (duration < 100 + 100)
                    m_threshold = 0.5;
                else
                    m_threshold = 100.0 / duration.ToDouble();
            };

            inline double apply(double progress) const {
                if (progress < m_threshold)
                    return progress * progress / m_threshold;
                if (progress > 1.0 - m_threshold) {
                    double temp = 1.0 - progress;
                    temp = temp * temp / m_threshold;
                    return 1.0 - m_threshold + temp;
                }
                return progress;
            }
        };

        class Animation {
        public:
            typedef int Type;
            static const Type NoType = -1;

            typedef std::tr1::shared_ptr<Animation> Ptr;
            typedef std::vector<Ptr> List;

            typedef enum {
                FlatCurve,
                EaseInEaseOutCurve
            } Curve;

            static inline Type uniqueType() {
                static Type type = 0;
                return type++;
            }
        private:
            const AnimationCurve* m_curve;
            const wxLongLong m_duration;
            wxLongLong m_elapsed;
            double m_progress;
            wxCriticalSection m_lock;
        protected:
            virtual void doUpdate(double progress) = 0;
        public:
            Animation(Curve curve, wxLongLong duration);
            virtual ~Animation();

            virtual Type type() const = 0;

            bool step(wxLongLong delta);
            void update();
        };

        class AnimationExecutable : public ExecutableEvent::Executable {
        private:
            Animation::List m_animations;
        protected:
            void execute();
        public:
            AnimationExecutable(const Animation::List& animations);
        };
        
        class AnimationManager : public wxThread {
        private:
            typedef std::map<Animation::Type, Animation::List> AnimationMap;

            AnimationMap m_animations;
            wxCriticalSection m_animationsLock;
            wxLongLong m_lastTime;

            ExitCode Entry();
        public:
            AnimationManager();

            inline void runAnimation(Animation* animation, bool replace) {
                assert(animation != NULL);

                wxCriticalSectionLocker lockAnimations(m_animationsLock);
                Animation::List& list = m_animations[animation->type()];
                if (replace)
                    list.clear();
                list.push_back(Animation::Ptr(animation));
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Animation__) */
