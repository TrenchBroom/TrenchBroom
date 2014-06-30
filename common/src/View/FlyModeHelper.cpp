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
#include "View/KeyboardShortcut.h"

#ifdef __APPLE__
// Don't include Quickdraw - we don't need it and it leads to symbol redefinition errrors
#define __QUICKDRAWAPI__
#include <ApplicationServices/ApplicationServices.h>
#endif

namespace TrenchBroom {
    namespace View {
        FlyModeHelper::Input::Input() :
        forward(false),
        backward(false),
        left(false),
        right(false),
        delta(0, 0),
        time(0) {}

        FlyModeHelper::FlyModeHelper(wxWindow* window) :
        m_window(window),
        m_enabled(false) {}
        
        FlyModeHelper::~FlyModeHelper() {
            if (enabled())
                disable();
        }
        
        void FlyModeHelper::enable() {
            assert(!enabled());
            
            m_forward = m_backward = m_left = m_right = false;
            
            m_window->Bind(wxEVT_KEY_DOWN, &FlyModeHelper::OnKeyDown, this);
            m_window->Bind(wxEVT_KEY_UP, &FlyModeHelper::OnKeyUp, this);

            lockMouse();
            
            m_lastPollTime = ::wxGetLocalTimeMillis();
            m_enabled = true;
        }
        
        void FlyModeHelper::disable() {
            assert(enabled());
            
            unlockMouse();
            m_enabled = false;
            m_window->Unbind(wxEVT_KEY_DOWN, &FlyModeHelper::OnKeyDown, this);
            m_window->Unbind(wxEVT_KEY_UP, &FlyModeHelper::OnKeyUp, this);
        }
        
        bool FlyModeHelper::enabled() const {
            return m_enabled;
        }
        
        FlyModeHelper::Input FlyModeHelper::poll() {
            assert(enabled());
            
            Input result;
            result.forward = m_forward;
            result.backward = m_backward;
            result.left = m_left;
            result.right = m_right;
            result.delta = mouseDelta();
            
            const wxLongLong currentTime = ::wxGetLocalTimeMillis();
            result.time = static_cast<unsigned int>((currentTime - m_lastPollTime).ToLong());
            m_lastPollTime = currentTime;
            
            return result;
        }
        
        void FlyModeHelper::lockMouse() {
            m_window->SetCursor(wxCursor(wxCURSOR_BLANK));
            
#ifdef __APPLE__
            CGAssociateMouseAndMouseCursorPosition(false);
            int32_t dx, dy;
            CGGetLastMouseDelta(&dx, &dy);
#else
            m_originalMousePos = m_window->ScreenToClient(::wxGetMousePosition());
            const wxPoint center = windowCenter();
            m_window->WarpPointer(center.x, center.y);
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
        
        wxPoint FlyModeHelper::mouseDelta() const {
#ifndef __APPLE__
            const wxPoint currentMousePos = m_window->ScreenToClient(::wxGetMousePosition());
            return currentMousePos - windowCenter();
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
            if (enabled())
                onKey(event, true);
        }
        
        void FlyModeHelper::OnKeyUp(wxKeyEvent& event) {
            if (enabled())
                onKey(event, false);
        }

        void FlyModeHelper::onKey(wxKeyEvent& event, const bool down) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const KeyboardShortcut& forward = prefs.get(Preferences::CameraForwardShortcut);
            const KeyboardShortcut& backward = prefs.get(Preferences::CameraBackwardShortcut);
            const KeyboardShortcut& left = prefs.get(Preferences::CameraLeftShortcut);
            const KeyboardShortcut& right = prefs.get(Preferences::CameraRightShortcut);
            
            if (forward.matches(event.GetKeyCode()))
                m_forward = down;
            else if (backward.matches(event.GetKeyCode()))
                m_backward = down;
            else if (left.matches(event.GetKeyCode()))
                m_left = down;
            else if (right.matches(event.GetKeyCode()))
                m_right = down;
            else
                event.Skip();
        }
    }
}
