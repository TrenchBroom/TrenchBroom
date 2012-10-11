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
#include "Controller/CreateEntityCommand.h"
#include "Controller/DeleteObjectsCommand.h"
#include "Controller/DeleteObjectsCommand.h"
#include "Controller/EntityPropertyCommand.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Face.h"
#include "Model/Filter.h"
#include "Model/MapDocument.h"
#include "Model/Map.h"
#include "Model/TextureManager.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "Utility/CommandProcessor.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"
#include "View/CommandIds.h"
#include "View/EditorFrame.h"
#include "View/EntityInspector.h"
#include "View/FaceInspector.h"
#include "View/Inspector.h"
#include "View/MapGLCanvas.h"
#include "View/ViewOptions.h"

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EditorView, wxView)
        EVT_CAMERA_MOVE(EditorView::OnCameraMove)
        EVT_CAMERA_LOOK(EditorView::OnCameraLook)
        EVT_CAMERA_ORBIT(EditorView::OnCameraOrbit)

        EVT_MENU(wxID_UNDO, EditorView::OnUndo)
        EVT_MENU(wxID_REDO, EditorView::OnRedo)

        EVT_MENU(wxID_DELETE, EditorView::OnEditDelete)
        
        EVT_MENU(CommandIds::Menu::EditSelectAll, EditorView::OnEditSelectAll)
        EVT_MENU(CommandIds::Menu::EditSelectNone, EditorView::OnEditSelectNone)
        
        EVT_MENU(CommandIds::Menu::EditHideSelected, EditorView::OnEditHideSelected)
        EVT_MENU(CommandIds::Menu::EditHideUnselected, EditorView::OnEditHideUnselected)
        EVT_MENU(CommandIds::Menu::EditUnhideAll, EditorView::OnEditUnhideAll)
        
        EVT_MENU(CommandIds::Menu::EditLockSelected, EditorView::OnEditLockSelected)
        EVT_MENU(CommandIds::Menu::EditLockUnselected, EditorView::OnEditLockUnselected)
        EVT_MENU(CommandIds::Menu::EditUnlockAll, EditorView::OnEditUnlockAll)

        EVT_UPDATE_UI(wxID_UNDO, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_REDO, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_CUT, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_COPY, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_PASTE, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_DELETE, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI_RANGE(CommandIds::Menu::Lowest, CommandIds::Menu::Highest, EditorView::OnUpdateMenuItem)
        END_EVENT_TABLE()

        IMPLEMENT_DYNAMIC_CLASS(EditorView, wxView);
        
        void EditorView::submit(wxCommand* command) {
            mapDocument().GetCommandProcessor()->Submit(command);
        }
        
        EditorView::EditorView() :
        wxView(),
        m_camera(NULL),
        m_renderer(NULL),
        m_filter(NULL),
        m_viewOptions(NULL) {}
        
        ViewOptions& EditorView::viewOptions() const {
            return *m_viewOptions;
        }

        Model::Filter& EditorView::filter() const {
            return *m_filter;
        }
        
        Model::MapDocument& EditorView::mapDocument() const {
            return *static_cast<Model::MapDocument*>(GetDocument());
        }

        Renderer::Camera& EditorView::camera() const {
            return *m_camera;
        }
        
        Renderer::MapRenderer& EditorView::renderer() const {
            return *m_renderer;
        }
        
        View::Inspector& EditorView::inspector() const {
            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            return frame->inspector();
        }

        Utility::Console& EditorView::console() const {
            return mapDocument().console();
        }
        
        bool EditorView::OnCreate(wxDocument* doc, long flags) {
            m_viewOptions = new ViewOptions();
            m_filter = new Model::DefaultFilter(*m_viewOptions);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float fieldOfVision = prefs.getFloat(Preferences::CameraFieldOfVision);
            float nearPlane = prefs.getFloat(Preferences::CameraNearPlane);
            float farPlane = prefs.getFloat(Preferences::CameraFarPlane);
            Vec3f position(0.0f, 0.0f, 0.0f);
            Vec3f direction(1.0f, 0.0f, 0.0f);
            m_camera = new Renderer::Camera(fieldOfVision, nearPlane, farPlane, position, direction);
            
            Model::MapDocument& document = *static_cast<Model::MapDocument*>(doc);
            m_renderer = new Renderer::MapRenderer(document);
            
            EditorFrame* frame = new EditorFrame(document, *this);
            console().setTextCtrl(frame->logView());

            SetFrame(frame);
            frame->Show();

            return wxView::OnCreate(doc, flags);
        }
        
        void EditorView::OnUpdate(wxView* sender, wxObject* hint) {
            if (hint != NULL) {
                Controller::Command* command = static_cast<Controller::Command*>(hint);
                switch (command->type()) {
                    case Controller::Command::LoadMap:
                        m_renderer->loadMap();
                        inspector().faceInspector().updateFaceAttributes();
                        inspector().faceInspector().updateTextureBrowser();
                        inspector().faceInspector().updateSelectedTexture();
                        inspector().faceInspector().updateTextureCollectionList();
                        inspector().entityInspector().updateProperties();
                        inspector().entityInspector().updateEntityBrowser();
                        break;
                    case Controller::Command::ClearMap:
                        m_renderer->clearMap();
                        inspector().faceInspector().updateFaceAttributes();
                        inspector().faceInspector().updateTextureBrowser();
                        inspector().faceInspector().updateSelectedTexture();
                        inspector().faceInspector().updateTextureCollectionList();
                        inspector().entityInspector().updateProperties();
                        inspector().entityInspector().updateEntityBrowser();
                        break;
                    case Controller::Command::ChangeEditState: {
                        Controller::ChangeEditStateCommand* changeEditStateCommand = static_cast<Controller::ChangeEditStateCommand*>(command);
                        m_renderer->changeEditState(changeEditStateCommand->changeSet());
                        inspector().faceInspector().updateFaceAttributes();
                        inspector().faceInspector().updateSelectedTexture();
                        inspector().entityInspector().updateProperties();
                        break;
                    }
                    case Controller::Command::InvalidateRendererEntityState:
                        m_renderer->invalidateEntities();
                        break;
                    case Controller::Command::InvalidateRendererBrushState:
                        m_renderer->invalidateBrushes();
                        break;
                    case Controller::Command::InvalidateRendererState:
                        m_renderer->invalidateAll();
                        break;
                    case Controller::Command::InvalidateEntityModelRendererCache:
                        m_renderer->invalidateEntityModelRendererCache();
                        break;
                    case Controller::Command::SetFaceAttribute: {
                        m_renderer->invalidateSelectedBrushes();
                        inspector().faceInspector().updateFaceAttributes();
                        break;
                    }
                    case Controller::Command::RemoveTextureCollection:
                    case Controller::Command::AddTextureCollection: {
                        m_renderer->invalidateAll();
                        inspector().faceInspector().updateFaceAttributes();
                        inspector().faceInspector().updateTextureBrowser();
                        inspector().faceInspector().updateSelectedTexture();
                        inspector().faceInspector().updateTextureCollectionList();
                        break;
                    }
                    case Controller::Command::CreateEntity: {
                        Controller::CreateEntityCommand* createEntityCommand = static_cast<Controller::CreateEntityCommand*>(command);
                        Model::Entity* entity = createEntityCommand->entity();
                        if (createEntityCommand->state() == Controller::Command::Doing)
                            m_renderer->addEntity(*entity);
                        else if (createEntityCommand->state() == Controller::Command::Undoing)
                            m_renderer->removeEntity(*entity);
                    }
                    case Controller::Command::SetEntityPropertyKey:
                    case Controller::Command::SetEntityPropertyValue:
                    case Controller::Command::RemoveEntityProperty: {
                        Controller::EntityPropertyCommand* entityPropertyCommand = static_cast<Controller::EntityPropertyCommand*>(command);
                        m_renderer->invalidateEntities();
                        if (entityPropertyCommand->definitionChanged())
                            m_renderer->invalidateEntityModelRendererCache();
                        inspector().entityInspector().updateProperties();
                        break;
                    }
                    case Controller::Command::DeleteObjects: {
                        Controller::DeleteObjectsCommand* deleteObjectsCommand = static_cast<Controller::DeleteObjectsCommand*>(command);
                        m_renderer->removeEntities(deleteObjectsCommand->deletedEntities());
                        if (!deleteObjectsCommand->deletedBrushes().empty())
                            m_renderer->invalidateBrushes();
                    }
                    case Controller::Command::UpdateFigures:
                        break;
                    default:
                        break;
                }
            }

            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            if (frame != NULL)
                frame->mapCanvas().Refresh();
        }
        
        void EditorView::OnDraw(wxDC* dc) {
        }
        
        bool EditorView::OnClose(bool deleteWindow) {
            if (!wxView::OnClose(deleteWindow))
                return false;
            
            if (deleteWindow) {
                EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                if (frame != NULL) {
					frame->Disable();
                    frame->disableProcessing();
                    frame->Destroy(); // don't call close because that method will try to destroy the document again
				}
            }

            if (m_filter != NULL) {
                delete m_filter;
                m_filter = NULL;
            }
            if (m_viewOptions != NULL) {
                delete m_viewOptions;
                m_viewOptions = NULL;
            }
            if (m_camera != NULL) {
                delete m_camera;
                m_camera = NULL;
            }
            if (m_renderer != NULL) {
                delete m_renderer;
                m_renderer = NULL;
            }
            return true;
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
        
        void EditorView::OnUndo(wxCommandEvent& event) {
            GetDocumentManager()->OnUndo(event);
        }
        
        void EditorView::OnRedo(wxCommandEvent& event) {
            GetDocumentManager()->OnRedo(event);
        }

        void EditorView::OnEditDelete(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList entities = editStateManager.selectedEntities();
            const Model::BrushList brushes = editStateManager.selectedBrushes();
            
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::deselectAll(mapDocument());
            Controller::DeleteObjectsCommand* deleteObjectsCommand = Controller::DeleteObjectsCommand::deleteObjects(mapDocument(), entities, brushes);

            wxCommandProcessor* commandProcessor = mapDocument().GetCommandProcessor();
            CommandProcessor::BeginGroup(commandProcessor, deleteObjectsCommand->GetName());
            commandProcessor->Submit(changeEditStateCommand);
            commandProcessor->Submit(deleteObjectsCommand);
            CommandProcessor::EndGroup(commandProcessor);
        }
        
        void EditorView::OnEditSelectAll(wxCommandEvent& event) {
            const Model::EntityList& entities = mapDocument().map().entities();
            Model::EntityList selectEntities;
            Model::BrushList selectBrushes;
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity& entity = *entities[i];
                if (entity.selectable() && m_filter->entityVisible(entity)) {
                    selectEntities.push_back(&entity);
                } else {
                    const Model::BrushList& entityBrushes = entity.brushes();
                    for (unsigned int j = 0; j < entityBrushes.size(); j++) {
                        Model::Brush& brush = *entityBrushes[j];
                        if (m_filter->brushVisible(brush))
                            selectBrushes.push_back(&brush);
                    }
                }
            }
    
            wxCommand* command = Controller::ChangeEditStateCommand::replace(mapDocument(), selectEntities, selectBrushes);
            submit(command);
        }

        void EditorView::OnEditSelectNone(wxCommandEvent& event) {
            wxCommand* command = Controller::ChangeEditStateCommand::deselectAll(mapDocument());
            submit(command);
        }
        
        void EditorView::OnEditHideSelected(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& hideEntities = editStateManager.selectedEntities();
            const Model::BrushList& hideBrushes = editStateManager.selectedBrushes();
            
            wxCommand* command = Controller::ChangeEditStateCommand::hide(mapDocument(), hideEntities, hideBrushes);
            submit(command);
        }
        
        void EditorView::OnEditHideUnselected(wxCommandEvent& event) {
            const Model::EntityList& entities = mapDocument().map().entities();
            Model::EntityList hideEntities;
            Model::BrushList hideBrushes;
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity& entity = *entities[i];
                if (!entity.selected() && entity.hideable()) {
                    hideEntities.push_back(&entity);

                    const Model::BrushList& entityBrushes = entity.brushes();
                    for (unsigned int j = 0; j < entityBrushes.size(); j++) {
                        Model::Brush& brush = *entityBrushes[j];
                        if (!brush.selected() && brush.hideable())
                            hideBrushes.push_back(&brush);
                    }
                }
            }
            
            wxCommand* command = Controller::ChangeEditStateCommand::hide(mapDocument(), hideEntities, hideBrushes);
            submit(command);
        }
        
        void EditorView::OnEditUnhideAll(wxCommandEvent& event) {
            wxCommand* command = Controller::ChangeEditStateCommand::unhideAll(mapDocument());
            submit(command);
        }
        
        void EditorView::OnEditLockSelected(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& lockEntities = editStateManager.selectedEntities();
            const Model::BrushList& lockBrushes = editStateManager.selectedBrushes();
            
            wxCommand* command = Controller::ChangeEditStateCommand::lock(mapDocument(), lockEntities, lockBrushes);
            submit(command);
        }
        
        void EditorView::OnEditLockUnselected(wxCommandEvent& event) {
            const Model::EntityList& entities = mapDocument().map().entities();
            Model::EntityList lockEntities;
            Model::BrushList lockBrushes;
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity& entity = *entities[i];
                if (!entity.selected() && entity.lockable()) {
                    lockEntities.push_back(&entity);
                    
                    const Model::BrushList& entityBrushes = entity.brushes();
                    for (unsigned int j = 0; j < entityBrushes.size(); j++) {
                        Model::Brush& brush = *entityBrushes[j];
                        if (!brush.selected() && brush.lockable())
                            lockBrushes.push_back(&brush);
                    }
                }
            }
            
            wxCommand* command = Controller::ChangeEditStateCommand::lock(mapDocument(), lockEntities, lockBrushes);
            submit(command);
        }
        
        void EditorView::OnEditUnlockAll(wxCommandEvent& event) {
            wxCommand* command = Controller::ChangeEditStateCommand::unlockAll(mapDocument());
            submit(command);
        }

        void EditorView::OnUpdateMenuItem(wxUpdateUIEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            switch (event.GetId()) {
                case wxID_UNDO:
                    GetDocumentManager()->OnUpdateUndo(event);
                    break;
                case wxID_REDO:
                    GetDocumentManager()->OnUpdateRedo(event);
                    break;
                case CommandIds::Menu::EditSelectAll:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditSelectSiblings:
                    event.Enable(false);
                    break;
                case CommandIds::Menu::EditSelectTouching:
                    event.Enable(false);
                    break;
                case CommandIds::Menu::EditSelectNone:
                    event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone);
                    break;
                case wxID_CUT:
                case wxID_COPY:
                case wxID_DELETE:
                    event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone &&
                                 editStateManager.selectionMode() != Model::EditStateManager::SMFaces);
                    break;
                case wxID_PASTE:
                    event.Enable(false);
                    break;
                case CommandIds::Menu::EditHideSelected:
                case CommandIds::Menu::EditHideUnselected:
                    event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone &&
                                 editStateManager.selectionMode() != Model::EditStateManager::SMFaces);
                    break;
                case CommandIds::Menu::EditUnhideAll:
                    event.Enable(editStateManager.hasHiddenObjects());
                    break;
                case CommandIds::Menu::EditLockSelected:
                case CommandIds::Menu::EditLockUnselected:
                    event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone &&
                                 editStateManager.selectionMode() != Model::EditStateManager::SMFaces);
                    break;
                case CommandIds::Menu::EditUnlockAll:
                    event.Enable(editStateManager.hasLockedObjects());
                    break;
            }
        }
    }
}