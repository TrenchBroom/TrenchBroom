/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include "View/Console.h"
#include "View/FrameManager.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapView.h"
#include "View/Menu.h"
#include "View/NavBar.h"

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
        wxFrame(NULL, wxID_ANY, _("")),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_console(NULL),
        m_navBar(NULL),
        m_mapView(NULL) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        wxFrame(NULL, wxID_ANY, _("")),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_console(NULL),
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
            m_document->setParentLogger(m_console);

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
            return m_console;
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
            m_controller->removeObject(*selectionBrush);
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
            m_controller->removeObject(*selectionBrush);
            m_controller->selectObjects(selectObjects);
            m_controller->closeGroup();
        }
        
        void MapFrame::OnEditSelectByLineNumber(wxCommandEvent& event) {
            const wxString string = wxGetTextFromUser(_("Enter a comma- or space separated list of line numbers."), _("Select by Line Numbers"), _(""), this);
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

        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
            switch (event.GetId()) {
                case wxID_OPEN:
                case wxID_SAVE:
                case wxID_SAVEAS:
                case wxID_CLOSE:
                    event.Enable(true);
                    break;
                case wxID_UNDO:
                    if (m_controller->hasLastCommand()) {
                        event.Enable(true);
                        event.SetText(Menu::undoShortcut().menuText(m_controller->lastCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(Menu::undoShortcut().menuText());
                    }
                    break;
                case wxID_REDO:
                    if (m_controller->hasNextCommand()) {
                        event.Enable(true);
                        event.SetText(Menu::redoShortcut().menuText(m_controller->nextCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(Menu::redoShortcut().menuText());
                    }
                    break;
                case wxID_CUT:
                case wxID_COPY:
                    event.Enable(!m_mapView->anyToolActive() &&
                                 (m_document->hasSelectedObjects() ||
                                  m_document->selectedFaces().size() == 1));
                    break;
                case wxID_PASTE:
                case CommandIds::Menu::EditPasteAtOriginalPosition: {
                    OpenClipboard openClipboard;
                    static bool canPaste = wxTheClipboard->IsOpened() && wxTheClipboard->IsSupported(wxDF_TEXT);
                    event.Enable(!m_mapView->anyToolActive() && canPaste);
                    break;
                }
                case wxID_DELETE:
                    event.Enable(m_document->hasSelectedObjects() &&
                                 !m_mapView->anyToolActive());
                    break;
                case CommandIds::Menu::EditSelectAll:
                    event.Enable(!m_mapView->anyToolActive());
                    break;
                case CommandIds::Menu::EditSelectSiblings:
                    event.Enable(!m_mapView->anyToolActive()&&
                                 !m_document->hasSelectedEntities() &&
                                 m_document->hasSelectedBrushes());
                    break;
                case CommandIds::Menu::EditSelectTouching:
                case CommandIds::Menu::EditSelectContained:
                    event.Enable(!m_mapView->anyToolActive() &&
                                 !m_document->hasSelectedEntities() &&
                                 m_document->selectedBrushes().size() == 1);
                    break;
                case CommandIds::Menu::EditSelectByFilePosition:
                    event.Enable(!m_mapView->anyToolActive());
                    break;
                case CommandIds::Menu::EditSelectNone:
                    event.Enable(!m_mapView->anyToolActive() &&
                                 m_document->hasSelection());
                    break;
                case CommandIds::Menu::EditToggleClipTool:
                    event.Enable(m_document->hasSelectedBrushes());
                    event.Check(m_mapView->clipToolActive());
                    break;
                case CommandIds::Menu::EditActions:
                    event.Enable(m_mapView->clipToolActive() ||
                                 m_document->hasSelectedObjects());
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
                case CommandIds::Menu::EditToggleRotateObjectsTool:
                    event.Enable(m_document->hasSelectedObjects());
                    event.Check(m_mapView->rotateObjectsToolActive());
                    break;
                case CommandIds::Menu::EditToggleMovementRestriction:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditToggleTextureLock:
                    event.Enable(true);
                    event.Check(m_document->textureLock());
                    break;
                case CommandIds::Menu::ViewToggleShowGrid:
                    event.Enable(true);
                    event.Check(m_document->grid().visible());
                    break;
                case CommandIds::Menu::ViewToggleSnapToGrid:
                    event.Enable(true);
                    event.Check(m_document->grid().snap());
                    break;
                case CommandIds::Menu::ViewIncGridSize:
                    event.Enable(m_document->grid().size() < Grid::MaxSize);
                    break;
                case CommandIds::Menu::ViewDecGridSize:
                    event.Enable(m_document->grid().size() > 0);
                    break;
                case CommandIds::Menu::ViewSetGridSize1:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 0);
                    break;
                case CommandIds::Menu::ViewSetGridSize2:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 1);
                    break;
                case CommandIds::Menu::ViewSetGridSize4:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 2);
                    break;
                case CommandIds::Menu::ViewSetGridSize8:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 3);
                    break;
                case CommandIds::Menu::ViewSetGridSize16:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 4);
                    break;
                case CommandIds::Menu::ViewSetGridSize32:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 5);
                    break;
                case CommandIds::Menu::ViewSetGridSize64:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 6);
                    break;
                case CommandIds::Menu::ViewSetGridSize128:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 7);
                    break;
                case CommandIds::Menu::ViewSetGridSize256:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == 8);
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
            m_autosaver->triggerAutosave(m_console);
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
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(350);
            
            wxSplitterWindow* consoleSplitter = new wxSplitterWindow(inspectorSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            consoleSplitter->SetSashGravity(1.0f);
            consoleSplitter->SetMinimumPaneSize(0);
            
            m_console = new Console(consoleSplitter);

            wxPanel* container = new wxPanel(consoleSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
#ifdef _WIN32
                                             wxBORDER_SUNKEN
#else
                                             wxBORDER_NONE
#endif
                                             );
            
            m_navBar = new NavBar(container);
            m_mapView = new MapView(container, m_console, m_document, m_controller);

            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_navBar, 0, wxEXPAND);
            containerSizer->Add(m_mapView, 1, wxEXPAND);
            container->SetSizer(containerSizer);
            
            consoleSplitter->SplitHorizontally(container, m_console, -100);
            m_inspector = new Inspector(inspectorSplitter, m_document, m_controller, m_mapView->renderResources());
            inspectorSplitter->SplitVertically(consoleSplitter, m_inspector, -350);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        void MapFrame::bindEvents() {
            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileSave, this, wxID_SAVE);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnFileSaveAs, this, wxID_SAVEAS);
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
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleClipTool, this, CommandIds::Menu::EditToggleClipTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleClipSide, this, CommandIds::Menu::EditToggleClipSide);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditPerformClip, this, CommandIds::Menu::EditPerformClip);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditDeleteLastClipPoint, this, CommandIds::Menu::EditDeleteLastClipPoint);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleRotateObjectsTool, this, CommandIds::Menu::EditToggleRotateObjectsTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleMovementRestriction, this, CommandIds::Menu::EditToggleMovementRestriction);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnEditToggleTextureLock, this, CommandIds::Menu::EditToggleTextureLock);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewToggleShowGrid, this, CommandIds::Menu::ViewToggleShowGrid);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewToggleSnapToGrid, this, CommandIds::Menu::ViewToggleSnapToGrid);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewIncGridSize, this, CommandIds::Menu::ViewIncGridSize);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewDecGridSize, this, CommandIds::Menu::ViewDecGridSize);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapFrame::OnViewSetGridSize, this, CommandIds::Menu::ViewSetGridSize1, CommandIds::Menu::ViewSetGridSize256);
            
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
                if (m_document->hasSelectedObjects())
                    return multiMenu.menuById(CommandIds::Menu::EditObjectActions);
                
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
            const int result = ::wxMessageBox(m_document->filename() + " has been modified. Do you want to save the changes?", _("TrenchBroom"), wxYES_NO | wxCANCEL, this);
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
                if (IO::Disk::fileExists(IO::Disk::fixPath(m_document->path()))) {
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
                wxFileDialog saveDialog(this, _("Save map file"), "", "", "Map files (*.map)|*.map", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
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
            
            const Model::ObjectList pastedObjects = collectPastedObjects(objects);
            const String groupName = String("Paste ") + String(objects.size() == 1 ? "object" : "objects");
            m_controller->beginUndoableGroup(groupName);
            m_controller->deselectAll();
            m_controller->addObjects(objects);
            m_controller->selectObjects(pastedObjects);
            if (!delta.null())
                m_controller->moveObjects(pastedObjects, delta, m_document->textureLock());
            m_controller->closeGroup();
            
            StringStream logMsg;
            logMsg << "Pasted " << objects.size() << (objects.size() == 1 ? " object" : " objects") << " from clipboard";
            logger()->info(logMsg.str());
        }
        
        Model::ObjectList MapFrame::collectPastedObjects(const Model::ObjectList& objects) {
            Model::ObjectList result;
            Model::ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Model::Object* object = *it;
                if (object->type() == Model::Object::OTEntity) {
                    Model::Entity* entity = static_cast<Model::Entity*>(object);
                    const Model::BrushList& brushes = entity->brushes();
                    if (brushes.empty()) {
                        result.push_back(object);
                    } else {
                        result.insert(result.end(), brushes.begin(), brushes.end());
                    }
                } else {
                    result.push_back(object);
                }
            }
            return result;
        }
    }
}
