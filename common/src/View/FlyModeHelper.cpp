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

#include "FlyModeHelper.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "View/ExecutableEvent.h"
#include "View/KeyboardShortcut.h"

#ifdef __APPLE__
// Don't include Quickdraw - we don't need it and it leads to symbol redefinition errrors
#define __QUICKDRAWAPI__
#include <ApplicationServices/ApplicationServices.h>
#endif

namespace TrenchBroom {
    namespace View {
        class FlyModeHelper::CameraEvent : public ExecutableEvent::Executable {
        private:
            wxWindow* m_window;
            Renderer::Camera& m_camera;
            Vec3f m_moveDelta;
            Vec2f m_rotateAngles;
        public:
            CameraEvent(wxWindow* window, Renderer::Camera& camera) :
            m_window(window),
            m_camera(camera) {}
            
            void setMoveDelta(const Vec3f& moveDelta) {
                m_moveDelta = moveDelta;
            }
            
            void setRotateAngles(const Vec2f& rotateAngles) {
                m_rotateAngles = rotateAngles;
            }
        private:
            void execute() {
                m_camera.moveBy(m_moveDelta);
                m_camera.rotate(m_rotateAngles.x(), m_rotateAngles.y());
                m_window->Refresh();
            }
        };
        
        FlyModeHelper::FlyModeHelper(wxWindow* window, Renderer::Camera& camera) :
        m_window(window),
        m_camera(camera),
        m_enabled(false) {
            m_window->Bind(wxEVT_KEY_DOWN, &FlyModeHelper::OnKeyDown, this);
            m_window->Bind(wxEVT_KEY_UP, &FlyModeHelper::OnKeyUp, this);

            m_forward = m_backward = m_left = m_right = false;
            m_lastPollTime = ::wxGetLocalTimeMillis();
            Run();
        }
        
        FlyModeHelper::~FlyModeHelper() {
            /* Since the window is already deleted when this destructor is called, we omit the cleanup.
            m_window->Unbind(wxEVT_KEY_DOWN, &FlyModeHelper::OnKeyDown, this);
            m_window->Unbind(wxEVT_KEY_UP, &FlyModeHelper::OnKeyUp, this);

            if (enabled())
                disable();
             */
        }
        
        void FlyModeHelper::enable() {
            assert(!enabled());
            lockMouse();
            m_enabled = true;
        }
        
        void FlyModeHelper::disable() {
            assert(enabled());
            unlockMouse();
            m_enabled = false;
        }
        
        bool FlyModeHelper::enabled() const {
            return m_enabled;
        }

        void FlyModeHelper::lockMouse() {
            m_window->SetCursor(wxCursor(wxCURSOR_BLANK));
            
#ifdef __APPLE__
            CGAssociateMouseAndMouseCursorPosition(false);
            int32_t dx, dy;
            CGGetLastMouseDelta(&dx, &dy);
#else
            m_originalMousePos = m_window->ScreenToClient(::wxGetMousePosition());
            resetMouse();
#endif
        }
        
        void FlyModeHelper::unlockMouse() {
#ifdef __APPLE__
            CGAssociateMouseAndMouseCursorPosition(true);
#else
            m_window->WarpPointer(m_originalMousePos.x, m_originalMousePos.y);
#endif
            m_window->SetCursor(wxNullCursor);
        }
        
        wxThread::ExitCode FlyModeHelper::Entry() {
            while (!TestDestroy()) {
                const Vec3f delta = moveDelta(pollTime());
                const Vec2f angles = m_enabled ? lookDelta(pollMouseDelta()) : Vec2f::Null;
                
                if (!delta.null() || !angles.null()) {
                    CameraEvent* event = new CameraEvent(m_window, m_camera);
                    event->setMoveDelta(delta);
                    event->setRotateAngles(angles);
                    
                    if (!TestDestroy() && wxTheApp != NULL)
                        wxTheApp->QueueEvent(new ExecutableEvent(event));
                }
                
                Sleep(20);
            }
            return static_cast<ExitCode>(0);
        }

        Vec3f FlyModeHelper::moveDelta(const float time) const {
            const float dist = moveSpeed() * time;

            Vec3f delta;
            if (m_forward)
                delta += m_camera.direction() * dist;
            if (m_backward)
                delta -= m_camera.direction() * dist;
            if (m_left)
                delta -= m_camera.right() * dist;
            if (m_right)
                delta += m_camera.right() * dist;
            return delta;
        }

        Vec2f FlyModeHelper::lookDelta(const wxPoint mouseDelta) const {
            const Vec2f speed = lookSpeed();
            const float hAngle = static_cast<float>(mouseDelta.x) * speed.x();
            const float vAngle = static_cast<float>(mouseDelta.y) * speed.y();
            return Vec2f(hAngle, vAngle);
        }

        Vec2f FlyModeHelper::lookSpeed() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            Vec2f speed(prefs.get(Preferences::CameraFlySpeed), prefs.get(Preferences::CameraFlySpeed));
            speed /= -50.0f;
            if (prefs.get(Preferences::CameraFlyInvertV))
                speed[1] *= -1.0f;
            return speed;
        }
        
        float FlyModeHelper::moveSpeed() const {
            return 256.0f / 1000.0f;
        }

        float FlyModeHelper::pollTime() {
            const wxLongLong currentTime = ::wxGetLocalTimeMillis();
            const float time = static_cast<float>((currentTime - m_lastPollTime).ToLong());
            m_lastPollTime = currentTime;
            
            return time;
        }

        wxPoint FlyModeHelper::pollMouseDelta() {
#ifndef __APPLE__
            const wxPoint currentMousePos = m_window->ScreenToClient(::wxGetMousePosition());
            const wxPoint delta = currentMousePos - windowCenter();
            resetMouse();
            return delta;
#else
            int32_t dx, dy;
            CGGetLastMouseDelta(&dx, &dy);
            return wxPoint(dx, dy);
#endif
        }
        
        void FlyModeHelper::resetMouse() {
#ifndef __APPLE__
            const wxPoint center = windowCenter();
            m_window->WarpPointer(center.x, center.y);
#endif
        }

        wxPoint FlyModeHelper::windowCenter() const {
            const wxSize size = m_window->GetSize();
            return wxPoint(size.x / 2, size.y / 2);
        }

        void FlyModeHelper::OnKeyDown(wxKeyEvent& event) {
            onKey(event, true);
        }
        
        void FlyModeHelper::OnKeyUp(wxKeyEvent& event) {
            onKey(event, false);
        }

        void FlyModeHelper::onKey(wxKeyEvent& event, const bool down) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const KeyboardShortcut& forward = prefs.get(Preferences::CameraFlyForward);
            const KeyboardShortcut& backward = prefs.get(Preferences::CameraFlyBackward);
            const KeyboardShortcut& left = prefs.get(Preferences::CameraFlyLeft);
            const KeyboardShortcut& right = prefs.get(Preferences::CameraFlyRight);
            
            if (forward.matches(event))
                m_forward = down;
            else if (backward.matches(event))
                m_backward = down;
            else if (left.matches(event))
                m_left = down;
            else if (right.matches(event))
                m_right = down;
            else
                event.Skip();
        }
    }
}
