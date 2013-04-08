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
#include "Controller/ChangeEditStateCommand.h"
#include "Controller/Command.h"
#include "Controller/ControllerUtils.h"
#include "Controller/EntityPropertyCommand.h"
#include "Controller/FlipObjectsCommand.h"
#include "Controller/InputController.h"
#include "Controller/MoveObjectsCommand.h"
#include "Controller/MoveTexturesCommand.h"
#include "Controller/MoveVerticesTool.h"
#include "Controller/ObjectsCommand.h"
#include "Controller/RebuildBrushGeometryCommand.h"
#include "Controller/RemoveObjectsCommand.h"
#include "Controller/RotateObjects90Command.h"
#include "Controller/RotateTexturesCommand.h"
#include "Controller/SetFaceAttributesCommand.h"
#include "Controller/SnapVerticesCommand.h"
#include "IO/ByteBuffer.h"
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
#include "Model/PointFile.h"
#include "Model/TextureManager.h"
#include "Renderer/Camera.h"
#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/SharedResources.h"
#include "Renderer/TextureRendererManager.h"
#include "Utility/CommandProcessor.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/List.h"
#include "Utility/Preferences.h"
#include "View/AbstractApp.h"
#include "View/CameraAnimation.h"
#include "View/CommandIds.h"
#include "View/EditorFrame.h"
#include "View/EntityInspector.h"
#include "View/FaceInspector.h"
#include "View/FlashSelectionAnimation.h"
#include "View/Inspector.h"
#include "View/MapGLCanvas.h"
#include "View/MapPropertiesDialog.h"
#include "View/ViewOptions.h"

#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/tokenzr.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EditorView, wxView)
        EVT_CAMERA_MOVE(EditorView::OnCameraMove)
        EVT_CAMERA_LOOK(EditorView::OnCameraLook)
        EVT_CAMERA_ORBIT(EditorView::OnCameraOrbit)
        EVT_CAMERA_SET(EditorView::OnCameraSet)

        EVT_MENU(wxID_NEW, EditorView::OnFileNew)
        EVT_MENU(wxID_OPEN, EditorView::OnFileOpen)
        EVT_MENU(wxID_SAVE, EditorView::OnFileSave)
        EVT_MENU(wxID_SAVEAS, EditorView::OnFileSaveAs)
        EVT_MENU(CommandIds::Menu::FileLoadPointFile, EditorView::OnFileLoadPointFile)
        EVT_MENU(CommandIds::Menu::FileUnloadPointFile, EditorView::OnFileUnloadPointFile)
        EVT_MENU(wxID_CLOSE, EditorView::OnFileClose)

        EVT_MENU(wxID_UNDO, EditorView::OnUndo)
        EVT_MENU(wxID_REDO, EditorView::OnRedo)

        EVT_MENU(wxID_CUT, EditorView::OnEditCut)
        EVT_MENU(wxID_COPY, EditorView::OnEditCopy)
        EVT_MENU(wxID_PASTE, EditorView::OnEditPaste)
        EVT_MENU(CommandIds::Menu::EditPasteAtOriginalPosition, EditorView::OnEditPasteAtOriginalPosition)
        EVT_MENU(wxID_DELETE, EditorView::OnEditDelete)

        EVT_MENU(CommandIds::Menu::EditSelectAll, EditorView::OnEditSelectAll)
        EVT_MENU(CommandIds::Menu::EditSelectSiblings, EditorView::OnEditSelectSiblings)
        EVT_MENU(CommandIds::Menu::EditSelectTouching, EditorView::OnEditSelectTouching)
        EVT_MENU(CommandIds::Menu::EditSelectByFilePosition, EditorView::OnEditSelectByFilePosition)
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
        EVT_MENU(CommandIds::Menu::EditToggleRotateObjectsTool, EditorView::OnEditToggleRotateObjectsTool)

        EVT_MENU(CommandIds::Menu::EditMoveTexturesUp, EditorView::OnEditMoveTexturesUp)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesRight, EditorView::OnEditMoveTexturesRight)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesDown, EditorView::OnEditMoveTexturesDown)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesLeft, EditorView::OnEditMoveTexturesLeft)
        EVT_MENU(CommandIds::Menu::EditRotateTexturesCW, EditorView::OnEditRotateTexturesCW)
        EVT_MENU(CommandIds::Menu::EditRotateTexturesCCW, EditorView::OnEditRotateTexturesCCW)

        EVT_MENU(CommandIds::Menu::EditMoveTexturesUpFine, EditorView::OnEditMoveTexturesUpFine)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesRightFine, EditorView::OnEditMoveTexturesRightFine)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesDownFine, EditorView::OnEditMoveTexturesDownFine)
        EVT_MENU(CommandIds::Menu::EditMoveTexturesLeftFine, EditorView::OnEditMoveTexturesLeftFine)
        EVT_MENU(CommandIds::Menu::EditRotateTexturesCWFine, EditorView::OnEditRotateTexturesCWFine)
        EVT_MENU(CommandIds::Menu::EditRotateTexturesCCWFine, EditorView::OnEditRotateTexturesCCWFine)

        EVT_MENU(CommandIds::Menu::EditMoveObjectsForward, EditorView::OnEditMoveObjectsForward)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsRight, EditorView::OnEditMoveObjectsRight)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsBackward, EditorView::OnEditMoveObjectsBackward)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsLeft, EditorView::OnEditMoveObjectsLeft)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsUp, EditorView::OnEditMoveObjectsUp)
        EVT_MENU(CommandIds::Menu::EditMoveObjectsDown, EditorView::OnEditMoveObjectsDown)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjectsForward, EditorView::OnEditDuplicateObjectsForward)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjectsRight, EditorView::OnEditDuplicateObjectsRight)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjectsBackward, EditorView::OnEditDuplicateObjectsBackward)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjectsLeft, EditorView::OnEditDuplicateObjectsLeft)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjectsUp, EditorView::OnEditDuplicateObjectsUp)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjectsDown, EditorView::OnEditDuplicateObjectsDown)
        EVT_MENU(CommandIds::Menu::EditRollObjectsCW, EditorView::OnEditRollObjectsCW)
        EVT_MENU(CommandIds::Menu::EditRollObjectsCCW, EditorView::OnEditRollObjectsCCW)
        EVT_MENU(CommandIds::Menu::EditPitchObjectsCW, EditorView::OnEditPitchObjectsCW)
        EVT_MENU(CommandIds::Menu::EditPitchObjectsCCW, EditorView::OnEditPitchObjectsCCW)
        EVT_MENU(CommandIds::Menu::EditYawObjectsCW, EditorView::OnEditYawObjectsCW)
        EVT_MENU(CommandIds::Menu::EditYawObjectsCCW, EditorView::OnEditYawObjectsCCW)
        EVT_MENU(CommandIds::Menu::EditFlipObjectsHorizontally, EditorView::OnEditFlipObjectsH)
        EVT_MENU(CommandIds::Menu::EditFlipObjectsVertically, EditorView::OnEditFlipObjectsV)
        EVT_MENU(CommandIds::Menu::EditDuplicateObjects, EditorView::OnEditDuplicateObjects)
        EVT_MENU(CommandIds::Menu::EditSnapVertices, EditorView::OnEditSnapVertices)
        EVT_MENU(CommandIds::Menu::EditCorrectVertices, EditorView::OnEditCorrectVertices)

        EVT_MENU(CommandIds::Menu::EditMoveVerticesForward, EditorView::OnEditMoveVerticesForward)
        EVT_MENU(CommandIds::Menu::EditMoveVerticesBackward, EditorView::OnEditMoveVerticesBackward)
        EVT_MENU(CommandIds::Menu::EditMoveVerticesLeft, EditorView::OnEditMoveVerticesLeft)
        EVT_MENU(CommandIds::Menu::EditMoveVerticesRight, EditorView::OnEditMoveVerticesRight)
        EVT_MENU(CommandIds::Menu::EditMoveVerticesUp, EditorView::OnEditMoveVerticesUp)
        EVT_MENU(CommandIds::Menu::EditMoveVerticesDown, EditorView::OnEditMoveVerticesDown)

        EVT_MENU(CommandIds::Menu::EditToggleTextureLock, EditorView::OnEditToggleTextureLock)
        EVT_MENU(CommandIds::Menu::EditNavigateUp, EditorView::OnEditNavigateUp)
        EVT_MENU(CommandIds::Menu::EditShowMapProperties, EditorView::OnEditShowMapProperties)

        EVT_MENU(CommandIds::Menu::ViewToggleShowGrid, EditorView::OnViewToggleShowGrid)
        EVT_MENU(CommandIds::Menu::ViewToggleSnapToGrid, EditorView::OnViewToggleSnapToGrid)
        EVT_MENU(CommandIds::Menu::ViewIncGridSize, EditorView::OnViewIncGridSize)
        EVT_MENU(CommandIds::Menu::ViewDecGridSize, EditorView::OnViewDecGridSize)
        EVT_MENU_RANGE(CommandIds::Menu::ViewSetGridSize1, CommandIds::Menu::ViewSetGridSize256, EditorView::OnViewSetGridSize)

        EVT_MENU(CommandIds::Menu::ViewMoveCameraForward, EditorView::OnViewMoveCameraForward)
        EVT_MENU(CommandIds::Menu::ViewMoveCameraBackward, EditorView::OnViewMoveCameraBackward)
        EVT_MENU(CommandIds::Menu::ViewMoveCameraLeft, EditorView::OnViewMoveCameraLeft)
        EVT_MENU(CommandIds::Menu::ViewMoveCameraRight, EditorView::OnViewMoveCameraRight)
        EVT_MENU(CommandIds::Menu::ViewMoveCameraUp, EditorView::OnViewMoveCameraUp)
        EVT_MENU(CommandIds::Menu::ViewMoveCameraDown, EditorView::OnViewMoveCameraDown)
        EVT_MENU(CommandIds::Menu::ViewCenterCameraOnSelection, EditorView::OnViewCenterCameraOnSelection)
        EVT_MENU(CommandIds::Menu::ViewMoveCameraToNextPoint, EditorView::OnViewMoveCameraToNextPoint)
        EVT_MENU(CommandIds::Menu::ViewMoveCameraToPreviousPoint, EditorView::OnViewMoveCameraToPreviousPoint)

        EVT_MENU(CommandIds::Menu::ViewSwitchToEntityTab, EditorView::OnViewSwitchToEntityInspector)
        EVT_MENU(CommandIds::Menu::ViewSwitchToFaceTab, EditorView::OnViewSwitchToFaceInspector)
        EVT_MENU(CommandIds::Menu::ViewSwitchToViewTab, EditorView::OnViewSwitchToViewInspector)

        EVT_UPDATE_UI(wxID_SAVE, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_UNDO, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_REDO, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_CUT, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_COPY, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_PASTE, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI(wxID_DELETE, EditorView::OnUpdateMenuItem)
        EVT_UPDATE_UI_RANGE(CommandIds::Menu::Lowest, CommandIds::Menu::Highest, EditorView::OnUpdateMenuItem)

        EVT_MENU(CommandIds::CreateEntityPopupMenu::ReparentBrushes, EditorView::OnPopupReparentBrushes)
        EVT_UPDATE_UI(CommandIds::CreateEntityPopupMenu::ReparentBrushes, EditorView::OnPopupUpdateReparentBrushesMenuItem)
        EVT_MENU(CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld, EditorView::OnPopupMoveBrushesToWorld)
        EVT_UPDATE_UI(CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld, EditorView::OnPopupUpdateMoveBrushesToWorldMenuItem)
        EVT_MENU_RANGE(CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem, EditorView::OnPopupCreatePointEntity)
        EVT_UPDATE_UI_RANGE(CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem, EditorView::OnPopupUpdatePointMenuItem)

        EVT_MENU_RANGE(CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem, EditorView::OnPopupCreateBrushEntity)
        EVT_UPDATE_UI_RANGE(CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem, EditorView::OnPopupUpdateBrushMenuItem)

        END_EVENT_TABLE()

        IMPLEMENT_DYNAMIC_CLASS(EditorView, wxView)

        void EditorView::submit(wxCommand* command, bool store) {
            mapDocument().GetCommandProcessor()->Submit(command, store);
        }

        void EditorView::pasteObjects(const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& delta) {
            assert(entities.empty() != brushes.empty());
            
            Model::EntityList selectEntities;
            Model::BrushList selectBrushes = brushes;
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                const Model::BrushList& entityBrushes = entity.brushes();
                if (!entityBrushes.empty())
                    selectBrushes.insert(selectBrushes.begin(), entityBrushes.begin(), entityBrushes.end());
                else
                    selectEntities.push_back(&entity);
            }
            
            Controller::AddObjectsCommand* addObjectsCommand = Controller::AddObjectsCommand::addObjects(mapDocument(), entities, brushes);
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::replace(mapDocument(), selectEntities, selectBrushes);
            Controller::MoveObjectsCommand* moveObjectsCommand = delta.null() ? NULL : Controller::MoveObjectsCommand::moveObjects(mapDocument(), selectEntities, selectBrushes, delta, mapDocument().textureLock());
            
            wxCommandProcessor* commandProcessor = mapDocument().GetCommandProcessor();
            CommandProcessor::BeginGroup(commandProcessor, Controller::Command::makeObjectActionName(wxT("Paste"), selectEntities, selectBrushes));
            submit(addObjectsCommand);
            submit(changeEditStateCommand);
            if (moveObjectsCommand != NULL)
                submit(moveObjectsCommand);
            CommandProcessor::EndGroup(commandProcessor);
            
            StringStream message;
            message << "Pasted "    << selectEntities.size()    << (selectEntities.size() == 1 ? " entity " : " entities");
            message << " and "      << selectBrushes.size()     << (selectBrushes.size() == 1 ? " brush " : " brushes");
            mapDocument().console().info(message.str());
        }
        
        Vec3f EditorView::moveDelta(Direction direction, bool snapToGrid) {
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
            return moveDirection * dist;
        }

        void EditorView::moveTextures(Direction direction, bool snapToGrid) {
            float distance = snapToGrid ? static_cast<float>(mapDocument().grid().actualSize()) : 1.0f;

            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::FaceList& faces = editStateManager.selectedFaces();
            wxString actionName = faces.size() == 1 ? wxT("Move Texture") : wxT("Move Textures");

            Controller::MoveTexturesCommand* command = Controller::MoveTexturesCommand::moveTextures(mapDocument(), actionName, camera().up(), camera().right(), direction, distance);
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
            Vec3f delta = moveDelta(direction, snapToGrid);

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
                default:
                    absoluteAxis = Axis::AZ;
                    break;
            }

            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            assert(entities.size() + brushes.size() > 0);

            Vec3f center = mapDocument().grid().referencePoint(editStateManager.bounds());
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
            assert(entities.size() + brushes.size() > 0);

            Vec3f center = mapDocument().grid().referencePoint(editStateManager.bounds());
            Controller::FlipObjectsCommand* command = Controller::FlipObjectsCommand::flip(mapDocument(), entities, brushes, axis, center, mapDocument().textureLock());
            submit(command);
        }

        void EditorView::moveVertices(Direction direction, bool snapToGrid) {
            assert(inputController().moveVerticesToolActive());

            if (inputController().moveVerticesTool().hasSelection()) {
                Vec3f delta = moveDelta(direction, snapToGrid);
                inputController().moveVerticesTool().moveVertices(delta);
            }
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

        EditorView::EditorView() :
        wxView(),
        m_animationManager(new AnimationManager()),
        m_camera(NULL),
        m_renderer(NULL),
        m_filter(NULL),
        m_viewOptions(NULL),
        m_createEntityPopupMenu(NULL),
        m_createPointEntityMenu(NULL) {}

        EditorView::~EditorView() {
            m_animationManager->Delete();
            m_animationManager = NULL;
        }
        
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

        AnimationManager& EditorView::animationManager() const {
            return *m_animationManager;
        }

        wxMenu* EditorView::createEntityPopupMenu() {
            if (m_createEntityPopupMenu == NULL) {
                Model::EntityDefinitionManager& definitionManager = mapDocument().definitionManager();
                Model::EntityDefinitionManager::EntityDefinitionGroups::const_iterator groupIt, groupEnd;
                Model::EntityDefinitionList::const_iterator defIt, defEnd;

                int id = 0;
                wxMenu* pointMenu = new wxMenu();
                pointMenu->SetEventHandler(this);

                Model::EntityDefinitionManager::EntityDefinitionGroups groups = definitionManager.groups(Model::EntityDefinition::PointEntity);
                for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                    const String& groupName = groupIt->first;
                    wxMenu* groupMenu = new wxMenu();
                    groupMenu->SetEventHandler(this);
                    
                    const Model::EntityDefinitionList& definitions = groupIt->second;
                    for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                        Model::EntityDefinition& definition = **defIt;
                        groupMenu->Append(CommandIds::CreateEntityPopupMenu::LowestPointEntityItem + id++, definition.shortName());
                    }
                    
                    pointMenu->AppendSubMenu(groupMenu, groupName);
                }
                

                m_createPointEntityMenu = pointMenu;

                id = 0;
                wxMenu* brushMenu = new wxMenu();
                brushMenu->SetEventHandler(this);
                
                groups = definitionManager.groups(Model::EntityDefinition::BrushEntity);
                for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                    const String& groupName = groupIt->first;
                    wxMenu* groupMenu = new wxMenu();
                    groupMenu->SetEventHandler(this);
                    
                    const Model::EntityDefinitionList& definitions = groupIt->second;
                    for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                        Model::EntityDefinition& definition = **defIt;
                        if (definition.name() != Model::Entity::WorldspawnClassname)
                            groupMenu->Append(CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem + id, definition.shortName());
                        ++id;
                    }
                    
                    brushMenu->AppendSubMenu(groupMenu, groupName);
                }

                m_createEntityPopupMenu = new wxMenu();
                m_createEntityPopupMenu->SetEventHandler(this);

                m_createEntityPopupMenu->Append(CommandIds::CreateEntityPopupMenu::ReparentBrushes, wxT("Move Brushes to..."));
                m_createEntityPopupMenu->Append(CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld, wxT("Move Brushes to World"));
                m_createEntityPopupMenu->AppendSeparator();
                m_createEntityPopupMenu->AppendSubMenu(pointMenu, wxT("Create Point Entity"));
                m_createEntityPopupMenu->AppendSubMenu(brushMenu, wxT("Create Brush Entity"));
            }

            return m_createEntityPopupMenu;
        }

        void EditorView::setModified(bool modified) {
            wxFrame* frame = wxDynamicCast(GetFrame(), wxFrame);
#if defined __APPLE__
            frame->OSXSetModified(modified);
#else
            frame->SetTitle(mapDocument().GetTitle() + (GetDocument()->IsModified() ? wxT(" *") : wxT("")));
#endif
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
                    case Controller::Command::LoadMap: {
                        m_camera->moveTo(Vec3f(160.0f, 160.0f, 48.0f));
                        m_camera->setDirection(Vec3f(-1.0f, -1.0f, 0.0f).normalized(), Vec3f::PosZ);

                        m_renderer->loadMap();

                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateNavBar();
                        frame->updateMenuBar();
                        break;
                    }
                    case Controller::Command::ClearMap: {
                        m_renderer->clearMap();

                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateNavBar();
                        frame->updateMenuBar();
                        break;
                    }
                    case Controller::Command::ChangeEditState: {
                        Controller::ChangeEditStateCommand* changeEditStateCommand = static_cast<Controller::ChangeEditStateCommand*>(command);
                        m_renderer->changeEditState(changeEditStateCommand->changeSet());

                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateNavBar();
                        frame->updateMenuBar();

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
                        mapDocument().invalidateSearchPaths();
                        m_renderer->invalidateEntityModelRendererCache();
                        break;
                    case Controller::Command::InvalidateInstancedRenderers:
                        inputController().moveVerticesTool().resetInstancedRenderers();
                        break;
                    case Controller::Command::SetFaceAttributes:
                    case Controller::Command::MoveTextures:
                    case Controller::Command::RotateTextures: {
                        m_renderer->invalidateSelectedBrushes();
                        break;
                    }
                    case Controller::Command::RemoveTextureCollection:
                    case Controller::Command::MoveTextureCollectionUp:
                    case Controller::Command::MoveTextureCollectionDown:
                        mapDocument().sharedResources().textureRendererManager().invalidate();
                    case Controller::Command::AddTextureCollection:
                        m_renderer->invalidateAll();
                        break;
                    case Controller::Command::SetEntityPropertyKey:
                    case Controller::Command::SetEntityPropertyValue:
                    case Controller::Command::RemoveEntityProperty: {
                        m_renderer->invalidateEntities();
                        m_renderer->invalidateSelectedEntityModelRendererCache();
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
                    case Controller::Command::RebuildBrushGeometry:
                    case Controller::Command::MoveVertices: {
                        if (command->type() == Controller::Command::RebuildBrushGeometry) {
                            Controller::RebuildBrushGeometryCommand* rebuildCommand = static_cast<Controller::RebuildBrushGeometryCommand*>(command);
                            if (rebuildCommand->state() == Controller::Command::Undoing && rebuildCommand->activateMoveVerticesTool()) {
                                assert(!inputController().moveVerticesToolActive());
                                inputController().toggleMoveVerticesTool(rebuildCommand->precedingChangeCount());
                            }
                        } else {
                            assert(inputController().moveVerticesToolActive());
                            if (command->state() == Controller::Command::Doing)
                                inputController().moveVerticesTool().incChangeCount();
                            else
                                inputController().moveVerticesTool().decChangeCount();
                        }
                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateNavBar();
                    }
                    case Controller::Command::SnapVertices:
                    case Controller::Command::MoveObjects:
                    case Controller::Command::RotateObjects:
                    case Controller::Command::FlipObjects:
                    case Controller::Command::ResizeBrushes: {
                        m_renderer->invalidateSelectedBrushes();
                        m_renderer->invalidateSelectedEntities();
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
                    case Controller::Command::ReparentBrushes: {
                        m_renderer->invalidateSelectedBrushes();
                        m_renderer->invalidateEntities();
                        m_renderer->invalidateSelectedEntities();
                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateNavBar();
                        break;
                    }
                    case Controller::Command::UpdateFigures:
                        break;
                    case Controller::Command::SetMod:
                    case Controller::Command::SetEntityDefinitionFile: {
                        mapDocument().sharedResources().modelRendererManager().clearMismatches();
                        m_renderer->invalidateEntityModelRendererCache();
                        m_renderer->invalidateAll();
                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateNavBar();
                        break;
                    }
                    case Controller::Command::ClipToolChange:
                    case Controller::Command::MoveVerticesToolChange: {
                        EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                        frame->updateNavBar();
                        break;
                    }
                    default:
                        break;
                }
                inputController().update(*command);
                inspector().update(*command);
            }

            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            if (frame != NULL)
                frame->mapCanvas().Refresh();
        }

        void EditorView::OnChangeFilename() {
            EditorFrame* frame = wxDynamicCast(GetFrame(), EditorFrame);
            if (frame != NULL) {
#if defined __APPLE__
                frame->SetTitle(mapDocument().GetTitle());
#else
                frame->SetTitle(mapDocument().GetTitle() + (GetDocument()->IsModified() ? wxT(" *") : wxT("")));
#endif
                frame->SetRepresentedFilename(mapDocument().GetFilename());
            }
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
            inputController().cameraChanged();
            inspector().cameraChanged(*m_camera);
            OnUpdate(this);
        }

        void EditorView::OnCameraLook(Controller::CameraLookEvent& event) {
            m_camera->rotate(event.hAngle(), event.vAngle());
            inputController().cameraChanged();
            inspector().cameraChanged(*m_camera);
            OnUpdate(this);
        }

        void EditorView::OnCameraOrbit(Controller::CameraOrbitEvent& event) {
            m_camera->orbit(event.center(), event.hAngle(), event.vAngle());
            inputController().cameraChanged();
            inspector().cameraChanged(*m_camera);
            OnUpdate(this);
        }

        void EditorView::OnCameraSet(Controller::CameraSetEvent& event) {
            m_camera->moveTo(event.position());
            m_camera->setDirection(event.direction(), event.up());
            inputController().cameraChanged();
            inspector().cameraChanged(*m_camera);
            OnUpdate(this);
        }

        void EditorView::OnFileNew(wxCommandEvent& event) {
            GetDocumentManager()->OnFileNew(event);
        }

        void EditorView::OnFileOpen(wxCommandEvent& event) {
            GetDocumentManager()->OnFileOpen(event);
        }

        void EditorView::OnFileSave(wxCommandEvent& event) {
            GetDocument()->Save();
        }

        void EditorView::OnFileSaveAs(wxCommandEvent& event) {
            GetDocument()->SaveAs();
        }

        void EditorView::OnFileLoadPointFile(wxCommandEvent& event) {
            mapDocument().loadPointFile();
            renderer().removePointTrace();
            if (mapDocument().pointFileLoaded()) {
                Model::PointFile& pointFile = mapDocument().pointFile();
                renderer().setPointTrace(pointFile.points());

                const Vec3f position = pointFile.currentPoint() + Vec3f(0.0f, 0.0f, 16.0f);
                const Vec3f& direction = pointFile.direction();

                Controller::CameraSetEvent cameraEvent;
                cameraEvent.set(position, direction, Vec3f::PosZ);
                cameraEvent.SetEventObject(this);
                ProcessEvent(cameraEvent);
            }
            OnUpdate(this);
        }

        void EditorView::OnFileUnloadPointFile(wxCommandEvent& event) {
            mapDocument().unloadPointFile();
            renderer().removePointTrace();
            OnUpdate(this);
        }

        void EditorView::OnFileClose(wxCommandEvent& event) {
            GetDocument()->GetDocumentManager()->CloseDocument(GetDocument());
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
                    if (editStateManager.selectionMode() == Model::EditStateManager::SMFaces) {
                        mapWriter.writeFacesToStream(editStateManager.selectedFaces(), clipboardData);
                        wxTheClipboard->SetData(new wxTextDataObject(clipboardData.str()));
                    } else {
                        mapWriter.writeObjectsToStream(editStateManager.selectedEntities(), editStateManager.selectedBrushes(), clipboardData);
                        wxTheClipboard->SetData(new wxTextDataObject(clipboardData.str()));
                    }

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

                        wxTextDataObject textData;
                        String text;
                        
                        if (wxTheClipboard->GetData(textData))
                            text = textData.GetText();
                        
                        IO::MapParser mapParser(text, console());
                        if (mapParser.parseFaces(mapDocument().map().worldBounds(), mapDocument().map().forceIntegerFacePoints(), faces)) {
                            assert(!faces.empty());

                            Model::Face& face = *faces.back();
                            Model::TextureManager& textureManager = mapDocument().textureManager();
                            Model::Texture* texture = textureManager.texture(face.textureName());
                            face.setTexture(texture);

                            const Model::FaceList& selectedFaces = mapDocument().editStateManager().selectedFaces();
                            if (!selectedFaces.empty()) {
                                Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(mapDocument(), selectedFaces, wxT("Paste Faces"));
                                command->setTemplate(face);
                                submit(command);

                                if (faces.size() == 1)
                                    mapDocument().console().info("Pasted 1 face from clipboard", faces.size());
                                else
                                    mapDocument().console().info("Pasted last of %d faces from clipboard", faces.size());
                            } else {
                                mapDocument().console().warn("Could not paste faces because no faces are selected");
                            }
                        } else if (mapParser.parseEntities(mapDocument().map().worldBounds(), mapDocument().map().forceIntegerFacePoints(), entities) ||
                                   mapParser.parseBrushes(mapDocument().map().worldBounds(), mapDocument().map().forceIntegerFacePoints(), brushes)) {
                            assert(entities.empty() != brushes.empty());

                            const BBox objectsBounds = Model::MapObject::bounds(entities, brushes);
                            const Vec3f objectsPosition = mapDocument().grid().referencePoint(objectsBounds);

                            Vec3f delta;

                            wxMouseState mouseState = wxGetMouseState();
                            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                            const wxPoint clientCoords = frame->mapCanvas().ScreenToClient(mouseState.GetPosition());
                            if (frame->mapCanvas().HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                                Controller::InputState& inputState = inputController().inputState();
                                Model::PickResult& pickResult = inputState.pickResult();
                                Model::FaceHit* hit = static_cast<Model::FaceHit*>(pickResult.first(Model::HitType::FaceHit, true, filter()));
                                if (hit != NULL) {
                                    const Vec3f snappedHitPoint = mapDocument().grid().snap(hit->hitPoint());
                                    delta = mapDocument().grid().moveDeltaForBounds(hit->face(), objectsBounds, mapDocument().map().worldBounds(), inputState.pickRay(), snappedHitPoint);
                                } else {
                                    const Vec3f targetPosition = mapDocument().grid().snap(camera().defaultPoint(inputState.pickRay().direction));
                                    delta = targetPosition - objectsPosition;
                                }
                            } else {
                                const Vec3f targetPosition = mapDocument().grid().snap(camera().defaultPoint());
                                delta = targetPosition - objectsPosition;
                            }

                            pasteObjects(entities, brushes, delta);
                        } else {
                            mapDocument().console().warn("Unable to parse clipboard contents");
                        }
                    }
                    wxTheClipboard->Close();
                }
            }
        }

        void EditorView::OnEditPasteAtOriginalPosition(wxCommandEvent& event) {
            if (wxTextCtrl* textCtrl = wxDynamicCast(GetFrame()->FindFocus(), wxTextCtrl)) {
                textCtrl->Paste();
            } else {
                if (wxTheClipboard->Open()) {
                    if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                        Model::EntityList entities;
                        Model::BrushList brushes;
                        
                        wxTextDataObject textData;
                        String text;
                        
                        if (wxTheClipboard->GetData(textData))
                            text = textData.GetText();
                        
                        IO::MapParser mapParser(text, console());
                        if (mapParser.parseEntities(mapDocument().map().worldBounds(), mapDocument().map().forceIntegerFacePoints(), entities) ||
                            mapParser.parseBrushes(mapDocument().map().worldBounds(), mapDocument().map().forceIntegerFacePoints(), brushes)) {
                            assert(entities.empty() != brushes.empty());
                            
                            pasteObjects(entities, brushes, Vec3f::Null);
                        } else {
                            mapDocument().console().warn("Unable to parse clipboard contents");
                        }
                    }
                    wxTheClipboard->Close();
                }
            }
        }

        void EditorView::OnEditDelete(wxCommandEvent& event) {
            if (inputController().clipToolActive() && inputController().canDeleteClipPoint())
                inputController().deleteClipPoint();
            else
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
                    if (m_filter->entitySelectable(entity)) {
                        assert(entity.brushes().empty());
                        selectEntities.push_back(&entity);
                    } else {
                        const Model::BrushList& entityBrushes = entity.brushes();
                        for (unsigned int j = 0; j < entityBrushes.size(); j++) {
                            Model::Brush& brush = *entityBrushes[j];
                            if (m_filter->brushSelectable(brush))
                                selectBrushes.push_back(&brush);
                        }
                    }
                }

                if (!selectEntities.empty() || !selectBrushes.empty()) {
                    wxCommand* command = Controller::ChangeEditStateCommand::replace(mapDocument(), selectEntities, selectBrushes);
                    submit(command);
                }
            }
        }

        void EditorView::OnEditSelectSiblings(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            assert(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);

            const Model::BrushList& selectedBrushes = editStateManager.selectedBrushes();
            Model::BrushSet selectBrushesSet;

            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = selectedBrushes.begin(), brushEnd = selectedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush* brush = *brushIt;
                Model::Entity* entity = brush->entity();
                Model::BrushList entityBrushes = m_filter->selectableBrushes(entity->brushes());
                selectBrushesSet.insert(entityBrushes.begin(), entityBrushes.end());
            }

            Model::BrushList selectBrushes;
            selectBrushes.insert(selectBrushes.begin(), selectBrushesSet.begin(), selectBrushesSet.end());

            if (!selectBrushes.empty()) {
                wxCommand* command = Controller::ChangeEditStateCommand::replace(mapDocument(), selectBrushes);
                submit(command);
            }
        }

        void EditorView::OnEditSelectTouching(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            assert(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes &&
                   editStateManager.selectedBrushes().size() == 1);

            Model::Brush* selectionBrush = editStateManager.selectedBrushes().front();
            Model::EntityList selectEntities;
            Model::BrushList selectBrushes;

            const Model::EntityList& allEntities = mapDocument().map().entities();
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = allEntities.begin(), entityEnd = allEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                const Model::BrushList& entityBrushes = entity.brushes();
                if (!entityBrushes.empty()) {
                    Model::BrushList::const_iterator brushIt, brushEnd;
                    for (brushIt = entityBrushes.begin(), brushEnd = entityBrushes.end(); brushIt != brushEnd; ++brushIt) {
                        Model::Brush* brush = *brushIt;
                        if (brush != selectionBrush && selectionBrush->intersectsBrush(*brush) && m_filter->brushSelectable(*brush))
                            selectBrushes.push_back(brush);
                    }
                } else if (selectionBrush->intersectsEntity(entity) && m_filter->entitySelectable(entity)) {
                    selectEntities.push_back(&entity);
                }
            }

            Controller::ChangeEditStateCommand* select;
            if (!selectEntities.empty() || !selectBrushes.empty()) {
                select = Controller::ChangeEditStateCommand::replace(mapDocument(), selectEntities, selectBrushes);
            } else {
                select = Controller::ChangeEditStateCommand::deselectAll(mapDocument());
            }

            Controller::RemoveObjectsCommand* remove = Controller::RemoveObjectsCommand::removeBrush(mapDocument(), *selectionBrush);

            CommandProcessor::BeginGroup(mapDocument().GetCommandProcessor(), wxT("Select Touching"));
            submit(select);
            submit(remove);
            CommandProcessor::EndGroup(mapDocument().GetCommandProcessor());
        }

        void EditorView::OnEditSelectByFilePosition(wxCommandEvent& event) {
            wxString string = wxGetTextFromUser(wxT("Enter a comma- or space separated list of line numbers."), wxT("Select by Line Numbers"), wxT(""), GetFrame());
            if (string.empty())
                return;

            const Model::EntityList& entities = mapDocument().map().entities();
            Model::EntitySet selectEntities;
            Model::BrushSet selectBrushes;
            
            wxStringTokenizer tokenizer(string, ", ");
            while (tokenizer.HasMoreTokens()) {
                wxString token = tokenizer.NextToken();
                unsigned long position;
                if (token.ToULong(&position)) {
                    Model::Entity* selectEntity = NULL;
                    Model::Brush* selectBrush = NULL;
                    
                    Model::EntityList::const_iterator entityIt, entityEnd;
                    for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd && selectEntity == NULL; ++entityIt) {
                        Model::Entity& entity = **entityIt;
                        if (entity.occupiesFileLine(position)) {
                            if (entity.brushes().empty())
                                selectEntity = &entity;
                            
                            const Model::BrushList& brushes = entity.brushes();
                            Model::BrushList::const_iterator brushIt, brushEnd;
                            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd && selectBrush == NULL; ++brushIt) {
                                Model::Brush& brush = **brushIt;
                                if (brush.occupiesFileLine(position))
                                    selectBrush = &brush;
                            }
                        }
                    }
                    
                    if (selectBrush != NULL)
                        selectBrushes.insert(selectBrush);
                    else if (selectEntity != NULL)
                        selectEntities.insert(selectEntity);
                }
            }
            
            if (!selectEntities.empty() || !selectBrushes.empty()) {
                wxCommand* command = Controller::ChangeEditStateCommand::replace(mapDocument(), Utility::makeList(selectEntities), Utility::makeList(selectBrushes));
                submit(command);
                StringStream message;
                message << "Selected " << selectEntities.size() << " " << (selectEntities.size() == 1 ? "entity" : "entities") << " and " << selectBrushes.size() << " " << (selectBrushes.size() == 1 ? "brush" : "brushes");
                console().info(message.str());
            } else {
                console().info("No objects with the given line numbers found");
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

            assert(!hideEntities.empty() || !hideBrushes.empty());

            wxCommand* command = Controller::ChangeEditStateCommand::hide(mapDocument(), hideEntities, hideBrushes);
            submit(command);
        }

        void EditorView::OnEditHideUnselected(wxCommandEvent& event) {
            const Model::EntityList& entities = mapDocument().map().entities();
            Model::EntityList hideEntities;
            Model::BrushList hideBrushes;

            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity& entity = *entities[i];
                if (!entity.selected() && !entity.partiallySelected() && entity.hideable())
                    hideEntities.push_back(&entity);

                const Model::BrushList& entityBrushes = entity.brushes();
                for (unsigned int j = 0; j < entityBrushes.size(); j++) {
                    Model::Brush& brush = *entityBrushes[j];
                    if (!brush.selected() && brush.hideable())
                        hideBrushes.push_back(&brush);
                }
            }

            // might happen of all visible brushes are selected (not checking for this when enabling the menu item for performance reasons)
            if (hideEntities.empty() && hideBrushes.empty())
                return;

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
                if (!entity.selected() && !entity.partiallySelected() && entity.lockable()) {
                    lockEntities.push_back(&entity);

                    const Model::BrushList& entityBrushes = entity.brushes();
                    for (unsigned int j = 0; j < entityBrushes.size(); j++) {
                        Model::Brush& brush = *entityBrushes[j];
                        if (!brush.selected() && brush.lockable())
                            lockBrushes.push_back(&brush);
                    }
                }
            }

            // might happen of all visible brushes are selected (not checking for this when enabling the menu item for performance reasons)
            if (lockEntities.empty() && lockBrushes.empty())
                return;

            wxCommand* command = Controller::ChangeEditStateCommand::lock(mapDocument(), lockEntities, lockBrushes);
            submit(command);
        }

        void EditorView::OnEditUnlockAll(wxCommandEvent& event) {
            wxCommand* command = Controller::ChangeEditStateCommand::unlockAll(mapDocument());
            submit(command);
        }

        void EditorView::OnEditToggleClipTool(wxCommandEvent& event) {
            inputController().toggleClipTool();
            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            frame->updateMenuBar();
        }

        void EditorView::OnEditToggleClipSide(wxCommandEvent& event) {
            inputController().toggleClipSide();
        }

        void EditorView::OnEditPerformClip(wxCommandEvent& event) {
            inputController().performClip();
            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            frame->updateMenuBar();
        }

        void EditorView::OnEditToggleVertexTool(wxCommandEvent& event) {
            inputController().toggleMoveVerticesTool();
            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            frame->updateMenuBar();
        }

        void EditorView::OnEditToggleRotateObjectsTool(wxCommandEvent& event) {
            inputController().toggleRotateObjectsTool();
            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            frame->updateMenuBar();
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

        void EditorView::OnEditDuplicateObjectsForward(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            CommandProcessor::BeginGroup(mapDocument().GetCommandProcessor(), Controller::Command::makeObjectActionName(wxT("Duplicate & Move"), entities, brushes));
            Controller::duplicateObjects(mapDocument());
            moveObjects(DForward, true);
            CommandProcessor::EndGroup(mapDocument().GetCommandProcessor());
            
            EditorFrame* editorFrame = static_cast<EditorFrame*>(GetFrame());
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(*m_renderer, editorFrame->mapCanvas(), 150);
            m_animationManager->runAnimation(animation, true);
        }
        
        void EditorView::OnEditDuplicateObjectsRight(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            CommandProcessor::BeginGroup(mapDocument().GetCommandProcessor(), Controller::Command::makeObjectActionName(wxT("Duplicate & Move"), entities, brushes));
            Controller::duplicateObjects(mapDocument());
            moveObjects(DRight, true);
            CommandProcessor::EndGroup(mapDocument().GetCommandProcessor());
            
            EditorFrame* editorFrame = static_cast<EditorFrame*>(GetFrame());
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(*m_renderer, editorFrame->mapCanvas(), 150);
            m_animationManager->runAnimation(animation, true);
        }
        
        void EditorView::OnEditDuplicateObjectsBackward(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            CommandProcessor::BeginGroup(mapDocument().GetCommandProcessor(), Controller::Command::makeObjectActionName(wxT("Duplicate & Move"), entities, brushes));
            Controller::duplicateObjects(mapDocument());
            moveObjects(DBackward, true);
            CommandProcessor::EndGroup(mapDocument().GetCommandProcessor());
            
            EditorFrame* editorFrame = static_cast<EditorFrame*>(GetFrame());
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(*m_renderer, editorFrame->mapCanvas(), 150);
            m_animationManager->runAnimation(animation, true);
        }
        
        void EditorView::OnEditDuplicateObjectsLeft(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            CommandProcessor::BeginGroup(mapDocument().GetCommandProcessor(), Controller::Command::makeObjectActionName(wxT("Duplicate & Move"), entities, brushes));
            Controller::duplicateObjects(mapDocument());
            moveObjects(DLeft, true);
            CommandProcessor::EndGroup(mapDocument().GetCommandProcessor());
            
            EditorFrame* editorFrame = static_cast<EditorFrame*>(GetFrame());
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(*m_renderer, editorFrame->mapCanvas(), 150);
            m_animationManager->runAnimation(animation, true);
        }
        
        void EditorView::OnEditDuplicateObjectsUp(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            CommandProcessor::BeginGroup(mapDocument().GetCommandProcessor(), Controller::Command::makeObjectActionName(wxT("Duplicate & Move"), entities, brushes));
            Controller::duplicateObjects(mapDocument());
            moveObjects(DUp, true);
            CommandProcessor::EndGroup(mapDocument().GetCommandProcessor());
            
            EditorFrame* editorFrame = static_cast<EditorFrame*>(GetFrame());
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(*m_renderer, editorFrame->mapCanvas(), 150);
            m_animationManager->runAnimation(animation, true);
        }
        
        void EditorView::OnEditDuplicateObjectsDown(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            CommandProcessor::BeginGroup(mapDocument().GetCommandProcessor(), Controller::Command::makeObjectActionName(wxT("Duplicate & Move"), entities, brushes));
            Controller::duplicateObjects(mapDocument());
            moveObjects(DDown, true);
            CommandProcessor::EndGroup(mapDocument().GetCommandProcessor());
            
            EditorFrame* editorFrame = static_cast<EditorFrame*>(GetFrame());
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(*m_renderer, editorFrame->mapCanvas(), 150);
            m_animationManager->runAnimation(animation, true);
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

        void EditorView::OnEditMoveTexturesUpFine(wxCommandEvent& event) {
            moveTextures(DUp, false);
        }

        void EditorView::OnEditMoveTexturesRightFine(wxCommandEvent& event) {
            moveTextures(DRight, false);
        }

        void EditorView::OnEditMoveTexturesDownFine(wxCommandEvent& event) {
            moveTextures(DDown, false);
        }

        void EditorView::OnEditMoveTexturesLeftFine(wxCommandEvent& event) {
            moveTextures(DLeft, false);
        }

        void EditorView::OnEditRotateTexturesCWFine(wxCommandEvent& event) {
            rotateTextures(true, false);
        }

        void EditorView::OnEditRotateTexturesCCWFine(wxCommandEvent& event) {
            rotateTextures(false, false);
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
            Controller::duplicateObjects(mapDocument());
            
            EditorFrame* editorFrame = static_cast<EditorFrame*>(GetFrame());
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(*m_renderer, editorFrame->mapCanvas(), 150);
            m_animationManager->runAnimation(animation, true);
        }

        void EditorView::OnEditCorrectVertices(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            assert(entities.empty());
            assert(!brushes.empty());

            Controller::SnapVerticesCommand* command = Controller::SnapVerticesCommand::correct(mapDocument(), brushes);
            submit(command);
        }

        void EditorView::OnEditSnapVertices(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            assert(entities.empty());
            assert(!brushes.empty());

            Controller::SnapVerticesCommand* command = Controller::SnapVerticesCommand::snapTo1(mapDocument(), brushes);
            submit(command);
        }

        void EditorView::OnEditMoveVerticesForward(wxCommandEvent& event) {
            moveVertices(DForward, true);
        }

        void EditorView::OnEditMoveVerticesBackward(wxCommandEvent& event) {
            moveVertices(DBackward, true);
        }

        void EditorView::OnEditMoveVerticesLeft(wxCommandEvent& event) {
            moveVertices(DLeft, true);
        }

        void EditorView::OnEditMoveVerticesRight(wxCommandEvent& event) {
            moveVertices(DRight, true);
        }

        void EditorView::OnEditMoveVerticesUp(wxCommandEvent& event) {
            moveVertices(DUp, true);
        }

        void EditorView::OnEditMoveVerticesDown(wxCommandEvent& event) {
            moveVertices(DDown, true);
        }

        void EditorView::OnEditToggleTextureLock(wxCommandEvent& event) {
            mapDocument().setTextureLock(!mapDocument().textureLock());
        }

        void EditorView::OnEditNavigateUp(wxCommandEvent& event) {
            if (!inputController().navigateUp()) {
                wxCommand* command = Controller::ChangeEditStateCommand::deselectAll(mapDocument());
                submit(command);
            } else {
                EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
                frame->updateNavBar();
            }
        }

        void EditorView::OnEditShowMapProperties(wxCommandEvent& event) {
            MapPropertiesDialog dialog(GetFrame(), mapDocument());

            wxPoint pos = GetFrame()->GetPosition();
            pos.x += (GetFrame()->GetSize().x - dialog.GetSize().x) / 2;
            pos.y += (GetFrame()->GetSize().y - dialog.GetSize().y) / 2;
            dialog.SetPosition(pos);

            dialog.ShowModal();
        }

        void EditorView::OnViewToggleShowGrid(wxCommandEvent& event) {
            mapDocument().grid().toggleVisible();
            mapDocument().UpdateAllViews(NULL, new Controller::Command(Controller::Command::ChangeGrid));
        }

        void EditorView::OnViewToggleSnapToGrid(wxCommandEvent& event) {
            mapDocument().grid().toggleSnap();
            mapDocument().UpdateAllViews(NULL, new Controller::Command(Controller::Command::ChangeGrid));
        }

        void EditorView::OnViewSetGridSize(wxCommandEvent& event) {
            switch (event.GetId()) {
                case CommandIds::Menu::ViewSetGridSize1:
                    mapDocument().grid().setSize(0);
                    break;
                case CommandIds::Menu::ViewSetGridSize2:
                    mapDocument().grid().setSize(1);
                    break;
                case CommandIds::Menu::ViewSetGridSize4:
                    mapDocument().grid().setSize(2);
                    break;
                case CommandIds::Menu::ViewSetGridSize8:
                    mapDocument().grid().setSize(3);
                    break;
                case CommandIds::Menu::ViewSetGridSize16:
                    mapDocument().grid().setSize(4);
                    break;
                case CommandIds::Menu::ViewSetGridSize32:
                    mapDocument().grid().setSize(5);
                    break;
                case CommandIds::Menu::ViewSetGridSize64:
                    mapDocument().grid().setSize(6);
                    break;
                case CommandIds::Menu::ViewSetGridSize128:
                    mapDocument().grid().setSize(7);
                    break;
                case CommandIds::Menu::ViewSetGridSize256:
                    mapDocument().grid().setSize(8);
                    break;
            }
            mapDocument().UpdateAllViews(NULL, new Controller::Command(Controller::Command::ChangeGrid));
        }

        void EditorView::OnViewIncGridSize(wxCommandEvent& event) {
            mapDocument().grid().incSize();
            mapDocument().UpdateAllViews(NULL, new Controller::Command(Controller::Command::ChangeGrid));
        }

        void EditorView::OnViewDecGridSize(wxCommandEvent& event) {
            mapDocument().grid().decSize();
            mapDocument().UpdateAllViews(NULL, new Controller::Command(Controller::Command::ChangeGrid));
        }

        void EditorView::OnViewMoveCameraForward(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);

            Controller::CameraMoveEvent cameraEvent;
            cameraEvent.setForward(10.0f * speed);

            cameraEvent.SetEventObject(this);
            ProcessEvent(cameraEvent);
        }

        void EditorView::OnViewMoveCameraBackward(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);

            Controller::CameraMoveEvent cameraEvent;
            cameraEvent.setForward(10.0f * -speed);

            cameraEvent.SetEventObject(this);
            ProcessEvent(cameraEvent);
        }

        void EditorView::OnViewMoveCameraLeft(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);

            Controller::CameraMoveEvent cameraEvent;
            cameraEvent.setRight(10.0f * -speed);

            cameraEvent.SetEventObject(this);
            ProcessEvent(cameraEvent);
        }

        void EditorView::OnViewMoveCameraRight(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);

            Controller::CameraMoveEvent cameraEvent;
            cameraEvent.setRight(10.0f * speed);

            cameraEvent.SetEventObject(this);
            ProcessEvent(cameraEvent);
        }

        void EditorView::OnViewMoveCameraUp(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);

            Controller::CameraMoveEvent cameraEvent;
            cameraEvent.setUp(10.0f * speed);

            cameraEvent.SetEventObject(this);
            ProcessEvent(cameraEvent);
        }

        void EditorView::OnViewMoveCameraDown(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float speed = prefs.getFloat(Preferences::CameraPanSpeed);

            Controller::CameraMoveEvent cameraEvent;
            cameraEvent.setUp(10.0f * -speed);

            cameraEvent.SetEventObject(this);
            ProcessEvent(cameraEvent);
        }

        void EditorView::OnViewMoveCameraToNextPoint(wxCommandEvent& event) {
            assert(mapDocument().pointFileLoaded());

            Model::PointFile& pointFile = mapDocument().pointFile();
            assert(pointFile.hasNextPoint());

            const Vec3f position = pointFile.nextPoint() + Vec3f(0.0f, 0.0f, 16.0f);
            const Vec3f& direction = pointFile.direction();

            CameraAnimation* animation = new CameraAnimation(*this, position, direction, Vec3f::PosZ, 100);
            m_animationManager->runAnimation(animation, true);
        }

        void EditorView::OnViewMoveCameraToPreviousPoint(wxCommandEvent& event) {
            assert(mapDocument().pointFileLoaded());

            Model::PointFile& pointFile = mapDocument().pointFile();
            assert(pointFile.hasPreviousPoint());

            const Vec3f position = pointFile.previousPoint() + Vec3f(0.0f, 0.0f, 16.0f);
            const Vec3f& direction = pointFile.direction();

            CameraAnimation* animation = new CameraAnimation(*this, position, direction, Vec3f::PosZ, 100);
            m_animationManager->runAnimation(animation, true);
        }

        void EditorView::OnViewCenterCameraOnSelection(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            assert(editStateManager.hasSelectedObjects());

            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            Model::EntityList::const_iterator entityIt, entityEnd;
            Model::BrushList::const_iterator brushIt, brushEnd;

            float minDist = std::numeric_limits<float>::max();

            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity& entity = **entityIt;
                if (entity.brushes().empty()) {
                    for (unsigned int i = 0; i < 8; i++) {
                        const Vec3f vertex = entity.bounds().vertex(i);

                        const Vec3f toPosition = vertex - m_camera->position();
                        minDist = std::min(minDist, toPosition.dot(m_camera->direction()));
                    }
                }
            }

            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush& brush = **brushIt;
                const Model::VertexList& vertices = brush.vertices();
                Model::VertexList::const_iterator vertexIt, vertexEnd;
                for (vertexIt = vertices.begin(), vertexEnd = vertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                    const Model::Vertex& vertex = **vertexIt;

                    const Vec3f toPosition = vertex.position - m_camera->position();
                    minDist = std::min(minDist, toPosition.dot(m_camera->direction()));
                }
            }

            if (minDist < 0.0f) { // move camera so that all vertices are in front of it
                Controller::CameraMoveEvent moveBack;
                moveBack.setForward(minDist - 10.0f);
                moveBack.SetEventObject(this);
                ProcessEvent(moveBack);
            }

            // now look at the center
            const BBox bounds = Model::MapObject::bounds(entities, brushes);
            const Vec3f center = bounds.center();

            // act as if the camera were there already:
            const Vec3f oldPosition = camera().position();
            camera().moveTo(center);
            
            float offset = std::numeric_limits<float>::max();

            Plane frustumPlanes[4];
            m_camera->frustumPlanes(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);

            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity& entity = **entityIt;
                if (entity.brushes().empty()) {
                    for (unsigned int i = 0; i < 8; i++) {
                        const Vec3f vertex = entity.bounds().vertex(i);

                        for (size_t j = 0; j < 4; j++) {
                            const Plane& plane = frustumPlanes[j];
                            float dist = (vertex - m_camera->position()).dot(plane.normal) + 8.0f; // adds a bit of a border
                            offset = std::min(offset, dist / m_camera->direction().dot(plane.normal));
                        }
                    }
                }
            }

            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush& brush = **brushIt;
                const Model::VertexList& vertices = brush.vertices();
                Model::VertexList::const_iterator vertexIt, vertexEnd;
                for (vertexIt = vertices.begin(), vertexEnd = vertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                    const Model::Vertex& vertex = **vertexIt;

                    for (size_t i = 0; i < 4; i++) {
                        const Plane& plane = frustumPlanes[i];
                        float dist = (vertex.position - m_camera->position()).dot(plane.normal) + 8.0f; // adds a bit of a border
                        offset = std::min(offset, dist / m_camera->direction().dot(plane.normal));
                    }
                }
            }

            // jump back
            camera().moveTo(oldPosition);
            
            const Vec3f newPosition = center + camera().direction() * offset;
            CameraAnimation* animation = new CameraAnimation(*this, newPosition, camera().direction(), camera().up(), 150);
            m_animationManager->runAnimation(animation, true);
        }

        void EditorView::OnViewSwitchToEntityInspector(wxCommandEvent& event) {
            inspector().switchToInspector(0);
        }
        
        void EditorView::OnViewSwitchToFaceInspector(wxCommandEvent& event) {
            inspector().switchToInspector(1);
        }
        
        void EditorView::OnViewSwitchToViewInspector(wxCommandEvent& event) {
            inspector().switchToInspector(2);
        }

        void EditorView::OnUpdateMenuItem(wxUpdateUIEvent& event) {
            AbstractApp* app = static_cast<AbstractApp*>(wxTheApp);
            if (app->preferencesFrame() != NULL) {
                event.Enable(false);
                return;
            }
            
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            wxTextCtrl* textCtrl = wxDynamicCast(GetFrame()->FindFocus(), wxTextCtrl);
            switch (event.GetId()) {
                case wxID_SAVE:
                    event.Enable(mapDocument().IsModified());
                    break;
                case CommandIds::Menu::FileLoadPointFile:
                    event.Enable(mapDocument().pointFileExists());
                    if (mapDocument().pointFileLoaded())
                        event.SetText(wxT("Reload Point File"));
                    else
                        event.SetText(wxT("Load Point File"));
                    break;
                case CommandIds::Menu::FileUnloadPointFile:
                    event.Enable(mapDocument().pointFileLoaded());
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
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);
                    break;
                case CommandIds::Menu::EditSelectTouching:
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes &&
                                 editStateManager.selectedBrushes().size() == 1);
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
                    if (inputController().clipToolActive()) {
                        event.Enable(inputController().canDeleteClipPoint());
                    } else {
                        event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone &&
                                     editStateManager.selectionMode() != Model::EditStateManager::SMFaces);
                    }
                    break;
                case wxID_PASTE:
                    if (textCtrl != NULL) {
                        event.Enable(textCtrl->CanPaste());
                    } else {
                        bool canPaste = false;
                        if (wxTheClipboard->Open()) {
                            canPaste = wxTheClipboard->IsSupported(wxDF_TEXT);
                            wxTheClipboard->Close();
                        }
                        event.Enable(canPaste);
                    }
                    break;
                case CommandIds::Menu::EditPasteAtOriginalPosition:
                    if (textCtrl != NULL) {
                        event.Enable(false);
                    } else {
                        bool canPaste = false;
                        if (wxTheClipboard->Open()) {
                            canPaste = wxTheClipboard->IsSupported(wxDF_TEXT);
                            wxTheClipboard->Close();
                        }
                        event.Enable(canPaste);
                    }
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
                case CommandIds::Menu::EditToggleRotateObjectsTool:
                    event.Enable(editStateManager.hasSelectedObjects());
                    event.Check(inputController().rotateObjectsToolActive());
                    break;
                case CommandIds::Menu::EditActions:
                    event.Enable(false);
                    break;
                case CommandIds::Menu::EditMoveTexturesUp:
                case CommandIds::Menu::EditMoveTexturesRight:
                case CommandIds::Menu::EditMoveTexturesDown:
                case CommandIds::Menu::EditMoveTexturesLeft:
                case CommandIds::Menu::EditRotateTexturesCW:
                case CommandIds::Menu::EditRotateTexturesCCW:
                case CommandIds::Menu::EditMoveTexturesUpFine:
                case CommandIds::Menu::EditMoveTexturesRightFine:
                case CommandIds::Menu::EditMoveTexturesDownFine:
                case CommandIds::Menu::EditMoveTexturesLeftFine:
                case CommandIds::Menu::EditRotateTexturesCWFine:
                case CommandIds::Menu::EditRotateTexturesCCWFine:
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
                case CommandIds::Menu::EditSnapVertices:
                case CommandIds::Menu::EditCorrectVertices:
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);
                    break;
                case CommandIds::Menu::EditMoveVerticesForward:
                case CommandIds::Menu::EditMoveVerticesBackward:
                case CommandIds::Menu::EditMoveVerticesLeft:
                case CommandIds::Menu::EditMoveVerticesRight:
                case CommandIds::Menu::EditMoveVerticesUp:
                case CommandIds::Menu::EditMoveVerticesDown:
                    event.Enable(inputController().moveVerticesToolActive());
                    break;
                case CommandIds::Menu::EditToggleTextureLock:
                    event.Check(mapDocument().textureLock());
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditNavigateUp:
                    event.Enable(editStateManager.selectionMode() != Model::EditStateManager::SMNone);
                    break;
                case CommandIds::Menu::EditShowMapProperties:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditCreatePointEntity:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditCreateBrushEntity:
                    event.Enable(editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);
                    break;
                case CommandIds::Menu::ViewToggleShowGrid:
                    event.Enable(true);
                    event.Check(mapDocument().grid().visible());
                    break;
                case CommandIds::Menu::ViewToggleSnapToGrid:
                    event.Enable(true);
                    event.Check(mapDocument().grid().snap());
                    break;
                case CommandIds::Menu::ViewIncGridSize:
                    event.Enable(mapDocument().grid().size() < Utility::Grid::MaxSize);
                    break;
                case CommandIds::Menu::ViewDecGridSize:
                    event.Enable(mapDocument().grid().size() > 0);
                    break;
                case CommandIds::Menu::ViewSetGridSize1:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 0);
                    break;
                case CommandIds::Menu::ViewSetGridSize2:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 1);
                    break;
                case CommandIds::Menu::ViewSetGridSize4:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 2);
                    break;
                case CommandIds::Menu::ViewSetGridSize8:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 3);
                    break;
                case CommandIds::Menu::ViewSetGridSize16:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 4);
                    break;
                case CommandIds::Menu::ViewSetGridSize32:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 5);
                    break;
                case CommandIds::Menu::ViewSetGridSize64:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 6);
                    break;
                case CommandIds::Menu::ViewSetGridSize128:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 7);
                    break;
                case CommandIds::Menu::ViewSetGridSize256:
                    event.Enable(true);
                    event.Check(mapDocument().grid().size() == 8);
                    break;
                case CommandIds::Menu::ViewMoveCameraForward:
                case CommandIds::Menu::ViewMoveCameraBackward:
                case CommandIds::Menu::ViewMoveCameraLeft:
                case CommandIds::Menu::ViewMoveCameraRight:
                case CommandIds::Menu::ViewMoveCameraUp:
                case CommandIds::Menu::ViewMoveCameraDown:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewCenterCameraOnSelection:
                    event.Enable(editStateManager.hasSelectedObjects());
                    break;
                case CommandIds::Menu::ViewMoveCameraToNextPoint:
                    event.Enable(mapDocument().pointFileLoaded() && mapDocument().pointFile().hasNextPoint());
                    break;
                case CommandIds::Menu::ViewMoveCameraToPreviousPoint:
                    event.Enable(mapDocument().pointFileLoaded() && mapDocument().pointFile().hasPreviousPoint());
                    break;
                case CommandIds::Menu::ViewSwitchToEntityTab:
                case CommandIds::Menu::ViewSwitchToFaceTab:
                case CommandIds::Menu::ViewSwitchToViewTab:
                    event.Enable(true);
                    break;
            }
        }

        void EditorView::OnPopupReparentBrushes(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            inputController().reparentBrushes(editStateManager.selectedBrushes(), NULL);
        }

        void EditorView::OnPopupUpdateReparentBrushesMenuItem(wxUpdateUIEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            StringStream commandName;
            commandName << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to ";

            if (editStateManager.selectionMode() != Model::EditStateManager::SMBrushes) {
                commandName << "Entity";
                event.Enable(false);
            } else {
                const Model::Entity* newParent = inputController().canReparentBrushes(brushes, NULL);
                if (newParent != NULL) {
                    const Model::PropertyValue* classname = newParent->classname();
                    commandName << (classname == NULL ? "<missing classname>" : *classname);
                    event.Enable(true);
                } else {
                    commandName << "Entity";
                    event.Enable(false);
                }
            }
            event.SetText(commandName.str());
        }
        
        void EditorView::OnPopupMoveBrushesToWorld(wxCommandEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            inputController().reparentBrushes(editStateManager.selectedBrushes(), mapDocument().worldspawn(true));
        }

        void EditorView::OnPopupUpdateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            StringStream commandName;
            commandName << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to World";
            event.SetText(commandName.str());
            event.Enable(inputController().canReparentBrushes(brushes, mapDocument().worldspawn(true)) != NULL);
        }

        void EditorView::OnPopupCreatePointEntity(wxCommandEvent& event) {
            Model::EntityDefinitionManager& definitionManager = mapDocument().definitionManager();
            const Model::EntityDefinitionManager::EntityDefinitionGroups groups = definitionManager.groups(Model::EntityDefinition::PointEntity);

            size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestPointEntityItem);
            Model::EntityDefinitionManager::EntityDefinitionGroups::const_iterator groupIt, groupEnd;
            Model::EntityDefinitionList::const_iterator defIt, defEnd;

            size_t count = 0;
            for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                const Model::EntityDefinitionList& definitions = groupIt->second;
                if (index < count + definitions.size()) {
                    inputController().createEntity(*definitions[index - count]);
                    break;
                }
                
                count += definitions.size();
            }
        }

        void EditorView::OnPopupUpdatePointMenuItem(wxUpdateUIEvent& event) {
            event.Enable(!inputController().clipToolActive() && !inputController().moveVerticesToolActive());
        }

        void EditorView::OnPopupCreateBrushEntity(wxCommandEvent& event) {
            Model::EntityDefinitionManager& definitionManager = mapDocument().definitionManager();
            const Model::EntityDefinitionManager::EntityDefinitionGroups groups = definitionManager.groups(Model::EntityDefinition::BrushEntity);
            
            size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem);
            Model::EntityDefinitionManager::EntityDefinitionGroups::const_iterator groupIt, groupEnd;
            Model::EntityDefinitionList::const_iterator defIt, defEnd;
            
            size_t count = 0;
            for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                const Model::EntityDefinitionList& definitions = groupIt->second;
                if (index < count + definitions.size()) {
                    inputController().createEntity(*definitions[index - count]);
                    break;
                }
                
                count += definitions.size();
            }
        }

        void EditorView::OnPopupUpdateBrushMenuItem(wxUpdateUIEvent& event) {
            Model::EditStateManager& editStateManager = mapDocument().editStateManager();
            event.Enable(!inputController().clipToolActive() &&
                         !inputController().moveVerticesToolActive() &&
                         editStateManager.selectionMode() == Model::EditStateManager::SMBrushes);
        }
    }
}
