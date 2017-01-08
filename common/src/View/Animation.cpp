/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include <wx/app.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        Animation::Type Animation::freeType() {
            static Type type = 0;
            return type++;
        }
        
        Animation::Animation(const Type type, const Curve curve, const wxLongLong duration) :
        m_type(type),
        m_curve(NULL),
        m_duration(duration),
        m_elapsed(0),
        m_progress(0.0) {
            assert(m_duration > 0);
            switch (curve) {
                case Curve_EaseInEaseOut:
                    m_curve = new EaseInEaseOutAnimationCurve(m_duration);
                    break;
                case Curve_Flat:
                    m_curve = new FlatAnimationCurve();
                    break;
            }
        }
        
        Animation::~Animation() {
            delete m_curve;
            m_curve = NULL;
        }
        
        Animation::Type Animation::type() const {
            return m_type;
        }
        
        bool Animation::step(const wxLongLong delta) {
            m_elapsed = std::min(m_elapsed + delta, m_duration);
            m_progress = m_elapsed.ToDouble() / m_duration.ToDouble();
            return m_elapsed >= m_duration;
        }
        
        void Animation::update() {
            doUpdate(m_progress);
        }
        
        ExecutableAnimation::ExecutableAnimation(const Animation::Array& animations) :
        m_animations(animations) {}
        
        void ExecutableAnimation::execute() {
            for (Animation::Ptr animation : m_animations)
                animation->update();
        }
        
        AnimationManager::AnimationManager() :
        m_lastTime(wxGetLocalTimeMillis()) {
            Run();
        }
        
        void AnimationManager::runAnimation(Animation* animation, const bool replace) {
            ensure(animation != NULL, "animation is null");
            
            Animation::Array& array = m_animations[animation->type()];
            if (replace)
                array.clear();
            array.push_back(Animation::Ptr(animation));
        }
        
        wxThread::ExitCode AnimationManager::Entry() {
            while (!TestDestroy()) {
                const wxLongLong elapsed = wxGetLocalTimeMillis() - m_lastTime;
                
                Animation::Array updateAnimations;
                if (!m_animations.empty()) {
                    auto mapIt = std::begin(m_animations);
                    while (mapIt != std::end(m_animations)) {
                        Animation::Array& array = mapIt->second;
                        auto arrayIt = std::begin(array);
                        while (arrayIt != std::end(array)) {
                            Animation::Ptr animation = *arrayIt;
                            if (animation->step(elapsed))
                                arrayIt = array.erase(arrayIt);
                            updateAnimations.push_back(animation);
                            if (arrayIt != std::end(array))
                                ++arrayIt;
                        }
                        
                        if (array.empty())
                            m_animations.erase(mapIt++);
                        else
                            ++mapIt;
                    }
                }
                m_lastTime += elapsed;
                
                if (!TestDestroy() && wxTheApp != NULL && !updateAnimations.empty()) {
                    ExecutableEvent::Executable::Ptr executable(new ExecutableAnimation(updateAnimations));
                    ExecutableEvent* event = new ExecutableEvent(executable);
                    wxTheApp->QueueEvent(event);
                }
                
                Sleep(20);
            }
            
            return static_cast<ExitCode>(0);
        }
    }
}
