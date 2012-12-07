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

#include "Controller/AddObjectsCommand.h"
#include "Controller/CameraEvent.h"
#include "Controller/Command.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Controller/FlipObjectsCommand.h"
#include "Controller/InputController.h"
#include "Controller/MoveObjectsCommand.h"
#include "Controller/MoveTextureSCommand.h"
#include "Controller/ObjectsCommand.h"
#include "Controller/RemoveObjectsCommand.h"
#include "Controller/RotateObjects90Command.h"
#include "Controller/RotateTexturesCommand.h"
#include "Controller/EntityPropertyCommand.h"
#include "Controller/SetFaceAttributesCommand.h"
#include "IO/MapParser.h"
#include "IO/MapWriter.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/Face.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Model/MapObject.h"
#include "Model/TextureManager.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "Utility/CommandProcessor.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"
#include "View/AbstractApp.h"
#include "View/CommandIds.h"
#include "View/EditorFrame.h"
#include "View/EntityInspector.h"
#include "View/FaceInspector.h"
#include "View/Inspector.h"
#include "View/MapGLCanvas.h"
#include "View/ViewOptions.h"

#include <wx/clipbrd.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EditorView, wxView)
        EVT_CAMERA_MOVE(EditorView::OnCameraMove)
        EVT_CAMERA_LOOK(EditorView::OnCameraLook)
        EVT_CAMERA_ORBIT(EditorView::OnCameraOrbit)

        EVT_MENU(wxID_SAVE, EditorView::OnFileSave)
        EVT_MENU(wxID_SAVEAS, EditorView::OnFileSaveAs)
        
        EVT_MENU(wxID_UNDO, EditorView::OnUndo)
        EVT_MENU(wxID_REDO, EditorView::OnRedo)

        EVT_MENU(wxID_CUT, EditorView::OnEditCut)
        EVT_MENU(wxID_COPY, EditorView::OnEditCopy)
        EVT_MENU(wxID_PASTE, EditorView::OnEditPaste)
        EVT_MENU(wxID_DELETE, EditorView::OnEditDelete)
        
        EVT_MENU(CommandIds::Menu::EditSelectAll, EditorView::OnEditSelectAll)
        EVT_MENU(CommandIds::Menu::EditSelectNone, EditorView::OnEditSelectNone)
        
        EVT_MENU(CommandIds::Menu::EditHideSelected, EditorView::OnEditHideSelected)
        EVT_MENU(CommandIds::Menu::EditHideUnselected, EditorView::OnEditHideUnselected)
        EVT_MENU(CommandIds::Menu::EditUnhideAll, EditorView::OnEditUnhideAll)
        
        EVT_MENU(CommandIds::Menu::EditLockSelected, EditorView::OnEditLockSelected)
        EVT_MENU(CommandIds::Menu::EditLockUnselected, EditorView::OnEditLockUnselected)
        EVT_MENU(CommandIds::Menu::EditUnlockAll, EditorView::OnEditUnlockAll)

        EVT_MENU(CommandIds::Menu::EditToggleClipTool, EditorView::OnEditToggleClipTool)
        EVT_MENU(CommandIds::Menu::EditToggleClipSide, EditorView::OnEditToggleClipSide)
        EVT_MENU(CommandIds::Menu::EditPerformClip, EditorView::OnEditPerformClip)
        EVT_MENU(CommandIds::Menu::EditToggleVertexTool, EditorView::OnEditToggleVertexTool)
        
        EVT_MENU(CommandIds::Menu::EditMoveTexturesUp, EditorView::OnEditMoveTexturesUp)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesRight, EditorView::OnEditMoveTexturesRight)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesDown, EditorView::OnEditMoveTexturesDown)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesLeft, EditorView::OnEditMoveTexturesLeft)
        EVT_MENU(CommandIds::Menu::EditRotateTexturesCW, EditorView::OnEditRotateTexturesCW)
        EVT_MENU(CommandIds::Menu::EditRotateTexturesCCW, EditorView::OnEditRotateTexturesCCW)
        
        EVT_MENU(CommandIds::Menu::EditMoveObjectsForward, EditorView::OnEditMoveObjectsForward)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsRight, EditorView::OnEditMoveObjectsRight)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsBackward, EditorView::OnEditMoveObjectsBackward)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsLeft, EditorView::OnEditMoveObjectsLeft)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsUp, EditorView::OnEditMoveObjectsUp)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsDown, EditorView::OnEditMoveObjectsDown)
        EVT_MENU(CommandIds::Menu::EditRollObjectsCW, EditorView::OnEditRollObjectsCW)
        EVT_MENU(CommandIds::Menu::EditRollObjectsCCW, EditorView::OnEditRollObjectsCCW)
        EVT_MENU(CommandIds::Menu::EditPitchObjectsCW, EditorView::OnEditPitchObjectsCW)
        EVT_MENU(CommandIds::Menu::EditPitchObjectsCCW, EditorView::OnEditPitchObjectsCCW)
        EVT_MENU(CommandIds::Menu::EditYawObjectsCW, EditorView::OnEditYawObjectsCW)
        EVT_MENU(CommandIds::Menu::EditYawObjectsCCW, EditorView::OnEditYawObjectsCCW)
        EVT_MENU(CommandIds::Menu::EditFlipObjectsHorizontally, EditorView::OnEditFlipObjectsH)
        EVT_MENU(CommandIds::Menu::EditFlipObjectsVertically, EditorView::OnEditFlipObjectsV)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjects, EditorView::OnEditDuplicateObjects)
        
        EVT_MENU(CommandIds::Menu::EditToggleTextureLock, EditorView::OnEditToggleTextureLock)
        
        EVT_UPDATE_UI(wxID_SAVE, EditorView::OnUpdateMenuItem)
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
        
        void EditorView::moveTextures(Direction direction, bool snapToGrid) {
            Vec3f moveDirection;
            switch (direction) {
                case DUp:
                    moveDirection = m_camera->up();
                    break;
                case DRight:
                    moveDirection = m_camera->right();
                    break;
                case DDown:
                    moveDirection = m_camera->up() * -1.0f;
                    break;
                case DLeft:
                    moveDirection = m_camera->right() * -1.0f;
                    break;
                default:
                    assert(false);
            }
            
            float distance = snapToGrid ? static_cast<float>(mapDocument().grid().actualSize()) : 1.0f;

            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::FaceList& faces = editStateManager.selectedFaces();
            wxString actionName = faces.size() == 1 ? wxT("Move Texture") : wxT("Move Textures");

            Controller::MoveTexturesCommand* command = Controller::MoveTexturesCommand::moveTextures(mapDocument(), actionName, distance, moveDirection);
            submit(command);
        }

        void EditorView::rotateTextures(bool clockwise, bool snapToGrid) {
            float angle = snapToGrid ? mapDocument().grid().angle() : 1.0f;

            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::FaceList& faces = editStateManager.selectedFaces();
            
            Controller::RotateTexturesCommand* command = NULL;
            if (clockwise)
                command = Controller::RotateTexturesCommand::rotateClockwise(mapDocument(), faces, angle);
            else
                command = Controller::RotateTexturesCommand::rotateCounterClockwise(mapDocument(), faces, angle);
            submit(command);
        }

        void EditorView::moveObjects(Direction direction, bool snapToGrid) {
            Vec3f moveDirection;
            switch (direction) {
                case DUp:
                    moveDirection = Vec3f::PosZ;
                    break;
                case DRight:
                    moveDirection = m_camera->right().firstAxis();
                    break;
                case DDown:
                    moveDirection = Vec3f::NegZ;
                    break;
                case DLeft:
                    moveDirection = (m_camera->right() * -1.0f).firstAxis();
                    break;
                case DForward:
                    moveDirection = m_camera->direction().firstAxis();
                    if (moveDirection.firstComponent() == Axis::AZ)
                        moveDirection = m_camera->direction().secondAxis();
                    break;
                case DBackward:
                    moveDirection = (m_camera->direction() * -1.0f).firstAxis();
                    if (moveDirection.firstComponent() == Axis::AZ)
                        moveDirection = (m_camera->direction() * -1.0f).secondAxis();
                    break;
            }
            
            float dist = snapToGrid ? static_cast<float>(mapDocument().grid().actualSize()) : 1.0f;
            Vec3f delta = moveDirection * dist;
            
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            Controller::MoveObjectsCommand* command = Controller::MoveObjectsCommand::moveObjects(mapDocument(), entities, brushes, delta, mapDocument().textureLock());
            submit(command);
        }

        void EditorView::rotateObjects(RotationAxis rotationAxis, bool clockwise) {
            Axis::Type absoluteAxis;
            switch (rotationAxis) {
                case ARoll:
                    absoluteAxis = m_camera->direction().firstComponent();
                    break;
                case APitch:
                    absoluteAxis = m_camera->right().firstComponent();
                    break;
                case AYaw:
                    absoluteAxis = Axis::AZ;
                    break;
            }
            
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            Vec3f center = Model::MapObject::center(entities, brushes);
            
            Controller::RotateObjects90Command* command = NULL;
            if (clockwise)
                command = Controller::RotateObjects90Command::rotateClockwise(mapDocument(), entities, brushes, absoluteAxis, center, mapDocument().textureLock());
            else
                command = Controller::RotateObjects90Command::rotateCounterClockwise(mapDocument(), entities, brushes, absoluteAxis, center, mapDocument().textureLock());
            submit(command);
        }

        void EditorView::flipObjects(bool horizontally) {
            Axis::Type axis = horizontally ? m_camera->right().firstComponent() : Axis::AZ;
            
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            Vec3f center = Model::MapObject::center(entities, brushes);
            Controller::FlipObjectsCommand* command = Controller::FlipObjectsCommand::flip(mapDocument(), entities, brushes, axis, center, mapDocument().textureLock());
            submit(command);
        }

        void EditorView::removeObjects(const wxString& actionName) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList entities = editStateManager.selectedEntities();
            const Model::BrushList brushes = editStateManager.selectedBrushes();
            
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::deselectAll(mapDocument());
            Controller::RemoveObjectsCommand* deleteObjectsCommand = Controller::RemoveObjectsCommand::removeObjects(mapDocument(), entities, brushes);
            
            wxCommandProcessor* commandProcessor = mapDocument().GetCommandProcessor();
            CommandProcessor::BeginGroup(commandProcessor, actionName);
            commandProcessor->Submit(changeEditStateCommand);
            commandProcessor->Submit(deleteObjectsCommand);
            CommandProcessor::EndGroup(commandProcessor);
        }

        bool EditorView::canPaste() {
            bool result = false;
            if (wxTheClipboard->Open()) {
                if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                    Model::EntityList entities;
                    Model::BrushList brushes;
                    Model::FaceList faces;
                    
                    wxTextDataObject clipboardData;
                    wxTheClipboard->GetData(clipboardData);
                    StringStream stream;
                    stream.str(clipboardData.GetText().ToStdString());
                    IO::MapParser mapParser(stream, console());
                    
                    if (mapParser.parseEntities(mapDocument().map().worldBounds(), entities))
                        result = true;
                    else if (mapParser.parseBrushes(mapDocument().map().worldBounds(), brushes))
                        result = true;
                    else if (mapParser.parseFaces(mapDocument().map().worldBounds(), faces))
                        result = true;
                }
                wxTheClipboard->Close();
            }
            return result;
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
        
        Controller::InputController& EditorView::inputController() const {
            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            return frame->mapCanvas().inputController();
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
                        inputController().objectsChange();
                        
                        static_cast<EditorFrame*>(GetFrame())->updateMenuBar();
                        break;
                    case Controller::Command::ClearMap:
                        m_renderer->clearMap();
                        inspector().faceInspector().updateFaceAttributes();
                        inspector().faceInspector().updateTextureBrowser();
                        inspector().faceInspector().updateSelectedTexture();
                        inspector().faceInspector().updateTextureCollectionList();
                        inspector().entityInspector().updateProperties();
                        inspector().entityInspector().updateEntityBrowser();
                        inputController().objectsChange();
                        
                        static_cast<EditorFrame*>(GetFrame())->updateMenuBar();
                        break;
                    case Controller::Command::ChangeEditState: {
                        Controller::ChangeEditStateCommand* changeEditStateCommand = static_cast<Controller::ChangeEditStateCommand*>(command);
                        m_renderer->changeEditState(changeEditStateCommand->changeSet());

                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateMenuBar();
                        inputController().editStateChange(changeEditStateCommand->changeSet());
                        if (mapDocument().editStateManager().selectedBrushes().empty() && inputController().clipToolActive())
                            inputController().toggleClipTool();

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
                    case Controller::Command::SetFaceAttributes:
                    case Controller::Command::MoveTextures:
                    case Controller::Command::RotateTextures: {
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
                    case Controller::Command::SetEntityPropertyKey:
                    case Controller::Command::SetEntityPropertyValue:
                    case Controller::Command::RemoveEntityProperty: {
                        Controller::EntityPropertyCommand* entityPropertyCommand = static_cast<Controller::EntityPropertyCommand*>(command);
                        m_renderer->invalidateEntities();
                        if (entityPropertyCommand->definitionChanged())
                            m_renderer->invalidateEntityModelRendererCache();
                        inspector().entityInspector().updateProperties();
                        inputController().objectsChange();
                        break;
                    }
                    case Controller::Command::AddObjects: {
                        Controller::AddObjectsCommand* addObjectsCommand = static_cast<Controller::AddObjectsCommand*>(command);
                        if (addObjectsCommand->state() == Controller::Command::Doing)
                            m_renderer->addEntities(addObjectsCommand->addedEntities());
                        else
                            m_renderer->removeEntities(addObjectsCommand->addedEntities());
                        if (addObjectsCommand->hasAddedBrushes())
                            m_renderer->invalidateBrushes();
                        break;
                    }
                    case Controller::Command::MoveObjects:
                    case Controller::Command::RotateObjects:
                    case Controller::Command::FlipObjects: {
                        m_renderer->invalidateSelectedBrushes();
                        m_renderer->invalidateSelectedEntities();
                        inspector().entityInspector().updateProperties();
                        inputController().objectsChange();
                        break;
                    }
                    case Controller::Command::RemoveObjects: {
                        Controller::RemoveObjectsCommand* removeObjectsCommand = static_cast<Controller::RemoveObjectsCommand*>(command);
                        if (removeObjectsCommand->state() == Controller::Command::Doing)
                            m_renderer->removeEntities(removeObjectsCommand->removedEntities());
                        else
                            m_renderer->addEntities(removeObjectsCommand->removedEntities());
                        if (!removeObjectsCommand->removedBrushes().empty())
                            m_renderer->invalidateBrushes();
                        break;
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
            inputController().cameraChange();
            OnUpdate(this);
        }
        
        void EditorView::OnCameraLook(Controller::CameraLookEvent& event) {
            m_camera->rotate(event.hAngle(), event.vAngle());
            inputController().cameraChange();
            OnUpdate(this);
        }
        
        void EditorView::OnCameraOrbit(Controller::CameraOrbitEvent& event) {
            m_camera->orbit(event.center(), event.hAngle(), event.vAngle());
            inputController().cameraChange();
            OnUpdate(this);
        }
        
        void EditorView::OnFileSave(wxCommandEvent& event) {
            GetDocument()->Save();
        }
        
        void EditorView::OnFileSaveAs(wxCommandEvent& event) {
            GetDocument()->SaveAs();
        }

        void EditorView::OnUndo(wxCommandEvent& event) {
            GetDocumentManager()->OnUndo(event);
        }
        
        void EditorView::OnRedo(wxCommandEvent& event) {
            GetDocumentManager()->OnRedo(event);
        }

        void EditorView::OnEditCut(wxCommandEvent& event) {
            if (wxTextCtrl* textCtrl = wxDynamicCast(GetFrame()->FindFocus(), wxTextCtrl)) {
                textCtrl->Cut();
            } else {
                OnEditCopy(event);
                removeObjects(wxT("Cut"));
            }
        }
        
        void EditorView::OnEditCopy(wxCommandEvent& event) {
            if (wxTextCtrl* textCtrl = wxDynamicCast(GetFrame()->FindFocus(), wxTextCtrl)) {
                textCtrl->Copy();
            } else {
                Model::EditStateManager& editStateManager = mapDocument().editStateManager();
                assert(editStateManager.selectionMode() == Model::EditStateManager::SMFaces ||
                       editStateManager.selectionMode() == Model::EditStateManager::SMEntities ||
                       editStateManager.selectionMode() == Model::EditStateManager::SMBrushes ||
                       editStateManager.selectionMode() == Model::EditStateManager::SMEntitiesAndBrushes);
                
                if (wxTheClipboard->Open()) {
                    StringStream clipboardData;
                    IO::MapWriter mapWriter;
                    if (editStateManager.selectionMode() == Model::EditStateManager::SMFaces)
                        mapWriter.writeFacesToStream(editStateManager.selectedFaces(), clipboardData);
                    else
                        mapWriter.writeObjectsToStream(editStateManager.selectedEntities(), editStateManager.selectedBrushes(), clipboardData);
                    
                    wxTheClipboard->SetData(new wxTextDataObject(clipboardData.str()));
                    wxTheClipboard->Close();
                }
            }
        }
        
        void EditorView::OnEditPaste(wxCommandEvent& event) {
            if (wxTextCtrl* textCtrl = wxDynamicCast(GetFrame()->FindFocus(), wxTextCtrl)) {
                textCtrl->Paste();
            } else {
                if (wxTheClipboard->Open()) {
                    if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                        Model::EntityList entities;
                        Model::BrushList brushes;
                        Model::FaceList faces;
                        
                        wxTextDataObject clipboardData;
                        wxTheClipboard->GetData(clipboardData);
                        StringStream stream;
                        stream.str(clipboardData.GetText().ToStdString());
                        IO::MapParser mapParser(stream, console());
                        
                        if (mapParser.parseFaces(mapDocument().map().worldBounds(), faces)) {
                            assert(!faces.empty());
                            
                            Model::Face& face = *faces.back();
                            Model::TextureManager& textureManager = mapDocument().textureManager();
                            Model::Texture* texture = textureManager.texture(face.textureName());
                            face.setTexture(texture);
                            
                            const Model::FaceList& selectedFaces = mapDocument().editStateManager().selectedFaces();
                            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(mapDocument(), selectedFaces, wxT("Paste Faces"));
                            command->setTemplate(face);
                            submit(command);
                        } else {
                            mapParser.parseEntities(mapDocument().map().worldBounds(), entities);
                            mapParser.parseBrushes(mapDocument().map().worldBounds(), brushes);
                            assert(entities.empty() != brushes.empty());
                            
                            Model::EntityList selectEntities;
                            Model::BrushList selectBrushes;
                            
                            Model::EntityList::iterator entityIt, entityEnd;
                            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                                Model::Entity& entity = **entityIt;
                                const Model::BrushList& entityBrushes = entity.brushes();
                                if (!entityBrushes.empty())
                                    selectBrushes.insert(selectBrushes.begin(), entityBrushes.begin(), entityBrushes.end());
                                else
                                    selectEntities.push_back(&entity);
                            }
                            
                            Vec3f delta = camera().defaultPoint() - Model::MapObject::center(entities, brushes);
                            delta = mapDocument().grid().snap(delta);
                            
                            Controller::AddObjectsCommand* addObjectsCommand = Controller::AddObjectsCommand::addObjects(mapDocument(), entities, brushes);
                            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::replace(mapDocument(), selectEntities, selectBrushes);
                            Controller::MoveObjectsCommand* moveObjectsCommand = Controller::MoveObjectsCommand::moveObjects(mapDocument(), selectEntities, selectBrushes, delta, mapDocument().textureLock());
                            
                            wxCommandProcessor* commandProcessor = mapDocument().GetCommandProcessor();
                            CommandProcessor::BeginGroup(commandProcessor, Controller::Command::makeObjectActionName(wxT("Paste"), selectEntities, selectBrushes));
                            submit(addObjectsCommand);
                            submit(changeEditStateCommand);
                            submit(moveObjectsCommand);
                            CommandProcessor::EndGroup(commandProcessor);
                        }
                    }
                    wxTheClipboard->Close();
                }
            }
        }

        void EditorView::OnEditDelete(wxCommandEvent& event) {
            removeObjects(wxT("Delete"));
        }
        
        void EditorView::OnEditSelectAll(wxCommandEvent& event) {
            if (wxTextCtrl* textCtrl = wxDynamicCast(GetFrame()->FindFocus(), wxTextCtrl)) {
                textCtrl->SelectAll();
            } else {
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

        void EditorView::OnEditToggleClipTool(wxCommandEvent& event) {
            inputController().toggleClipTool();
        }
        
        void EditorView::OnEditToggleClipSide(wxCommandEvent& event) {
            inputController().toggleClipSide();
        }
        
        void EditorView::OnEditPerformClip(wxCommandEvent& event) {
            inputController().performClip();
        }

        void EditorView::OnEditToggleVertexTool(wxCommandEvent& event) {
            inputController().toggleMoveVerticesTool();
        }

        void EditorView::OnEditMoveObjectsForward(wxCommandEvent& event) {
            moveObjects(DForward, true);
        }

        void EditorView::OnEditMoveObjectsRight(wxCommandEvent& event) {
            moveObjects(DRight, true);
        }
        
        void EditorView::OnEditMoveObjectsBackward(wxCommandEvent& event) {
            moveObjects(DBackward, true);
        }
        
        void EditorView::OnEditMoveObjectsLeft(wxCommandEvent& event) {
            moveObjects(DLeft, true);
        }
        
        void EditorView::OnEditMoveObjectsUp(wxCommandEvent& event) {
            moveObjects(DUp, true);
        }
        
        void EditorView::OnEditMoveObjectsDown(wxCommandEvent& event) {
            moveObjects(DDown, true);
        }

        void EditorView::OnEditMoveTexturesUp(wxCommandEvent& event) {
            moveTextures(DUp, true);
        }
        
        void EditorView::OnEditMoveTexturesRight(wxCommandEvent& event) {
            moveTextures(DRight, true);
        }
        
        void EditorView::OnEditMoveTexturesDown(wxCommandEvent& event) {
            moveTextures(DDown, true);
        }
        
        void EditorView::OnEditMoveTexturesLeft(wxCommandEvent& event) {
            moveTextures(DLeft, true);
        }
        
        void EditorView::OnEditRotateTexturesCW(wxCommandEvent& event) {
            rotateTextures(true, true);
        }
        
        void EditorView::OnEditRotateTexturesCCW(wxCommandEvent& event) {
            rotateTextures(false, true);
        }

        void EditorView::OnEditRollObjectsCW(wxCommandEvent& event) {
            rotateObjects(ARoll, true);
        }
        
        void EditorView::OnEditRollObjectsCCW(wxCommandEvent& event) {
            rotateObjects(ARoll, false);
        }
        
        void EditorView::OnEditPitchObjectsCW(wxCommandEvent& event) {
            rotateObjects(APitch, true);
        }
        
        void EditorView::OnEditPitchObjectsCCW(wxCommandEvent& event) {
            rotateObjects(APitch, false);
        }
        
        void EditorView::OnEditYawObjectsCW(wxCommandEvent& event) {
            rotateObjects(AYaw, true);
        }
        
        void EditorView::OnEditYawObjectsCCW(wxCommandEvent& event) {
            rotateObjects(AYaw, false);
        }

        void EditorView::OnEditFlipObjectsH(wxCommandEvent& event) {
            flipObjects(true);
        }
        
        void EditorView::OnEditFlipObjectsV(wxCommandEvent& event) {
            flipObjects(false);
        }
        
        void EditorView::OnEditDuplicateObjects(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& originalEntities = editStateManager.selectedEntities();
            const Model::BrushList& originalBrushes = editStateManager.selectedBrushes();
            
            Model::EntityList newEntities;
            Model::BrushList newBrushes;
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = originalEntities.begin(), entityEnd = originalEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                assert(entity.definition() == NULL || entity.definition()->type() == Model::EntityDefinition::PointEntity);
                assert(!entity.worldspawn());
                
                Model::Entity* newEntity = new Model::Entity(mapDocument().map().worldBounds(), entity);
                newEntities.push_back(newEntity);
            }
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = originalBrushes.begin(), brushEnd = originalBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                
                Model::Brush* newBrush = new Model::Brush(mapDocument().map().worldBounds(), brush);
                newBrushes.push_back(newBrush);
            }
            
            const Vec3f direction = (-1.0f * m_camera->direction() + m_camera->right()) / 2.0f;
            Vec3f delta = mapDocument().grid().snapTowards(Vec3f::Null, direction, true);
            delta.z = 0.0f;
            
            Controller::AddObjectsCommand* addObjectsCommand = Controller::AddObjectsCommand::addObjects(mapDocument(), newEntities, newBrushes);
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::replace(mapDocument(), newEntities, newBrushes);
            Controller::MoveObjectsCommand* moveObjectsCommand = Controller::MoveObjectsCommand::moveObjects(mapDocument(), newEntities, newBrushes, delta, mapDocument().textureLock());
            
            wxCommandProcessor* commandProcessor = mapDocument().GetCommandProcessor();
            CommandProcessor::BeginGroup(commandProcessor, Controller::Command::makeObjectActionName(wxT("Duplicate"), newEntities, newBrushes));
            submit(addObjectsCommand);
            submit(changeEditStateCommand);
            submit(moveObjectsCommand);
            CommandProcessor::EndGroup(commandProcessor);
        }

        void EditorView::OnEditToggleTextureLock(wxCommandEvent& event) {
            mapDocument().setTextureLock(!mapDocument().textureLock());
        }

        void EditorView::OnUpdateMenuItem(wxUpdateUIEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            wxTextCtrl* textCtrl = wxDynamicCast(GetFrame()->FindFocus(), wxTextCtrl);
            switch (event.GetId()) {
                case wxID_SAVE:
                    event.Enable(mapDocument().IsModified());
                    break;
                case wxID_UNDO:
                    GetDocumentManager()->OnUpdateUndo(event);
                    if (textCtrl != NULL)
                        event.Enable(false);
                    break;
                case wxID_REDO:
                    GetDocumentManager()->OnUpdateRedo(event);
                    if (textCtrl != NULL)
                        event.Enable(false);
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
                case wxID_COPY:
                    if (textCtrl != NULL)
                        event.Enable(textCtrl->CanCopy());
                    else
                        event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone);
                    break;
                case wxID_CUT:
                case wxID_DELETE:
                    if (textCtrl != NULL)
                        event.Enable(textCtrl->CanCut());
                    else
                        event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone &&
                                     editStateManager.selectionMode() != Model::EditStateManager::SMFaces);
                    break;
                case wxID_PASTE:
                    if (textCtrl != NULL)
                        event.Enable(textCtrl->CanPaste());
                    else
                        event.Enable(canPaste());
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
                case CommandIds::Menu::EditToggleClipTool:
                    event.Enable(inputController().clipToolActive() || editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);
                    event.Check(inputController().clipToolActive());
                    break;
                case CommandIds::Menu::EditToggleClipSide:
                    event.Enable(inputController().clipToolActive());
                    break;
                case CommandIds::Menu::EditPerformClip:
                    event.Enable(inputController().canPerformClip());
                    break;
                case CommandIds::Menu::EditToggleVertexTool:
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);
                    event.Check(inputController().moveVerticesToolActive());
                    break;
                case CommandIds::Menu::EditToggleEdgeTool:
                case CommandIds::Menu::EditToggleFaceTool:
                    event.Enable(false);
                    break;
                case CommandIds::Menu::EditActions:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditMoveTexturesUp:
                case CommandIds::Menu::EditMoveTexturesRight:
                case CommandIds::Menu::EditMoveTexturesDown:
                case CommandIds::Menu::EditMoveTexturesLeft:
                case CommandIds::Menu::EditRotateTexturesCW:
                case CommandIds::Menu::EditRotateTexturesCCW:
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMFaces);
                    break;
                case CommandIds::Menu::EditMoveObjectsForward:
                case CommandIds::Menu::EditMoveObjectsRight:
                case CommandIds::Menu::EditMoveObjectsBackward:
                case CommandIds::Menu::EditMoveObjectsLeft:
                case CommandIds::Menu::EditMoveObjectsUp:
                case CommandIds::Menu::EditMoveObjectsDown:
                case CommandIds::Menu::EditRollObjectsCW:
                case CommandIds::Menu::EditRollObjectsCCW:
                case CommandIds::Menu::EditPitchObjectsCW:
                case CommandIds::Menu::EditPitchObjectsCCW:
                case CommandIds::Menu::EditYawObjectsCW:
                case CommandIds::Menu::EditYawObjectsCCW:
                case CommandIds::Menu::EditFlipObjectsHorizontally:
                case CommandIds::Menu::EditFlipObjectsVertically:
                case CommandIds::Menu::EditDuplicateObjects:
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMEntities || editStateManager.selectionMode() == Model::EditStateManager::SMBrushes || editStateManager.selectionMode() == Model::EditStateManager::SMEntitiesAndBrushes);
                    break;
                case CommandIds::Menu::EditToggleTextureLock:
                    event.Check(mapDocument().textureLock());
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditCreatePointEntity:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditCreateBrushEntity:
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);
                    break;
            }
        }
    }
}