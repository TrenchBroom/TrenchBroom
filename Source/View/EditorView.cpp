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
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Face.h"
#include "Model/Filter.h"
#include "Model/MapDocument.h"
#include "Model/Map.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"
#include "View/EditorFrame.h"
#include "View/MapGLCanvas.h"
#include "View/MenuCommandIds.h"

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EditorView, wxView)
        EVT_CAMERA_MOVE(EditorView::OnCameraMove)
        EVT_CAMERA_LOOK(EditorView::OnCameraLook)
        EVT_CAMERA_ORBIT(EditorView::OnCameraOrbit)

        EVT_MENU(MenuCommandIds::tbID_EDIT_SELECT_ALL, EditorView::OnEditSelectAll)
        EVT_MENU(MenuCommandIds::tbID_EDIT_SELECT_NONE, EditorView::OnEditSelectNone)
        
        EVT_UPDATE_UI_RANGE(MenuCommandIds::tbID_MENU_LOWEST, MenuCommandIds::tbID_MENU_HIGHEST, EditorView::OnUpdateMenuItem)
        END_EVENT_TABLE()

        IMPLEMENT_DYNAMIC_CLASS(EditorView, wxView);
        
        EditorView::EditorView() : wxView(), m_camera(NULL), m_renderer(NULL), m_filter(NULL) {}
        
        Model::Filter& EditorView::Filter() const {
            return *m_filter;
        }
        
        Model::MapDocument& EditorView::MapDocument() const {
            return *static_cast<Model::MapDocument*>(GetDocument());
        }

        Renderer::Camera& EditorView::Camera() const {
            return *m_camera;
        }
        
        Renderer::MapRenderer& EditorView::Renderer() const {
            return *m_renderer;
        }
        
        Utility::Console& EditorView::Console() const {
            return *m_console;
        }
        
        void EditorView::Submit(wxCommand* command) {
            MapDocument().GetCommandProcessor()->Submit(command);
        }

        bool EditorView::OnCreate(wxDocument* doc, long flags) {
            m_console = new Utility::Console();
            m_filter = new Model::Filter();
            
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
            
            if (m_filter != NULL) {
                delete m_filter;
                m_filter = NULL;
            }
            if (m_camera != NULL) {
                delete m_camera;
                m_camera = NULL;
            }
            if (m_renderer != NULL) {
                delete m_renderer;
                m_renderer = NULL;
            }
            
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
        
        void EditorView::OnEditSelectAll(wxCommandEvent& event) {
            const Model::EntityList& entities = MapDocument().Map().entities();
            Model::EntityList selectEntities;
            Model::BrushList selectBrushes;
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity& entity = *entities[i];
                if (entity.selectable()) {
                    selectEntities.push_back(&entity);
                } else {
                    const Model::BrushList& entityBrushes = entity.brushes();
                    selectBrushes.insert(selectBrushes.end(), entityBrushes.begin(), entityBrushes.end());
                }
            }
    
            wxCommand* command = Controller::ChangeEditStateCommand::replace(MapDocument(), selectEntities, selectBrushes);
            Submit(command);
        }

        void EditorView::OnEditSelectNone(wxCommandEvent& event) {
            wxCommand* command = Controller::ChangeEditStateCommand::deselectAll(MapDocument());
            Submit(command);
        }
        
        void EditorView::OnUpdateMenuItem(wxUpdateUIEvent& event) {
            Model::EditStateManager& editStateManager = MapDocument().EditStateManager();
            switch (event.GetId()) {
                case MenuCommandIds::tbID_EDIT_SELECT_ALL:
                    break;
                case MenuCommandIds::tbID_EDIT_SELECT_SIBLINGS:
                    event.Enable(false);
                    break;
                case MenuCommandIds::tbID_EDIT_SELECT_TOUCHING:
                    event.Enable(false);
                    break;
                case MenuCommandIds::tbID_EDIT_SELECT_NONE:
                    return event.Enable(editStateManager.selectionMode() != Model::EditStateManager::None);
                    break;
            }
        }
    }
}