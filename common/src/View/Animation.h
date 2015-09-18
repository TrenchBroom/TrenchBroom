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

#ifndef TrenchBroom_Animation
#define TrenchBroom_Animation

#include "SharedPointer.h"
#include "View/ExecutableEvent.h"

#include <map>
#include <vector>

#include <wx/longlong.h>
#include <wx/thread.h>

namespace TrenchBroom {
    namespace View {
        class AnimationCurve;
        
        class Animation {
        public:
            typedef int Type;
            static const Type NoType = -1;
            
            typedef std::tr1::shared_ptr<Animation> Ptr;
            typedef std::vector<Ptr> List;
            
            typedef enum {
                Curve_Flat,
                Curve_EaseInEaseOut
            } Curve;
            
        private:
            const Type m_type;
            const AnimationCurve* m_curve;
            
            const wxLongLong m_duration;
            wxLongLong m_elapsed;
            double m_progress;
        public:
            static Type freeType();

            Animation(Type type, Curve curve, wxLongLong duration);
            virtual ~Animation();
            
            Type type() const;
            bool step(wxLongLong delta);
            void update();
        private:
            virtual void doUpdate(double progress) = 0;
        };
        
        class ExecutableAnimation : public ExecutableEvent::Executable {
        private:
            Animation::List m_animations;
        public:
            ExecutableAnimation(const Animation::List& animations);
        private:
            void execute();
        };
        
        class AnimationManager : public wxThread {
        private:
            typedef std::map<Animation::Type, Animation::List> AnimationMap;
            
            AnimationMap m_animations;
            wxLongLong m_lastTime;
        public:
            AnimationManager();
            void runAnimation(Animation* animation, bool replace);
        private:
            ExitCode Entry();
        };
    }
}

#endif /* defined(TrenchBroom_Animation) */
