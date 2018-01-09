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

#include "FlyModeHelper.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "SetAny.h"
#include "Renderer/Camera.h"
#include "View/ExecutableEvent.h"
#include "View/KeyboardShortcut.h"

#include <wx/time.h>
#include <wx/window.h>
#include <wx/app.h>

#include <array>

namespace TrenchBroom {
    namespace View {
        class FlyModeHelper::CameraEvent : public ExecutableEvent::Executable {
        private:
            FlyModeHelper& m_helper;
            Renderer::Camera& m_camera;
            Vec3f m_moveDelta;
            Vec2f m_rotateAngles;
        public:
            CameraEvent(FlyModeHelper& helper, Renderer::Camera& camera) :
            m_helper(helper),
            m_camera(camera) {}

            void setMoveDelta(const Vec3f& moveDelta) {
                m_moveDelta = moveDelta;
            }

            void setRotateAngles(const Vec2f& rotateAngles) {
                m_rotateAngles = rotateAngles;
            }
        private:
            void execute() override {
                m_camera.moveBy(m_moveDelta);
                m_camera.rotate(m_rotateAngles.x(), m_rotateAngles.y());
                m_helper.resetMouse();
            }
        };

        FlyModeHelper::FlyModeHelper(wxWindow* window, Renderer::Camera& camera) :
        m_window(window),
        m_camera(camera),
        m_enabled(false),
        m_ignoreMotionEvents(false) {
            resetKeys();
            m_lastPollTime = ::wxGetLocalTimeMillis();

            Run();
        }

        FlyModeHelper::~FlyModeHelper() {
            /* Since the window is already deleted when this destructor is called, we omit the cleanup.
            if (enabled())
                disable();
             */
        }

        void FlyModeHelper::enable() {
            wxCriticalSectionLocker lock(m_critical);
            assert(!enabled());
            lockMouse();
            m_enabled = true;
        }

        void FlyModeHelper::disable() {
            wxCriticalSectionLocker lock(m_critical);
            assert(enabled());
            unlockMouse();
            m_enabled = false;
        }

        bool FlyModeHelper::enabled() const {
            return m_enabled;
        }

        void FlyModeHelper::lockMouse() {
            m_window->SetCursor(wxCursor(wxCURSOR_BLANK));

            m_originalMousePos = m_window->ScreenToClient(::wxGetMousePosition());
            m_currentMouseDelta = wxPoint(0,0);
            m_lastMousePos = m_originalMousePos;
            resetMouse();
        }

        void FlyModeHelper::unlockMouse() {
            m_window->WarpPointer(m_originalMousePos.x, m_originalMousePos.y);
            m_window->SetCursor(wxNullCursor);
        }

        bool FlyModeHelper::keyDown(wxKeyEvent& event) {
            const KeyboardShortcut& forward = pref(Preferences::CameraFlyForward);
            const KeyboardShortcut& backward = pref(Preferences::CameraFlyBackward);
            const KeyboardShortcut& left = pref(Preferences::CameraFlyLeft);
            const KeyboardShortcut& right = pref(Preferences::CameraFlyRight);
            const KeyboardShortcut& up = pref(Preferences::CameraFlyUp);
            const KeyboardShortcut& down = pref(Preferences::CameraFlyDown);

            wxCriticalSectionLocker lock(m_critical);

            bool anyMatch = false;
            if (forward.matchesKeyDown(event)) {
                m_forward = true;
                anyMatch = true;
            }
            if (backward.matchesKeyDown(event)) {
                m_backward = true;
                anyMatch = true;
            }
            if (left.matchesKeyDown(event)) {
                m_left = true;
                anyMatch = true;
            }
            if (right.matchesKeyDown(event)) {
                m_right = true;
                anyMatch = true;
            }
            if (up.matchesKeyDown(event)) {
                m_up = true;
                anyMatch = true;
            }
            if (down.matchesKeyDown(event)) {
                m_down = true;
                anyMatch = true;
            }
            return anyMatch;
        }

        bool FlyModeHelper::keyUp(wxKeyEvent& event) {
            const KeyboardShortcut& forward = pref(Preferences::CameraFlyForward);
            const KeyboardShortcut& backward = pref(Preferences::CameraFlyBackward);
            const KeyboardShortcut& left = pref(Preferences::CameraFlyLeft);
            const KeyboardShortcut& right = pref(Preferences::CameraFlyRight);
            const KeyboardShortcut& up = pref(Preferences::CameraFlyUp);
            const KeyboardShortcut& down = pref(Preferences::CameraFlyDown);

            wxCriticalSectionLocker lock(m_critical);

            bool anyMatch = false;
            if (forward.matchesKeyUp(event)) {
                m_forward = false;
                anyMatch = true;
            }
            if (backward.matchesKeyUp(event)) {
                m_backward = false;
                anyMatch = true;
            }
            if (left.matchesKeyUp(event)) {
                m_left = false;
                anyMatch = true;
            }
            if (right.matchesKeyUp(event)) {
                m_right = false;
                anyMatch = true;
            }
            if (up.matchesKeyUp(event)) {
                m_up = false;
                anyMatch = true;
            }
            if (down.matchesKeyUp(event)) {
                m_down = false;
                anyMatch = true;
            }
            return anyMatch;
        }

        void FlyModeHelper::resetKeys() {
            wxCriticalSectionLocker lock(m_critical);
            m_forward = m_backward = m_left = m_right = m_up = m_down = false;
        }

        void FlyModeHelper::motion(wxMouseEvent& event) {
            wxCriticalSectionLocker lock(m_critical);
            if (m_enabled && !m_ignoreMotionEvents) {
                const wxPoint currentMousePos = m_window->ScreenToClient(::wxGetMousePosition());
                const wxPoint delta = currentMousePos - m_lastMousePos;
                m_currentMouseDelta += delta;
                m_lastMousePos = currentMousePos;
            }
        }

        void FlyModeHelper::resetMouse() {
            wxCriticalSectionLocker lock(m_critical);
            if (m_enabled) {
                const SetBool ignoreMotion(m_ignoreMotionEvents);
                m_lastMousePos = windowCenter();
                m_window->WarpPointer(m_lastMousePos.x, m_lastMousePos.y);
            }
        }

        wxPoint FlyModeHelper::windowCenter() const {
            const wxSize size = m_window->GetSize();
            return wxPoint(size.x / 2, size.y / 2);
        }

        wxThread::ExitCode FlyModeHelper::Entry() {
            while (!TestDestroy()) {
                const Vec3f delta = moveDelta();
                const Vec2f angles = lookDelta();

                if (!delta.null() || !angles.null()) {
                    if (!TestDestroy() && wxTheApp != nullptr) {
                        CameraEvent* event = new CameraEvent(*this, m_camera);
                        event->setMoveDelta(delta);
                        event->setRotateAngles(angles);

                        ExecutableEvent* executable = new ExecutableEvent(event);
                        wxTheApp->QueueEvent(executable);
                    }
                }

                Sleep(20);
            }
            return static_cast<ExitCode>(nullptr);
        }

        Vec3f FlyModeHelper::moveDelta() {
            wxCriticalSectionLocker lock(m_critical);

            const wxLongLong currentTime = ::wxGetLocalTimeMillis();
            const float time = static_cast<float>((currentTime - m_lastPollTime).ToLong());
            m_lastPollTime = currentTime;

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
            if (m_up)
                delta += Vec3f::PosZ * dist;
            if (m_down)
                delta += Vec3f::NegZ * dist;
            return delta;
        }

        Vec2f FlyModeHelper::lookDelta() {
            if (!m_enabled)
                return Vec2f::Null;

            wxCriticalSectionLocker lock(m_critical);

            const Vec2f speed = lookSpeed();
            const float hAngle = static_cast<float>(m_currentMouseDelta.x) * speed.x();
            const float vAngle = static_cast<float>(m_currentMouseDelta.y) * speed.y();
            m_currentMouseDelta.x = m_currentMouseDelta.y = 0;
            return Vec2f(hAngle, vAngle);
        }

        Vec2f FlyModeHelper::lookSpeed() const {
            Vec2f speed(pref(Preferences::CameraFlyLookSpeed), pref(Preferences::CameraFlyLookSpeed));
            speed /= -50.0f;
            if (pref(Preferences::CameraFlyInvertV))
                speed[1] *= -1.0f;
            return speed;
        }

        float FlyModeHelper::moveSpeed() const {
            return pref(Preferences::CameraFlyMoveSpeed);
        }
    }
}
