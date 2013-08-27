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

#include "FlyTool.h"

#include "Controller/CameraEvent.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"
#include "View/EditorFrame.h"
#include "View/EditorView.h"
#include "View/MapGLCanvas.h"

namespace TrenchBroom {
    namespace Controller {
        void FlyTool::execute() {
            if (!holderValid())
                return;

            if (!wxTheApp->IsActive() || !view().editorFrame().IsActive() || !view().editorFrame().mapCanvas().HasFocus())
                return;

            if (wxGetKeyState(WXK_SHIFT) || wxGetKeyState(WXK_CONTROL) || wxGetKeyState(WXK_ALT))
                return;

            Renderer::Camera& camera = view().camera();
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            const wxLongLong updateTime = wxGetLocalTimeMillis();

            const float interval = static_cast<float>((updateTime - m_lastUpdateTime).ToDouble());
            const float distance = interval / 1000.0f * 320.0f;
            Vec3f direction;
            if (wxGetKeyState(wxKeyCode(prefs.getKeyboardShortcut(Preferences::CameraMoveForward).key())))
                direction += camera.direction();
            if (wxGetKeyState(wxKeyCode(prefs.getKeyboardShortcut(Preferences::CameraMoveBackward).key())))
                direction -= camera.direction();
            if (wxGetKeyState(wxKeyCode(prefs.getKeyboardShortcut(Preferences::CameraMoveLeft).key())))
                direction -= camera.right();
            if (wxGetKeyState(wxKeyCode(prefs.getKeyboardShortcut(Preferences::CameraMoveRight).key())))
                direction += camera.right();

            if (!direction.null()) {
                direction.normalize();
                const float forward = direction.dot(camera.direction()) * distance;
                const float right = direction.dot(camera.right()) * distance;
                const float up = direction.dot(camera.up()) * distance;
                const Vec3f delta(forward, right, up);

                Controller::CameraMoveEvent moveEvent;
                moveEvent.setDelta(delta);
                postEvent(moveEvent);
            }
            m_lastUpdateTime = updateTime;
        }

        FlyTool::FlyTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        Tool(documentViewHolder, inputController, false) {
            for (MoveKey::Type i = 0; i < 4; i++)
                m_moveKeys[i] = 0;
            m_lastUpdateTime = wxGetLocalTimeMillis();
            Start(1000 / 60);
        }

        FlyTool::~FlyTool() {
            Stop();
        }

        void FlyTool::Notify() {
            wxTheApp->QueueEvent(new ExecutableEvent(this));
        }
    }
}
