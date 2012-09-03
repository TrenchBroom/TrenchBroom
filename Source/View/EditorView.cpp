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

#include "EditorView.h"

#include "Controller/CameraEvent.h"
#include "Controller/Command.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Model/MapDocument.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"
#include "View/EditorFrame.h"
#include "View/MapGLCanvas.h"

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EditorView, wxView)
        EVT_CAMERA_MOVE(EditorView::OnCameraMove)
        EVT_CAMERA_LOOK(EditorView::OnCameraLook)
        EVT_CAMERA_ORBIT(EditorView::OnCameraOrbit)
        END_EVENT_TABLE()

        IMPLEMENT_DYNAMIC_CLASS(EditorView, wxView);
        
        EditorView::EditorView() : wxView(), m_camera(NULL), m_renderer(NULL) {}
        
        Utility::Console& EditorView::Console() const {
            return *m_console;
        }

        Renderer::Camera& EditorView::Camera() const {
            return *m_camera;
        }
        
        Renderer::MapRenderer& EditorView::Renderer() const {
            return *m_renderer;
        }
        
        bool EditorView::OnCreate(wxDocument* doc, long flags) {
            m_console = new Utility::Console();
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float fieldOfVision = prefs.getFloat(Preferences::CameraFieldOfVision);
            float nearPlane = prefs.getFloat(Preferences::CameraNearPlane);
            float farPlane = prefs.getFloat(Preferences::CameraFarPlane);
            Vec3f position(0.0f, 0.0f, 0.0f);
            Vec3f direction(1.0f, 0.0f, 0.0f);
            m_camera = new Renderer::Camera(fieldOfVision, nearPlane, farPlane, position, direction);
            
            Model::MapDocument& document = *static_cast<Model::MapDocument*>(doc);
            m_renderer = new Renderer::MapRenderer(document);
            m_renderer->loadMap();
            
            EditorFrame* frame = new EditorFrame(document, *this);
            SetFrame(frame);
            frame->Show();

            m_console->setTextCtrl(frame->LogView());
            
            return wxView::OnCreate(doc, flags);
        }
        
        void EditorView::OnUpdate(wxView* sender, wxObject* hint) {
            if (hint != NULL) {
                Controller::Command* command = static_cast<Controller::Command*>(hint);
                switch (command->type()) {
                    case Controller::Command::LoadMap:
                    case Controller::Command::ClearMap: {
                        m_renderer->loadMap();
                        break;
                    }
                    case Controller::Command::ChangeEditState: {
                        Controller::ChangeEditStateCommand* changeEditStateCommand = static_cast<Controller::ChangeEditStateCommand*>(command);
                        m_renderer->changeEditState(changeEditStateCommand->changeSet());
                        break;
                    }
                    default:
                        break;
                }
            }

            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            if (frame != NULL)
                frame->MapCanvas().Refresh();
        }
        
        void EditorView::OnDraw(wxDC* dc) {
        }
        
        bool EditorView::OnClose(bool deleteWindow) {
            if (deleteWindow)
                SetFrame(NULL);
            
            delete m_camera;
            m_camera = NULL;
            delete m_renderer;
            m_renderer = NULL;
            return wxView::OnClose(deleteWindow);
        }

        void EditorView::OnCameraMove(Controller::CameraMoveEvent& event) {
            m_camera->moveBy(event.forward(), event.right(), event.up());
            OnUpdate(this);
        }
        
        void EditorView::OnCameraLook(Controller::CameraLookEvent& event) {
            m_camera->rotate(event.hAngle(), event.vAngle());
            OnUpdate(this);
        }
        
        void EditorView::OnCameraOrbit(Controller::CameraOrbitEvent& event) {
            m_camera->orbit(event.center(), event.hAngle(), event.vAngle());
            OnUpdate(this);
        }
        
    }
}