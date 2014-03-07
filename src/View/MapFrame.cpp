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

#include "MapFrame.h"

#include "TrenchBroomApp.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "IO/DiskFileSystem.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/MapObjectsIterator.h"
#include "Model/Object.h"
#include "Model/Selection.h"
#include "View/Autosaver.h"
#include "View/CommandIds.h"
#include "View/InfoPanel.h"
#include "View/FrameManager.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapView.h"
#include "View/Menu.h"
#include "View/NavBar.h"
#include "View/StatusBar.h"

#include <wx/clipbrd.h>
#include <wx/display.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>

namespace TrenchBroom {
    namespace View {
        const wxEventType MapFrame::EVT_REBUILD_MENUBAR = wxNewEventType();

        IMPLEMENT_DYNAMIC_CLASS(MapFrame, wxFrame)

        class OpenClipboard {
        public:
            OpenClipboard() {
                if (!wxTheClipboard->IsOpened())
                    wxTheClipboard->Open();
            }
            
            ~OpenClipboard() {
                if (wxTheClipboard->IsOpened())
                    wxTheClipboard->Close();
            }
        };
        
        MapFrame::MapFrame() :
        wxFrame(NULL, wxID_ANY, ""),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_infoPanel(NULL),
        m_navBar(NULL),
        m_mapView(NULL) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        wxFrame(NULL, wxID_ANY, ""),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_infoPanel(NULL),
        m_navBar(NULL),
        m_mapView(NULL) {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocumentSPtr document) {
            m_frameManager = frameManager;
            m_document = document;
            m_controller = ControllerSPtr(new ControllerFacade(m_document));
            m_autosaver = new Autosaver(m_document);
            
            createGui();
            createMenuBar(false);
            updateTitle();
            m_document->setParentLogger(logger());

            bindEvents();
            bindObservers();

            m_autosaveTimer = new wxTimer(this);
            m_autosaveTimer->Start(1000);
        }

        MapFrame::~MapFrame() {
            unbindObservers();
            
            delete m_autosaveTimer;
            m_autosaveTimer = NULL;
            delete m_autosaver;
            m_autosaver = NULL;

            View::TrenchBroomApp::instance().removeRecentDocumentMenu(Menu::findRecentDocumentsMenu(GetMenuBar()));
        }

        Logger* MapFrame::logger() const {
            return m_infoPanel->console();
        }

        void MapFrame::positionOnScreen(wxFrame* reference) {
            const wxDisplay display;
            const wxRect displaySize = display.GetClientArea();
            if (reference == NULL) {
                SetSize(std::min(displaySize.width, 1024), std::min(displaySize.height, 768));
                CenterOnScreen();
            } else {
                wxPoint position = reference->GetPosition();
                position.x += 23;
                position.y += 23;
                
                if (displaySize.GetBottom() - position.x < 100 ||
                    displaySize.GetRight() - position.y < 70)
                    position = displaySize.GetTopLeft();
                
                SetPosition(position);
                SetSize(std::min(displaySize.GetRight() - position.x, 1024), std::min(displaySize.GetBottom() - position.y, 768));
            }
        }

        bool MapFrame::newDocument(Model::GamePtr game) {
            if (!confirmOrDiscardChanges())
                return false;
            return m_controller->newDocument(MapDocument::DefaultWorldBounds, game);
        }
        
        bool MapFrame::openDocument(Model::GamePtr game, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            return m_controller->openDocument(MapDocument::DefaultWorldBounds, game, path);
        }

        void MapFrame::OnClose(wxCloseEvent& event) {
            assert(m_frameManager != NULL);
            if (event.CanVeto() && !confirmOrDiscardChanges())
                event.Veto();
            else
                m_frameManager->removeAndDestroyFrame(this);
        }

        void MapFrame::OnFileSave(wxCommandEvent& event) {
            saveDocument();
        }
        
        void MapFrame::OnFileSaveAs(wxCommandEvent& event) {
            saveDocumentAs();
        }

        void MapFrame::OnFileLoadPointFile(wxCommandEvent& event) {
            lock(m_document)->loadPointFile();
        }
        
        void MapFrame::OnFileUnloadPointFile(wxCommandEvent& event) {
            lock(m_document)->unloadPointFile();
        }

        void MapFrame::OnFileClose(wxCommandEvent& event) {
            Close();
        }

        void MapFrame::OnEditUndo(wxCommandEvent& event) {
            m_controller->undoLastCommand();
        }
        
        void MapFrame::OnEditRedo(wxCommandEvent& event) {
            m_controller->redoNextCommand();
        }

        void MapFrame::OnEditCut(wxCommandEvent& event) {
            OnEditCopy(event);
            OnEditDeleteObjects(event);
        }
        
        void MapFrame::OnEditCopy(wxCommandEvent& event) {
            OpenClipboard openClipboard;
            if (wxTheClipboard->IsOpened()) {
                StringStream clipboardData;
                if (m_document->hasSelectedObjects())
                    m_document->writeObjectsToStream(m_document->selectedObjects(), clipboardData);
                else if (m_document->hasSelectedFaces())
                    m_document->writeFacesToStream(m_document->selectedFaces(), clipboardData);

                wxTheClipboard->SetData(new wxTextDataObject(clipboardData.str()));
            }
        }
        
        void MapFrame::OnEditPaste(wxCommandEvent& event) {
            OpenClipboard openClipboard;
            if (wxTheClipboard->IsOpened() && wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject textData;
                String text;
                
                if (wxTheClipboard->GetData(textData))
                    text = textData.GetText();

                const Model::EntityList entities = m_document->parseEntities(text);
                const Model::BrushList brushes = m_document->parseBrushes(text);
                const Model::BrushFaceList faces = m_document->parseFaces(text);

                if (!entities.empty() || !brushes.empty()) {
                    const Model::ObjectList objects = VectorUtils::concatenate(VectorUtils::cast<Model::Object*>(entities),
                                                                               VectorUtils::cast<Model::Object*>(brushes));
                    const BBox3 bounds = Model::Object::bounds(objects);
                    const Vec3 delta = m_mapView->pasteObjectsDelta(bounds);
                    pasteObjects(objects, delta);
                } else if (!faces.empty()) {
                    Model::BrushFace* pastedFace = faces.back();
                    Assets::Texture* texture = m_document->textureManager().texture(pastedFace->textureName());
                    pastedFace->setTexture(texture);
                    
                    const Model::BrushFaceList& selectedFaces = m_document->selectedFaces();
                    m_controller->beginUndoableGroup("Paste faces");
                    m_controller->setFaceAttributes(selectedFaces, *pastedFace);
                    m_controller->closeGroup();
                    
                    VectorUtils::deleteAll(faces);

                    StringStream logMsg;
                    logMsg << "Pasted face attributes to " << selectedFaces.size() << (selectedFaces.size() == 1 ? " face" : " faces") << " from clipboard";
                    logger()->info(logMsg.str());
                } else {
                    logger()->error("Could parse clipboard contents");
                }
            } else {
                logger()->error("Clipboard is empty");
            }
        }
        
        void MapFrame::OnEditPasteAtOriginalPosition(wxCommandEvent& event) {
            OpenClipboard openClipboard;
            if (wxTheClipboard->IsOpened() && wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject textData;
                String text;
                
                if (wxTheClipboard->GetData(textData))
                    text = textData.GetText();
                
                const Model::EntityList entities = m_document->parseEntities(text);
                const Model::BrushList brushes = m_document->parseBrushes(text);
                
                if (!entities.empty() || !brushes.empty()) {
                    const Model::ObjectList objects = VectorUtils::concatenate(VectorUtils::cast<Model::Object*>(entities),
                                                                               VectorUtils::cast<Model::Object*>(brushes));
                    pasteObjects(objects, Vec3::Null);
                } else {
                    logger()->error("Could parse clipboard contents");
                }
            } else {
                logger()->error("Clipboard is empty");
            }
        }
        
        void MapFrame::OnEditDeleteObjects(wxCommandEvent& event) {
            const Model::ObjectList objects = m_document->selectedObjects();
            assert(!objects.empty());
            
            m_controller->beginUndoableGroup(String("Delete ") + String(objects.size() == 1 ? "object" : "objects"));
            m_controller->deselectAll();
            m_controller->removeObjects(objects);
            m_controller->closeGroup();
        }

        void MapFrame::OnEditSelectAll(wxCommandEvent& event) {
            m_controller->selectAllObjects();
        }
        
        void MapFrame::OnEditSelectSiblings(wxCommandEvent& event) {
            const Model::BrushList& alreadySelectedBrushes = m_document->selectedBrushes();
            
            Model::EntitySet visitedEntities;
            Model::ObjectList selectBrushes;
            Model::BrushList::const_iterator it, end;
            for (it = alreadySelectedBrushes.begin(), end = alreadySelectedBrushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                Model::Entity* entity = brush->parent();
                if (visitedEntities.insert(entity).second)
                    VectorUtils::append(selectBrushes, entity->brushes());
            }
            
            if (!selectBrushes.empty())
                m_controller->selectObjects(selectBrushes);
        }
        
        void MapFrame::OnEditSelectTouching(wxCommandEvent& event) {
            const Model::BrushList& selectedBrushes = m_document->selectedBrushes();
            assert(selectedBrushes.size() == 1 && !m_document->hasSelectedEntities());

            Model::Brush* selectionBrush = selectedBrushes.front();
            Model::ObjectList selectObjects;
            
            Model::MapObjectsIterator::OuterIterator it = Model::MapObjectsIterator::begin(*m_document->map());
            Model::MapObjectsIterator::OuterIterator end = Model::MapObjectsIterator::end(*m_document->map());
            while (it != end) {
                Model::Object* object = *it;
                if (object != selectionBrush && selectionBrush->intersects(*object))
                    selectObjects.push_back(object);
                ++it;
            }
            
            m_controller->beginUndoableGroup("Select touching objects");
            m_controller->deselectAll();
            m_controller->removeObject(selectionBrush);
            m_controller->selectObjects(selectObjects);
            m_controller->closeGroup();
        }
        
        void MapFrame::OnEditSelectContained(wxCommandEvent& event) {
            const Model::BrushList& selectedBrushes = m_document->selectedBrushes();
            assert(selectedBrushes.size() == 1 && !m_document->hasSelectedEntities());
            
            Model::Brush* selectionBrush = selectedBrushes.front();
            Model::ObjectList selectObjects;
            
            Model::MapObjectsIterator::OuterIterator it = Model::MapObjectsIterator::begin(*m_document->map());
            Model::MapObjectsIterator::OuterIterator end = Model::MapObjectsIterator::end(*m_document->map());
            while (it != end) {
                Model::Object* object = *it;
                if (object != selectionBrush && selectionBrush->contains(*object))
                    selectObjects.push_back(object);
                ++it;
            }
            
            m_controller->beginUndoableGroup("Select contained objects");
            m_controller->deselectAll();
            m_controller->removeObject(selectionBrush);
            m_controller->selectObjects(selectObjects);
            m_controller->closeGroup();
        }
        
        void MapFrame::OnEditSelectByLineNumber(wxCommandEvent& event) {
            const wxString string = wxGetTextFromUser("Enter a comma- or space separated list of line numbers.", "Select by Line Numbers", "", this);
            if (string.empty())
                return;
            
            Model::ObjectList selectObjects;

            wxStringTokenizer tokenizer(string, ", ");
            while (tokenizer.HasMoreTokens()) {
                const wxString token = tokenizer.NextToken();
                long position;
                if (token.ToLong(&position) && position > 0) {
                    Model::Object* object = m_document->map()->findObjectByFilePosition(static_cast<size_t>(position));
                    if (object != NULL)
                        VectorUtils::setInsert(selectObjects, object);
                }
            }

            if (!selectObjects.empty()) {
                m_controller->beginUndoableGroup("Select objects by line numbers");
                m_controller->deselectAll();
                m_controller->selectObjects(selectObjects);
                m_controller->closeGroup();
                
                StringStream message;
                message << "Selected " << selectObjects.size() << " " << (selectObjects.size() == 1 ? "object" : "objects");
                logger()->info(message.str());
            } else {
                logger()->warn("No objects with the given line numbers could be found");
            }
        }
        
        void MapFrame::OnEditSelectNone(wxCommandEvent& event) {
            m_controller->deselectAll();
        }
        
        void MapFrame::OnEditMoveObjectsForward(wxCommandEvent& event) {
            m_mapView->moveObjects(Math::DForward);
        }
        
        void MapFrame::OnEditMoveObjectsBackward(wxCommandEvent& event) {
            m_mapView->moveObjects(Math::DBackward);
        }
        
        void MapFrame::OnEditMoveObjectsLeft(wxCommandEvent& event) {
            m_mapView->moveObjects(Math::DLeft);
        }
        
        void MapFrame::OnEditMoveObjectsRight(wxCommandEvent& event) {
            m_mapView->moveObjects(Math::DRight);
        }
        
        void MapFrame::OnEditMoveObjectsUp(wxCommandEvent& event) {
            m_mapView->moveObjects(Math::DUp);
        }
        
        void MapFrame::OnEditMoveObjectsDown(wxCommandEvent& event) {
            m_mapView->moveObjects(Math::DDown);
        }
        
        void MapFrame::OnEditDuplicateObjectsForward(wxCommandEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Duplicate Objects");
            duplicateObjects();
            m_mapView->moveObjects(Math::DForward);
            controller->closeGroup();
        }
        
        void MapFrame::OnEditDuplicateObjectsBackward(wxCommandEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Duplicate Objects");
            duplicateObjects();
            m_mapView->moveObjects(Math::DBackward);
            controller->closeGroup();
        }
        
        void MapFrame::OnEditDuplicateObjectsLeft(wxCommandEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Duplicate Objects");
            duplicateObjects();
            m_mapView->moveObjects(Math::DLeft);
            controller->closeGroup();
        }
        
        void MapFrame::OnEditDuplicateObjectsRight(wxCommandEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Duplicate Objects");
            duplicateObjects();
            m_mapView->moveObjects(Math::DRight);
            controller->closeGroup();
        }
        
        void MapFrame::OnEditDuplicateObjectsUp(wxCommandEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Duplicate Objects");
            duplicateObjects();
            m_mapView->moveObjects(Math::DUp);
            controller->closeGroup();
        }
        
        void MapFrame::OnEditDuplicateObjectsDown(wxCommandEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Duplicate Objects");
            duplicateObjects();
            m_mapView->moveObjects(Math::DDown);
            controller->closeGroup();
        }
        
        void MapFrame::OnEditRollObjectsCW(wxCommandEvent& event) {
            m_mapView->rotateObjects(RARoll, true);
        }
        
        void MapFrame::OnEditRollObjectsCCW(wxCommandEvent& event) {
            m_mapView->rotateObjects(RARoll, false);
        }
        
        void MapFrame::OnEditPitchObjectsCW(wxCommandEvent& event) {
            m_mapView->rotateObjects(RAPitch, true);
        }
        
        void MapFrame::OnEditPitchObjectsCCW(wxCommandEvent& event) {
            m_mapView->rotateObjects(RAPitch, false);
        }
        
        void MapFrame::OnEditYawObjectsCW(wxCommandEvent& event) {
            m_mapView->rotateObjects(RAYaw, true);
        }
        
        void MapFrame::OnEditYawObjectsCCW(wxCommandEvent& event) {
            m_mapView->rotateObjects(RAYaw, false);
        }
        
        void MapFrame::OnEditFlipObjectsH(wxCommandEvent& event) {
            m_mapView->flipObjects(Math::DLeft);
        }
        
        void MapFrame::OnEditFlipObjectsV(wxCommandEvent& event) {
            m_mapView->flipObjects(Math::DUp);
        }
        
        void MapFrame::OnEditMoveTexturesUp(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DUp, true);
        }
        
        void MapFrame::OnEditMoveTexturesDown(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DDown, true);
        }
        
        void MapFrame::OnEditMoveTexturesLeft(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DLeft, true);
        }
        
        void MapFrame::OnEditMoveTexturesRight(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DRight, true);
        }
        
        void MapFrame::OnEditRotateTexturesCW(wxCommandEvent& event) {
            rotateTextures(true, true);
        }
        
        void MapFrame::OnEditRotateTexturesCCW(wxCommandEvent& event) {
            rotateTextures(false, true);
        }
        
        void MapFrame::OnEditMoveTexturesUpFine(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DUp, false);
        }
        
        void MapFrame::OnEditMoveTexturesDownFine(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DDown, false);
        }
        
        void MapFrame::OnEditMoveTexturesLeftFine(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DLeft, false);
        }
        
        void MapFrame::OnEditMoveTexturesRightFine(wxCommandEvent& event) {
            m_mapView->moveTextures(Math::DRight, false);
        }
        
        void MapFrame::OnEditRotateTexturesCWFine(wxCommandEvent& event) {
            rotateTextures(true, false);
        }
        
        void MapFrame::OnEditRotateTexturesCCWFine(wxCommandEvent& event) {
            rotateTextures(false, false);
        }

        void MapFrame::OnEditToggleClipTool(wxCommandEvent& event) {
            m_mapView->toggleClipTool();
            updateMenuBar(m_mapView->HasFocus());
        }

        void MapFrame::OnEditToggleClipSide(wxCommandEvent& event) {
            m_mapView->toggleClipSide();
        }
        
        void MapFrame::OnEditPerformClip(wxCommandEvent& event) {
            m_mapView->performClip();
        }

        void MapFrame::OnEditDeleteLastClipPoint(wxCommandEvent& event) {
            m_mapView->deleteLastClipPoint();
        }

        void MapFrame::OnEditToggleVertexTool(wxCommandEvent& event) {
            m_mapView->toggleVertexTool();
            updateMenuBar(m_mapView->HasFocus());
        }
        
        void MapFrame::OnEditMoveVerticesForward(wxCommandEvent& event) {
            assert(m_mapView->vertexToolActive());
            m_mapView->moveVertices(Math::DForward);
        }
        
        void MapFrame::OnEditMoveVerticesBackward(wxCommandEvent& event) {
            assert(m_mapView->vertexToolActive());
            m_mapView->moveVertices(Math::DBackward);
        }
        
        void MapFrame::OnEditMoveVerticesLeft(wxCommandEvent& event) {
            assert(m_mapView->vertexToolActive());
            m_mapView->moveVertices(Math::DLeft);
        }
        
        void MapFrame::OnEditMoveVerticesRight(wxCommandEvent& event) {
            assert(m_mapView->vertexToolActive());
            m_mapView->moveVertices(Math::DRight);
        }
        
        void MapFrame::OnEditMoveVerticesUp(wxCommandEvent& event) {
            assert(m_mapView->vertexToolActive());
            m_mapView->moveVertices(Math::DUp);
        }
        
        void MapFrame::OnEditMoveVerticesDown(wxCommandEvent& event) {
            assert(m_mapView->vertexToolActive());
            m_mapView->moveVertices(Math::DDown);
        }

        void MapFrame::OnEditSnapVertices(wxCommandEvent& event) {
            assert(m_mapView->canSnapVertices());
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            m_mapView->snapVertices(grid.actualSize());
            document->info("Snapped brush vertices to grid size %u", grid.actualSize());
        }

        void MapFrame::OnEditToggleTextureTool(wxCommandEvent& event) {
            m_mapView->toggleTextureTool();
            updateMenuBar(m_mapView->HasFocus());
        }

        void MapFrame::OnEditToggleRotateObjectsTool(wxCommandEvent& event) {
            m_mapView->toggleRotateObjectsTool();
            updateMenuBar(m_mapView->HasFocus());
        }

        void MapFrame::OnEditToggleMovementRestriction(wxCommandEvent& event) {
            m_mapView->toggleMovementRestriction();
        }

        void MapFrame::OnEditToggleTextureLock(wxCommandEvent& event) {
            m_document->setTextureLock(!m_document->textureLock());
        }

        void MapFrame::OnViewToggleShowGrid(wxCommandEvent& event) {
            m_document->grid().toggleVisible();
            m_mapView->Refresh();
        }
        
        void MapFrame::OnViewToggleSnapToGrid(wxCommandEvent& event) {
            m_document->grid().toggleSnap();
            m_mapView->Refresh();
        }
        
        void MapFrame::OnViewIncGridSize(wxCommandEvent& event) {
            m_document->grid().incSize();
            m_mapView->Refresh();
        }
        
        void MapFrame::OnViewDecGridSize(wxCommandEvent& event) {
            m_document->grid().decSize();
            m_mapView->Refresh();
        }
        
        void MapFrame::OnViewSetGridSize(wxCommandEvent& event) {
            const size_t size = static_cast<size_t>(event.GetId() - CommandIds::Menu::ViewSetGridSize1);
            assert(size < Grid::MaxSize);
            m_document->grid().setSize(size);
            m_mapView->Refresh();
        }

        void MapFrame::OnViewMoveCameraToNextPoint(wxCommandEvent& event) {
            assert(m_document->isPointFileLoaded());
        
            Model::PointFile& pointFile = m_document->pointFile();
            assert(pointFile.hasNextPoint());
            
            const Vec3f position = pointFile.nextPoint() + Vec3f(0.0f, 0.0f, 16.0f);
            const Vec3f direction = pointFile.currentDirection();
            m_mapView->animateCamera(position, direction, Vec3f::PosZ, 150);
        }
        
        void MapFrame::OnViewMoveCameraToPreviousPoint(wxCommandEvent& event) {
            assert(m_document->isPointFileLoaded());
            
            Model::PointFile& pointFile = m_document->pointFile();
            assert(pointFile.hasPreviousPoint());
            
            const Vec3f position = pointFile.previousPoint() + Vec3f(0.0f, 0.0f, 16.0f);
            const Vec3f direction = pointFile.currentDirection();
            m_mapView->animateCamera(position, direction, Vec3f::PosZ, 150);
        }

        void MapFrame::OnViewCenterCameraOnSelection(wxCommandEvent& event) {
            m_mapView->centerCameraOnSelection();
        }

        void MapFrame::OnViewSwitchToMapInspector(wxCommandEvent& event) {
            m_inspector->switchToPage(MapInspectorPage);
        }
        
        void MapFrame::OnViewSwitchToEntityInspector(wxCommandEvent& event) {
            m_inspector->switchToPage(EntityInspectorPage);
        }
        
        void MapFrame::OnViewSwitchToFaceInspector(wxCommandEvent& event) {
            m_inspector->switchToPage(FaceInspectorPage);
        }

        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            
            switch (event.GetId()) {
                case wxID_OPEN:
                case wxID_SAVE:
                case wxID_SAVEAS:
                case wxID_CLOSE:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::FileLoadPointFile:
                    event.Enable(document->canLoadPointFile());
                    break;
                case CommandIds::Menu::FileUnloadPointFile:
                    event.Enable(document->isPointFileLoaded());
                    break;
                case wxID_UNDO:
                    if (controller->hasLastCommand()) {
                        event.Enable(true);
                        event.SetText(Menu::undoShortcut().menuText(controller->lastCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(Menu::undoShortcut().menuText());
                    }
                    break;
                case wxID_REDO:
                    if (controller->hasNextCommand()) {
                        event.Enable(true);
                        event.SetText(Menu::redoShortcut().menuText(controller->nextCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(Menu::redoShortcut().menuText());
                    }
                    break;
                case wxID_CUT:
                case wxID_COPY:
                    event.Enable(!m_mapView->anyToolActive() &&
                                 (document->hasSelectedObjects() ||
                                  document->selectedFaces().size() == 1));
                    break;
                case wxID_PASTE:
                case CommandIds::Menu::EditPasteAtOriginalPosition: {
                    OpenClipboard openClipboard;
                    static bool canPaste = wxTheClipboard->IsOpened() && wxTheClipboard->IsSupported(wxDF_TEXT);
                    event.Enable(!m_mapView->anyToolActive() && canPaste);
                    break;
                }
                case wxID_DELETE:
                    event.Enable(document->hasSelectedObjects() &&
                                 !m_mapView->anyToolActive());
                    break;
                case CommandIds::Menu::EditSelectAll:
                    event.Enable(!m_mapView->anyToolActive());
                    break;
                case CommandIds::Menu::EditSelectSiblings:
                    event.Enable(!m_mapView->anyToolActive()&&
                                 document->hasSelectedBrushes());
                    break;
                case CommandIds::Menu::EditSelectTouching:
                case CommandIds::Menu::EditSelectContained:
                    event.Enable(!m_mapView->anyToolActive() &&
                                 !document->hasSelectedEntities() &&
                                 document->selectedBrushes().size() == 1);
                    break;
                case CommandIds::Menu::EditSelectByFilePosition:
                    event.Enable(!m_mapView->anyToolActive());
                    break;
                case CommandIds::Menu::EditSelectNone:
                    event.Enable(!m_mapView->anyToolActive() &&
                                 document->hasSelection());
                    break;
                case CommandIds::Menu::EditActions:
                    event.Enable(m_mapView->clipToolActive() ||
                                 m_mapView->hasSelectedVertices() ||
                                 document->hasSelectedObjects() ||
                                 document->hasSelectedFaces());
                    break;
                case CommandIds::Menu::EditMoveTexturesUp:
                case CommandIds::Menu::EditMoveTexturesUpFine:
                case CommandIds::Menu::EditMoveTexturesDown:
                case CommandIds::Menu::EditMoveTexturesDownFine:
                case CommandIds::Menu::EditMoveTexturesLeft:
                case CommandIds::Menu::EditMoveTexturesLeftFine:
                case CommandIds::Menu::EditMoveTexturesRight:
                case CommandIds::Menu::EditMoveTexturesRightFine:
                case CommandIds::Menu::EditRotateTexturesCW:
                case CommandIds::Menu::EditRotateTexturesCWFine:
                case CommandIds::Menu::EditRotateTexturesCCW:
                case CommandIds::Menu::EditRotateTexturesCCWFine:
                    event.Enable(document->hasSelectedFaces());
                    break;
                case CommandIds::Menu::EditMoveObjectsForward:
                case CommandIds::Menu::EditMoveObjectsBackward:
                case CommandIds::Menu::EditMoveObjectsLeft:
                case CommandIds::Menu::EditMoveObjectsRight:
                case CommandIds::Menu::EditMoveObjectsUp:
                case CommandIds::Menu::EditMoveObjectsDown:
                case CommandIds::Menu::EditDuplicateObjectsForward:
                case CommandIds::Menu::EditDuplicateObjectsBackward:
                case CommandIds::Menu::EditDuplicateObjectsLeft:
                case CommandIds::Menu::EditDuplicateObjectsRight:
                case CommandIds::Menu::EditDuplicateObjectsUp:
                case CommandIds::Menu::EditDuplicateObjectsDown:
                case CommandIds::Menu::EditRollObjectsCW:
                case CommandIds::Menu::EditRollObjectsCCW:
                case CommandIds::Menu::EditPitchObjectsCW:
                case CommandIds::Menu::EditPitchObjectsCCW:
                case CommandIds::Menu::EditYawObjectsCW:
                case CommandIds::Menu::EditYawObjectsCCW:
                case CommandIds::Menu::EditFlipObjectsHorizontally:
                case CommandIds::Menu::EditFlipObjectsVertically:
                    event.Enable(document->hasSelectedObjects());
                    break;
                case CommandIds::Menu::EditToggleClipTool:
                    event.Enable(document->hasSelectedBrushes());
                    event.Check(m_mapView->clipToolActive());
                    break;
                case CommandIds::Menu::EditToggleClipSide:
                    event.Enable(m_mapView->clipToolActive() && m_mapView->canToggleClipSide());
                    break;
                case CommandIds::Menu::EditPerformClip:
                    event.Enable(m_mapView->clipToolActive() && m_mapView->canPerformClip());
                    break;
                case CommandIds::Menu::EditDeleteLastClipPoint:
                    event.Enable(m_mapView->clipToolActive() && m_mapView->canDeleteLastClipPoint());
                    break;
                case CommandIds::Menu::EditToggleVertexTool:
                    event.Enable(document->hasSelectedObjects() || m_mapView->vertexToolActive());
                    event.Check(m_mapView->vertexToolActive());
                    break;
                case CommandIds::Menu::EditMoveVerticesForward:
                case CommandIds::Menu::EditMoveVerticesBackward:
                case CommandIds::Menu::EditMoveVerticesLeft:
                case CommandIds::Menu::EditMoveVerticesRight:
                case CommandIds::Menu::EditMoveVerticesUp:
                case CommandIds::Menu::EditMoveVerticesDown:
                    event.Enable(m_mapView->vertexToolActive() && m_mapView->hasSelectedVertices());
                    break;
                case CommandIds::Menu::EditSnapVertices:
                    event.Enable(m_mapView->canSnapVertices());
                    break;
                case CommandIds::Menu::EditToggleRotateObjectsTool:
                    event.Enable(document->hasSelectedObjects() || m_mapView->rotateObjectsToolActive());
                    event.Check(m_mapView->rotateObjectsToolActive());
                    break;
                case CommandIds::Menu::EditToggleTextureTool:
                    event.Enable(document->hasSelectedFaces() || (document->hasSelectedBrushes() && !document->hasSelectedEntities()));
                    event.Check(m_mapView->textureToolActive());
                    break;
                case CommandIds::Menu::EditToggleMovementRestriction:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditToggleTextureLock:
                    event.Enable(true);
                    event.Check(document->textureLock());
                    break;
                case CommandIds::Menu::ViewToggleShowGrid:
                    event.Enable(true);
                    event.Check(document->grid().visible());
                    break;
                case CommandIds::Menu::ViewToggleSnapToGrid:
                    event.Enable(true);
                    event.Check(document->grid().snap());
                    break;
                case CommandIds::Menu::ViewIncGridSize:
                    event.Enable(document->grid().size() < Grid::MaxSize);
                    break;
                case CommandIds::Menu::ViewDecGridSize:
                    event.Enable(document->grid().size() > 0);
                    break;
                case CommandIds::Menu::ViewSetGridSize1:
                    event.Enable(true);
                    event.Check(document->grid().size() == 0);
                    break;
                case CommandIds::Menu::ViewSetGridSize2:
                    event.Enable(true);
                    event.Check(document->grid().size() == 1);
                    break;
                case CommandIds::Menu::ViewSetGridSize4:
                    event.Enable(true);
                    event.Check(document->grid().size() == 2);
                    break;
                case CommandIds::Menu::ViewSetGridSize8:
                    event.Enable(true);
                    event.Check(document->grid().size() == 3);
                    break;
                case CommandIds::Menu::ViewSetGridSize16:
                    event.Enable(true);
                    event.Check(document->grid().size() == 4);
                    break;
                case CommandIds::Menu::ViewSetGridSize32:
                    event.Enable(true);
                    event.Check(document->grid().size() == 5);
                    break;
                case CommandIds::Menu::ViewSetGridSize64:
                    event.Enable(true);
                    event.Check(document->grid().size() == 6);
                    break;
                case CommandIds::Menu::ViewSetGridSize128:
                    event.Enable(true);
                    event.Check(document->grid().size() == 7);
                    break;
                case CommandIds::Menu::ViewSetGridSize256:
                    event.Enable(true);
                    event.Check(document->grid().size() == 8);
                    break;
                case CommandIds::Menu::ViewMoveCameraToNextPoint:
                    event.Enable(document->isPointFileLoaded() && document->pointFile().hasNextPoint());
                    break;
                case CommandIds::Menu::ViewMoveCameraToPreviousPoint:
                    event.Enable(document->isPointFileLoaded() && document->pointFile().hasPreviousPoint());
                    break;
                case CommandIds::Menu::ViewCenterCameraOnSelection:
                    event.Enable(document->hasSelectedObjects());
                    break;
                case CommandIds::Menu::ViewSwitchToMapInspector:
                case CommandIds::Menu::ViewSwitchToEntityInspector:
                case CommandIds::Menu::ViewSwitchToFaceInspector:
                    event.Enable(true);
                    break;
                default:
                    event.Enable(false);
                    break;
            }
        }

        void MapFrame::OnRebuildMenuBar(wxEvent& event) {
            rebuildMenuBar();
        }

        void MapFrame::OnAutosaveTimer(wxTimerEvent& event) {
            m_autosaver->triggerAutosave(logger());
        }

        void MapFrame::OnIdleSetFocusToMapView(wxIdleEvent& event) {
            // we use this method to ensure that the 3D view gets the focus after startup has settled down
            if (m_mapView != NULL) {
                if (!m_mapView->HasFocus()) {
                    m_mapView->SetFocus();
                } else {
                    Unbind(wxEVT_IDLE, &MapFrame::OnIdleSetFocusToMapView, this);
                    rebuildMenuBar();
                    m_mapView->Refresh();
                }
            }
        }

        void MapFrame::bindObservers() {
            m_document->selectionDidChangeNotifier.addObserver(this, &MapFrame::selectionDidChange);
            m_controller->commandDoneNotifier.addObserver(this, &MapFrame::commandDone);
            m_controller->commandUndoneNotifier.addObserver(this, &MapFrame::commandUndone);
        }
        
        void MapFrame::unbindObservers() {
            m_document->selectionDidChangeNotifier.removeObserver(this, &MapFrame::selectionDidChange);
            m_controller->commandDoneNotifier.removeObserver(this, &MapFrame::commandDone);
            m_controller->commandUndoneNotifier.removeObserver(this, &MapFrame::commandUndone);
        }
        
        void MapFrame::selectionDidChange(const Model::SelectionResult& result) {
            rebuildMenuBar();
        }

        void MapFrame::commandDone(Controller::Command::Ptr command) {
            if (command->modifiesDocument()) {
                m_document->incModificationCount();
                m_autosaver->updateLastModificationTime();
            }
            updateTitle();
        }
        
        void MapFrame::commandUndone(Controller::Command::Ptr command) {
            if (command->modifiesDocument())
                m_document->decModificationCount();
            updateTitle();
        }

        void MapFrame::createGui() {
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(350);
            
            wxSplitterWindow* consoleSplitter = new wxSplitterWindow(inspectorSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
            consoleSplitter->SetSashGravity(1.0f);
            consoleSplitter->SetMinimumPaneSize(0);
            
            m_infoPanel = new InfoPanel(consoleSplitter, m_document, m_controller);

            wxPanel* container = new wxPanel(consoleSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
#ifdef _WIN32
                                             wxBORDER_SUNKEN
#else
                                             wxBORDER_NONE
#endif
                                             );
            
            m_navBar = new NavBar(container);
            m_mapView = new MapView(container, logger(), m_document, m_controller, m_camera3D);

            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_navBar, 0, wxEXPAND);
            containerSizer->Add(m_mapView, 1, wxEXPAND);
            container->SetSizer(containerSizer);
            
            consoleSplitter->SplitHorizontally(container, m_infoPanel, -150);
            m_inspector = new Inspector(inspectorSplitter, m_document, m_controller, m_mapView->renderResources(), m_camera3D);
            inspectorSplitter->SplitVertically(consoleSplitter, m_inspector, -350);

            m_statusBar = new StatusBar(this, m_document, m_infoPanel->console());
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND);
            outerSizer->Add(m_statusBar, 0, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        void MapFrame::bindEvents() {
            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileSave, this, wxID_SAVE);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileSaveAs, this, wxID_SAVEAS);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileLoadPointFile, this, CommandIds::Menu::FileLoadPointFile);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileUnloadPointFile, this, CommandIds::Menu::FileUnloadPointFile);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileClose, this, wxID_CLOSE);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditUndo, this, wxID_UNDO);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRedo, this, wxID_REDO);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditCut, this, wxID_CUT);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditCopy, this, wxID_COPY);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditPaste, this, wxID_PASTE);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditPasteAtOriginalPosition, this, CommandIds::Menu::EditPasteAtOriginalPosition);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDeleteObjects, this, wxID_DELETE);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditSelectAll, this, CommandIds::Menu::EditSelectAll);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditSelectSiblings, this, CommandIds::Menu::EditSelectSiblings);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditSelectTouching, this, CommandIds::Menu::EditSelectTouching);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditSelectContained, this, CommandIds::Menu::EditSelectContained);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditSelectByLineNumber, this, CommandIds::Menu::EditSelectByFilePosition);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditSelectNone, this, CommandIds::Menu::EditSelectNone);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesUp, this, CommandIds::Menu::EditMoveTexturesUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesUpFine, this, CommandIds::Menu::EditMoveTexturesUpFine);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesDown, this, CommandIds::Menu::EditMoveTexturesDown);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesDownFine, this, CommandIds::Menu::EditMoveTexturesDownFine);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesLeft, this, CommandIds::Menu::EditMoveTexturesLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesLeftFine, this, CommandIds::Menu::EditMoveTexturesLeftFine);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesRight, this, CommandIds::Menu::EditMoveTexturesRight);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveTexturesRightFine, this, CommandIds::Menu::EditMoveTexturesRightFine);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRotateTexturesCW, this, CommandIds::Menu::EditRotateTexturesCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRotateTexturesCWFine, this, CommandIds::Menu::EditRotateTexturesCWFine);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRotateTexturesCCW, this, CommandIds::Menu::EditRotateTexturesCCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRotateTexturesCCWFine, this, CommandIds::Menu::EditRotateTexturesCCWFine);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveObjectsForward, this, CommandIds::Menu::EditMoveObjectsForward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveObjectsBackward, this, CommandIds::Menu::EditMoveObjectsBackward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveObjectsLeft, this, CommandIds::Menu::EditMoveObjectsLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveObjectsRight, this, CommandIds::Menu::EditMoveObjectsRight);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveObjectsUp, this, CommandIds::Menu::EditMoveObjectsUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveObjectsDown, this, CommandIds::Menu::EditMoveObjectsDown);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDuplicateObjectsForward, this, CommandIds::Menu::EditDuplicateObjectsForward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDuplicateObjectsBackward, this, CommandIds::Menu::EditDuplicateObjectsBackward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDuplicateObjectsLeft, this, CommandIds::Menu::EditDuplicateObjectsLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDuplicateObjectsRight, this, CommandIds::Menu::EditDuplicateObjectsRight);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDuplicateObjectsUp, this, CommandIds::Menu::EditDuplicateObjectsUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDuplicateObjectsDown, this, CommandIds::Menu::EditDuplicateObjectsDown);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRollObjectsCW, this, CommandIds::Menu::EditRollObjectsCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditRollObjectsCCW, this, CommandIds::Menu::EditRollObjectsCCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditPitchObjectsCW, this, CommandIds::Menu::EditPitchObjectsCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditPitchObjectsCCW, this, CommandIds::Menu::EditPitchObjectsCCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditYawObjectsCW, this, CommandIds::Menu::EditYawObjectsCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditYawObjectsCCW, this, CommandIds::Menu::EditYawObjectsCCW);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditFlipObjectsH, this, CommandIds::Menu::EditFlipObjectsHorizontally);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditFlipObjectsV, this, CommandIds::Menu::EditFlipObjectsVertically);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleClipTool, this, CommandIds::Menu::EditToggleClipTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleClipSide, this, CommandIds::Menu::EditToggleClipSide);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditPerformClip, this, CommandIds::Menu::EditPerformClip);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDeleteLastClipPoint, this, CommandIds::Menu::EditDeleteLastClipPoint);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleRotateObjectsTool, this, CommandIds::Menu::EditToggleRotateObjectsTool);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleVertexTool, this, CommandIds::Menu::EditToggleVertexTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveVerticesForward, this, CommandIds::Menu::EditMoveVerticesForward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveVerticesBackward, this, CommandIds::Menu::EditMoveVerticesBackward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveVerticesLeft, this, CommandIds::Menu::EditMoveVerticesLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveVerticesRight, this, CommandIds::Menu::EditMoveVerticesRight);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveVerticesUp, this, CommandIds::Menu::EditMoveVerticesUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditMoveVerticesDown, this, CommandIds::Menu::EditMoveVerticesDown);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditSnapVertices, this, CommandIds::Menu::EditSnapVertices);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleTextureTool, this, CommandIds::Menu::EditToggleTextureTool);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleMovementRestriction, this, CommandIds::Menu::EditToggleMovementRestriction);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleTextureLock, this, CommandIds::Menu::EditToggleTextureLock);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewToggleShowGrid, this, CommandIds::Menu::ViewToggleShowGrid);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewToggleSnapToGrid, this, CommandIds::Menu::ViewToggleSnapToGrid);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewIncGridSize, this, CommandIds::Menu::ViewIncGridSize);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewDecGridSize, this, CommandIds::Menu::ViewDecGridSize);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewSetGridSize, this, CommandIds::Menu::ViewSetGridSize1, CommandIds::Menu::ViewSetGridSize256);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewMoveCameraToNextPoint, this, CommandIds::Menu::ViewMoveCameraToNextPoint);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewMoveCameraToPreviousPoint, this, CommandIds::Menu::ViewMoveCameraToPreviousPoint);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewCenterCameraOnSelection, this, CommandIds::Menu::ViewCenterCameraOnSelection);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewSwitchToMapInspector, this, CommandIds::Menu::ViewSwitchToMapInspector);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewSwitchToEntityInspector, this, CommandIds::Menu::ViewSwitchToEntityInspector);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewSwitchToFaceInspector, this, CommandIds::Menu::ViewSwitchToFaceInspector);
            
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVEAS);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CLOSE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_UNDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_REDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CUT);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_COPY);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_PASTE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_DELETE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Menu::Lowest, CommandIds::Menu::Highest);
            
            Bind(EVT_REBUILD_MENUBAR, &MapFrame::OnRebuildMenuBar, this);
            Bind(wxEVT_TIMER, &MapFrame::OnAutosaveTimer, this);
            
            Bind(wxEVT_ACTIVATE, &MapView::OnActivateFrame, m_mapView);
            Bind(wxEVT_IDLE, &MapFrame::OnIdleSetFocusToMapView, this);
        }
        
        class FrameMenuSelector : public TrenchBroom::View::MultiMenuSelector {
        private:
            const MapView* m_mapView;
            const MapDocumentSPtr m_document;
        public:
            FrameMenuSelector(const MapView* mapView, const MapDocumentSPtr document) :
            m_mapView(mapView),
            m_document(document) {}
            
            const Menu* select(const MultiMenu& multiMenu) const {
                if (m_mapView->clipToolActive())
                    return multiMenu.menuById(CommandIds::Menu::EditClipActions);
                else if (m_mapView->vertexToolActive())
                    return multiMenu.menuById(CommandIds::Menu::EditVertexActions);
                if (m_document->hasSelectedObjects())
                    return multiMenu.menuById(CommandIds::Menu::EditObjectActions);
                if (m_document->hasSelectedFaces())
                    return multiMenu.menuById(CommandIds::Menu::EditFaceActions);
                
                return NULL;
            }
        };
        
        void MapFrame::rebuildMenuBar() {
            updateMenuBar(m_mapView->HasFocus());
        }
        
        void MapFrame::createMenuBar(const bool showModifiers) {
            wxMenuBar* menuBar = Menu::createMenuBar(FrameMenuSelector(m_mapView, m_document), showModifiers);
            SetMenuBar(menuBar);
            
            View::TrenchBroomApp::instance().addRecentDocumentMenu(Menu::findRecentDocumentsMenu(menuBar));
        }
        
        void MapFrame::updateMenuBar(const bool showModifiers) {
            wxMenuBar* oldMenuBar = GetMenuBar();
            View::TrenchBroomApp::instance().removeRecentDocumentMenu(Menu::findRecentDocumentsMenu(oldMenuBar));
            createMenuBar(showModifiers);
            delete oldMenuBar;
        }
        
        void MapFrame::updateTitle() {
#ifdef __APPLE__
            SetTitle(m_document->filename());
            OSXSetModified(m_document->modified());
#else
            SetTitle(wxString(m_document->filename()) + wxString(m_document->modified() ? "*" : ""));
#endif
        }

        bool MapFrame::confirmOrDiscardChanges() {
            if (!m_document->modified())
                return true;
            const int result = ::wxMessageBox(m_document->filename() + " has been modified. Do you want to save the changes?", "TrenchBroom", wxYES_NO | wxCANCEL, this);
            switch (result) {
                case wxYES:
                    return saveDocument();
                case wxNO:
                    return true;
                default:
                    return false;
            }
        }

        bool MapFrame::saveDocument() {
            try {
                const IO::Path& path = m_document->path();
                if (path.isAbsolute() && IO::Disk::fileExists(IO::Disk::fixPath(path))) {
                    m_document->saveDocument();
                    updateTitle();
                    logger()->info("Saved " + m_document->path().asString());
                    return true;
                }
                return saveDocumentAs();
            } catch (FileSystemException e) {
                ::wxMessageBox(e.what(), "", wxOK | wxICON_ERROR, this);
                return false;
            } catch (...) {
                ::wxMessageBox("Unknown error while saving " + m_document->path().asString(), "", wxOK | wxICON_ERROR, this);
                return false;
            }
        }
        
        bool MapFrame::saveDocumentAs() {
            try {
                wxFileDialog saveDialog(this, "Save map file", "", "", "Map files (*.map)|*.map", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                if (saveDialog.ShowModal() == wxID_CANCEL)
                    return false;
                
                const IO::Path path(saveDialog.GetPath().ToStdString());
                m_document->saveDocumentAs(path);
                updateTitle();
                logger()->info("Saved " + m_document->path().asString());
                return true;
            } catch (FileSystemException e) {
                ::wxMessageBox(e.what(), "", wxOK | wxICON_ERROR, this);
                return false;
            } catch (...) {
                ::wxMessageBox("Unknown error while saving " + m_document->filename(), "", wxOK | wxICON_ERROR, this);
                return false;
            }
        }

        void MapFrame::pasteObjects(const Model::ObjectList& objects, const Vec3& delta) {
            assert(!objects.empty());
            
            Model::ObjectParentList pastedObjects;
            Model::ObjectList selectableObjects;
            collectPastedObjects(objects, pastedObjects, selectableObjects);
            
            const String groupName = String("Paste ") + String(objects.size() == 1 ? "Object" : "Objects");
            m_controller->beginUndoableGroup(groupName);
            m_controller->deselectAll();
            m_controller->addObjects(pastedObjects);
            m_controller->selectObjects(selectableObjects);
            if (!delta.null())
                m_controller->moveObjects(selectableObjects, delta, m_document->textureLock());
            m_controller->closeGroup();
            
            StringStream logMsg;
            logMsg << "Pasted " << pastedObjects.size() << (pastedObjects.size() == 1 ? " object" : " objects") << " from clipboard";
            logger()->info(logMsg.str());
        }
        
        void MapFrame::collectPastedObjects(const Model::ObjectList& objects, Model::ObjectParentList& pastedObjects, Model::ObjectList& selectableObjects) {
            Model::Entity* worldspawn = m_document->worldspawn();
            Model::ObjectList::const_iterator oIt, oEnd;
            Model::BrushList::const_iterator bIt, bEnd;
            
            for (oIt = objects.begin(), oEnd = objects.end(); oIt != oEnd; ++oIt) {
                Model::Object* object = *oIt;
                if (object->type() == Model::Object::OTEntity) {
                    Model::Entity* entity = static_cast<Model::Entity*>(object);
                    const Model::BrushList& brushes = entity->brushes();
                    if (!entity->worldspawn()) {
                        pastedObjects.push_back(Model::ObjectParentPair(entity));
                        if (brushes.empty())
                            selectableObjects.push_back(entity);
                        else
                            VectorUtils::append(selectableObjects, brushes);
                    } else {
                        for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                            Model::Brush* brush = *bIt;
                            entity->removeBrush(brush);
                            pastedObjects.push_back(Model::ObjectParentPair(brush, worldspawn));
                            selectableObjects.push_back(brush);
                        }
                        VectorUtils::append(selectableObjects, brushes);
                        delete entity;
                    }
                } else {
                    pastedObjects.push_back(Model::ObjectParentPair(object, worldspawn));
                    selectableObjects.push_back(object);
                }
            }
        }

        void MapFrame::duplicateObjects() {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            ControllerSPtr controller = lock(m_controller);
            const Model::ObjectList duplicates = controller->duplicateObjects(objects, document->worldBounds());
            controller->deselectAllAndSelectObjects(duplicates);
        }

        void MapFrame::rotateTextures(const bool clockwise, const bool snapAngle) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->selectedFaces();
            if (faces.empty())
                return;
            
            const Grid& grid = document->grid();
            const float angle = snapAngle ? Math::degrees(grid.angle()) : 1.0f;
            
            ControllerSPtr controller = lock(m_controller);
            controller->rotateTextures(faces, clockwise ? angle : -angle);
        }
    }
}
