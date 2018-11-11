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
#include "TemporarilySetAny.h"
#include "Renderer/Camera.h"
#include "View/ExecutableEvent.h"
#include "View/KeyboardShortcut.h"
#include "IO/Path.h"

#include <vecmath/vec.h>

#include <wx/time.h>

namespace TrenchBroom {
    namespace View {
        FlyModeHelper::FlyModeHelper(Renderer::Camera& camera) :
        m_camera(camera) {
            resetKeys();
            m_lastPollTime = ::wxGetLocalTimeMillis();
        }

        FlyModeHelper::~FlyModeHelper() {
            /* Since the window is already deleted when this destructor is called, we omit the cleanup.
            if (enabled())
                disable();
             */
        }

        void FlyModeHelper::pollAndUpdate() {
            const auto currentTime = ::wxGetLocalTimeMillis();
            const auto time = float((currentTime - m_lastPollTime).ToLong());
            m_lastPollTime = currentTime;

            if (anyKeyDown()) {
                const auto delta = moveDelta(time);
                if (!isZero(delta, vm::Cf::almostZero())) {
                    m_camera.moveBy(delta);
                }
            }
        }

        bool FlyModeHelper::keyDown(wxKeyEvent& event) {
            const KeyboardShortcut& forward = pref(Preferences::CameraFlyForward);
            const KeyboardShortcut& backward = pref(Preferences::CameraFlyBackward);
            const KeyboardShortcut& left = pref(Preferences::CameraFlyLeft);
            const KeyboardShortcut& right = pref(Preferences::CameraFlyRight);
            const KeyboardShortcut& up = pref(Preferences::CameraFlyUp);
            const KeyboardShortcut& down = pref(Preferences::CameraFlyDown);

            const auto wasAnyKeyDown = anyKeyDown();
            auto anyMatch = false;
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

            if (anyKeyDown() && !wasAnyKeyDown) {
                // Reset the last polling time, otherwise the view will jump!
                m_lastPollTime = ::wxGetLocalTimeMillis();
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

        bool FlyModeHelper::anyKeyDown() const {
            return m_forward || m_backward || m_left || m_right || m_up || m_down;
        }

        void FlyModeHelper::resetKeys() {
            m_forward = m_backward = m_left = m_right = m_up = m_down = false;
        }

        vm::vec3f FlyModeHelper::moveDelta(const float time) {
            const float dist = moveSpeed() * time;

            vm::vec3f delta;
            if (m_forward) {
                delta = delta + m_camera.direction() * dist;
            }
            if (m_backward) {
                delta = delta - m_camera.direction() * dist;
            }
            if (m_left) {
                delta = delta - m_camera.right() * dist;
            }
            if (m_right) {
                delta = delta + m_camera.right() * dist;
            }
            if (m_up) {
                delta = delta + vm::vec3f::pos_z * dist;
            }
            if (m_down) {
                delta = delta - vm::vec3f::pos_z * dist;
            }
            return delta;
        }

        float FlyModeHelper::moveSpeed() const {
            return pref(Preferences::CameraFlyMoveSpeed);
        }
    }
}
