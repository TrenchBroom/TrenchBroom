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
#include "Model/AddObjectsQuery.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/Object.h"
#include "Model/Selection.h"
#include "View/Action.h"
#include "View/ActionManager.h"
#include "View/Autosaver.h"
#include "View/CommandIds.h"
#include "View/InfoPanel.h"
#include "View/FrameManager.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapFrameDropTarget.h"
#include "View/MapView.h"
#include "View/Menu.h"
#include "View/MapViewBar.h"
#include "View/ReplaceTextureFrame.h"
#include "View/SplitterWindow.h"
#include "View/StatusBar.h"

#include <wx/clipbrd.h>
#include <wx/display.h>
#include <wx/persist.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>

namespace TrenchBroom {
    namespace View {
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
        wxFrame(NULL, wxID_ANY, "MapFrame"),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_infoPanel(NULL),
        m_mapViewBar(NULL),
        m_mapView(NULL) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        wxFrame(NULL, wxID_ANY, "MapFrame"),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_infoPanel(NULL),
        m_mapViewBar(NULL),
        m_mapView(NULL) {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocumentSPtr document) {
            m_frameManager = frameManager;
            m_document = document;
            m_controller = ControllerSPtr(new ControllerFacade(m_document));
            m_autosaver = new Autosaver(m_document);
            
            createGui();
            createMenuBar();
            updateTitle();
            m_document->setParentLogger(logger());

            bindEvents();
            bindObservers();

            m_autosaveTimer = new wxTimer(this);
            m_autosaveTimer->Start(1000);

            clearMapViewDropTarget();
        }

        MapFrame::~MapFrame() {
            unbindObservers();
            
            delete m_autosaveTimer;
            m_autosaveTimer = NULL;
            delete m_autosaver;
            m_autosaver = NULL;

            TrenchBroomApp& app = TrenchBroomApp::instance();
            
            wxMenu* recentDocumentsMenu = ActionManager::findRecentDocumentsMenu(GetMenuBar());
            app.removeRecentDocumentMenu(recentDocumentsMenu);
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

        bool MapFrame::newDocument(Model::GamePtr game, const Model::MapFormat::Type mapFormat) {
            if (!confirmOrDiscardChanges())
                return false;
            return m_controller->newDocument(MapDocument::DefaultWorldBounds, game, mapFormat);
        }
        
        bool MapFrame::openDocument(Model::GamePtr game, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            return m_controller->openDocument(MapDocument::DefaultWorldBounds, game, path);
        }

        void MapFrame::setMapViewDropTarget() {
            SetDropTarget(NULL);
            m_mapView->setToolboxDropTarget();
        }
        
        void MapFrame::clearMapViewDropTarget() {
            m_mapView->clearToolboxDropTarget();
            SetDropTarget(new MapFrameDropTarget(m_document, m_controller, this));
        }

        void MapFrame::OnMapViewBarSize(wxSizeEvent& event) {
            m_inspector->setTabBarHeight(event.GetSize().y);
            event.Skip();
        }

        void MapFrame::OnClose(wxCloseEvent& event) {
            if (!IsBeingDeleted()) {
                assert(m_frameManager != NULL);
                if (event.CanVeto() && !confirmOrDiscardChanges())
                    event.Veto();
                else
                    m_frameManager->removeAndDestroyFrame(this);
            }
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

        void MapFrame::OnEditRepeat(wxCommandEvent& event) {
            m_controller->repeatLastCommands();
        }
        
        void MapFrame::OnEditClearRepeat(wxCommandEvent& event) {
            m_controller->clearRepeatableCommands();
        }

        void MapFrame::OnEditCut(wxCommandEvent& event) {
            OnEditCopy(event);
            m_controller->deleteSelectedObjects();
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
                    m_controller->setFaceAttributes(selectedFaces, *pastedFace);
                    
                    VectorUtils::deleteAll(faces);

                    StringStream logMsg;
                    logMsg << "Pasted face attributes to " << selectedFaces.size() << (selectedFaces.size() == 1 ? " face" : " faces") << " from clipboard";
                    logger()->info(logMsg.str());
                } else {
                    logger()->error("Could not parse clipboard contents");
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
                    logger()->error("Could not parse clipboard contents");
                }
            } else {
                logger()->error("Clipboard is empty");
            }
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
        
        class CollectIntersectingObjects : public Model::ObjectVisitor {
        private:
            const Model::Brush* m_brush;
            Model::ObjectList m_objects;
        public:
            CollectIntersectingObjects(const Model::Brush* brush) :
            m_brush(brush) {
                assert(m_brush != NULL);
            }
            
            const Model::ObjectList& objects() const {
                return m_objects;
            }
        private:
            void doVisit(Model::Entity* entity) {
                if (entity->pointEntity() && m_brush->intersects(*entity))
                    m_objects.push_back(entity);
            }
            
            void doVisit(Model::Brush* brush) {
                if (brush != m_brush && m_brush->intersects(*brush))
                    m_objects.push_back(brush);
            }
        };
        
        void MapFrame::OnEditSelectTouching(wxCommandEvent& event) {
            const Model::BrushList& selectedBrushes = m_document->selectedBrushes();
            assert(selectedBrushes.size() == 1 && !m_document->hasSelectedEntities());

            const Model::EntityList& entities = m_document->map()->entities();
            Model::Brush* selectionBrush = selectedBrushes.front();
            CollectIntersectingObjects collect(selectionBrush);
            Model::Object::acceptRecursively(entities.begin(), entities.end(), collect);
            
            const UndoableCommandGroup commandGroup(m_controller, "Select touching objects");
            m_controller->deselectAll();
            m_controller->removeObject(selectionBrush);
            m_controller->selectObjects(collect.objects());
        }
        
        class CollectContainedObjects : public Model::ObjectVisitor {
        private:
            const Model::Brush* m_brush;
            Model::ObjectList m_objects;
        public:
            CollectContainedObjects(const Model::Brush* brush) :
            m_brush(brush) {
                assert(m_brush != NULL);
            }
            
            const Model::ObjectList& objects() const {
                return m_objects;
            }
        private:
            void doVisit(Model::Entity* entity) {
                if (entity->pointEntity() && m_brush->contains(*entity))
                    m_objects.push_back(entity);
            }
            
            void doVisit(Model::Brush* brush) {
                if (brush != m_brush && m_brush->contains(*brush))
                    m_objects.push_back(brush);
            }
        };

        void MapFrame::OnEditSelectInside(wxCommandEvent& event) {
            const Model::BrushList& selectedBrushes = m_document->selectedBrushes();
            assert(selectedBrushes.size() == 1 && !m_document->hasSelectedEntities());
            
            const Model::EntityList& entities = m_document->map()->entities();
            Model::Brush* selectionBrush = selectedBrushes.front();
            CollectContainedObjects collect(selectionBrush);
            Model::Object::acceptRecursively(entities.begin(), entities.end(), collect);
            
            const UndoableCommandGroup commandGroup(m_controller, "Select contained objects");
            m_controller->deselectAll();
            m_controller->removeObject(selectionBrush);
            m_controller->selectObjects(collect.objects());
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
                const UndoableCommandGroup commandGroup(m_controller, "Select objects by line numbers");
                m_controller->deselectAll();
                m_controller->selectObjects(selectObjects);
                
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

        void MapFrame::OnEditSnapVertices(wxCommandEvent& event) {
            assert(m_mapView->canSnapVertices());
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            m_mapView->snapVertices(grid.actualSize());
            document->info("Snapped brush vertices to grid size %u", grid.actualSize());
        }

        void MapFrame::OnEditReplaceTexture(wxCommandEvent& event) {
            ReplaceTextureFrame* frame = new ReplaceTextureFrame(this, m_mapView->contextHolder(), m_document, m_controller);
            frame->CenterOnParent();
            frame->Show();
        }

        void MapFrame::OnEditToggleTextureTool(wxCommandEvent& event) {
            m_mapView->toggleTextureTool();
        }

        void MapFrame::OnEditToggleTextureLock(wxCommandEvent& event) {
            m_document->setTextureLock(!m_document->textureLock());
        }

        void MapFrame::OnViewToggleShowGrid(wxCommandEvent& event) {
            m_document->grid().toggleVisible();
        }
        
        void MapFrame::OnViewToggleSnapToGrid(wxCommandEvent& event) {
            m_document->grid().toggleSnap();
        }
        
        void MapFrame::OnViewIncGridSize(wxCommandEvent& event) {
            m_document->grid().incSize();
        }
        
        void MapFrame::OnViewDecGridSize(wxCommandEvent& event) {
            m_document->grid().decSize();
        }
        
        void MapFrame::OnViewSetGridSize(wxCommandEvent& event) {
            const size_t size = static_cast<size_t>(event.GetId() - CommandIds::Menu::ViewSetGridSize1);
            assert(size < Grid::MaxSize);
            m_document->grid().setSize(size);
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

        void MapFrame::OnViewMoveCameraToPosition(wxCommandEvent& event) {
            wxTextEntryDialog dialog(this, "Enter a position (x y z) for the camera.", "Move Camera", "0.0 0.0 0.0");
            if (dialog.ShowModal() == wxID_OK) {
                const wxString str = dialog.GetValue();
                const Vec3 position = Vec3::parse(str.ToStdString());
                m_mapView->moveCameraToPosition(position);
            }
        }

        void MapFrame::OnViewSwitchToMapInspector(wxCommandEvent& event) {
            m_inspector->switchToPage(InspectorPage_Map);
        }
        
        void MapFrame::OnViewSwitchToEntityInspector(wxCommandEvent& event) {
            m_inspector->switchToPage(InspectorPage_Entity);
        }
        
        void MapFrame::OnViewSwitchToFaceInspector(wxCommandEvent& event) {
            m_inspector->switchToPage(InspectorPage_Face);
        }

        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            const ActionManager& actionManager = ActionManager::instance();
            
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
                case wxID_UNDO: {
                    const Action* action = actionManager.findMenuAction(wxID_UNDO);
                    assert(action != NULL);
                    if (controller->hasLastCommand()) {
                        event.Enable(true);
                        event.SetText(action->menuItemString(controller->lastCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(action->menuItemString());
                    }
                    break;
                }
                case wxID_REDO: {
                    const Action* action = actionManager.findMenuAction(wxID_REDO);
                    if (controller->hasNextCommand()) {
                        event.Enable(true);
                        event.SetText(action->menuItemString(controller->nextCommandName()));
                    } else {
                        event.Enable(false);
                        event.SetText(action->menuItemString());
                    }
                    break;
                }
                case CommandIds::Menu::EditRepeat:
                case CommandIds::Menu::EditClearRepeat:
                    event.Enable(true);
                    break;
                case wxID_CUT:
                case wxID_COPY:
                    event.Enable(document->hasSelectedObjects() ||
                                 document->selectedFaces().size() == 1);
                    break;
                case wxID_PASTE:
                case CommandIds::Menu::EditPasteAtOriginalPosition: {
                    OpenClipboard openClipboard;
                    event.Enable(wxTheClipboard->IsOpened() && wxTheClipboard->IsSupported(wxDF_TEXT));
                    break;
                }
                case CommandIds::Menu::EditSelectAll:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditSelectSiblings:
                    event.Enable(document->hasSelectedBrushes());
                    break;
                case CommandIds::Menu::EditSelectTouching:
                case CommandIds::Menu::EditSelectInside:
                    event.Enable(!document->hasSelectedEntities() &&
                                 document->selectedBrushes().size() == 1);
                    break;
                case CommandIds::Menu::EditSelectByFilePosition:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditSelectNone:
                    event.Enable(document->hasSelection());
                    break;
                case CommandIds::Menu::EditSnapVertices:
                    event.Enable(m_mapView->canSnapVertices());
                    break;
                case CommandIds::Menu::EditReplaceTexture:
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
                case CommandIds::Menu::ViewMoveCameraToPosition:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewToggleCameraFlyMode:
                    event.Enable(true);
                    event.Check(m_mapView->cameraFlyModeActive());
                    break;
                case CommandIds::Menu::ViewSwitchToMapInspector:
                case CommandIds::Menu::ViewSwitchToEntityInspector:
                case CommandIds::Menu::ViewSwitchToFaceInspector:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::FileOpenRecent:
                    event.Enable(true);
                    break;
                default:
                    if (event.GetId() >= CommandIds::Menu::FileRecentDocuments &&
                        event.GetId() < CommandIds::Menu::FileRecentDocuments + 10)
                        event.Enable(true);
                    else
                        event.Enable(false);
                    break;
            }
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
                    m_mapView->Refresh();
                }
            }
        }

        void MapFrame::bindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapFrame::preferenceDidChange);
            m_controller->commandDoneNotifier.addObserver(this, &MapFrame::commandDone);
            m_controller->commandUndoneNotifier.addObserver(this, &MapFrame::commandUndone);
        }
        
        void MapFrame::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapFrame::preferenceDidChange);
            m_controller->commandDoneNotifier.removeObserver(this, &MapFrame::commandDone);
            m_controller->commandUndoneNotifier.removeObserver(this, &MapFrame::commandUndone);
        }

        void MapFrame::preferenceDidChange(const IO::Path& path) {
            const ActionManager& actionManager = ActionManager::instance();
            if (actionManager.isMenuShortcutPreference(path))
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
            SplitterWindow* horizontalSplitter = new SplitterWindow(this);
            horizontalSplitter->setSashGravity(1.0f);
            horizontalSplitter->SetName("MapFrameHSplitter");
            
            SplitterWindow* verticalSplitter = new SplitterWindow(horizontalSplitter);
            verticalSplitter->setSashGravity(1.0f);
            verticalSplitter->SetName("MapFrameVSplitter");

            m_infoPanel = new InfoPanel(verticalSplitter, m_document, m_controller);

            wxPanel* mapViewContainer = new wxPanel(verticalSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_mapViewBar = new MapViewBar(mapViewContainer, m_document);
            m_mapView = new MapView(mapViewContainer, logger(), m_mapViewBar->toolBook(), m_document, m_controller, m_camera3D);

            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_mapViewBar, 0, wxEXPAND);
            containerSizer->Add(m_mapView, 1, wxEXPAND);
            mapViewContainer->SetSizer(containerSizer);
            
            verticalSplitter->splitHorizontally(mapViewContainer, m_infoPanel, wxSize(100, 100), wxSize(100, 100));
            
            m_inspector = new Inspector(horizontalSplitter, m_mapView->contextHolder(), m_document, m_controller, m_camera3D);
            horizontalSplitter->splitVertically(verticalSplitter, m_inspector, wxSize(350, 100), wxSize(350, 100));

            m_statusBar = new StatusBar(this, m_document, m_infoPanel->console());
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(horizontalSplitter, 1, wxEXPAND);
            outerSizer->Add(m_statusBar, 0, wxEXPAND);
            SetSizer(outerSizer);

            wxPersistenceManager::Get().RegisterAndRestore(horizontalSplitter);
            wxPersistenceManager::Get().RegisterAndRestore(verticalSplitter);
        }
        
        void MapFrame::bindEvents() {
            m_mapViewBar->Bind(wxEVT_SIZE, &MapFrame::OnMapViewBarSize, this);
            
            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
            
            Bind(wxEVT_MENU, &MapFrame::OnFileSave, this, wxID_SAVE);
            Bind(wxEVT_MENU, &MapFrame::OnFileSaveAs, this, wxID_SAVEAS);
            Bind(wxEVT_MENU, &MapFrame::OnFileLoadPointFile, this, CommandIds::Menu::FileLoadPointFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileUnloadPointFile, this, CommandIds::Menu::FileUnloadPointFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileClose, this, wxID_CLOSE);
            Bind(wxEVT_MENU, &MapFrame::OnEditUndo, this, wxID_UNDO);
            Bind(wxEVT_MENU, &MapFrame::OnEditRedo, this, wxID_REDO);
            Bind(wxEVT_MENU, &MapFrame::OnEditCut, this, wxID_CUT);
            Bind(wxEVT_MENU, &MapFrame::OnEditCopy, this, wxID_COPY);
            Bind(wxEVT_MENU, &MapFrame::OnEditPaste, this, wxID_PASTE);
            Bind(wxEVT_MENU, &MapFrame::OnEditPasteAtOriginalPosition, this, CommandIds::Menu::EditPasteAtOriginalPosition);
            
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectAll, this, CommandIds::Menu::EditSelectAll);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectSiblings, this, CommandIds::Menu::EditSelectSiblings);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectTouching, this, CommandIds::Menu::EditSelectTouching);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectInside, this, CommandIds::Menu::EditSelectInside);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectByLineNumber, this, CommandIds::Menu::EditSelectByFilePosition);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectNone, this, CommandIds::Menu::EditSelectNone);

            Bind(wxEVT_MENU, &MapFrame::OnEditSnapVertices, this, CommandIds::Menu::EditSnapVertices);
            Bind(wxEVT_MENU, &MapFrame::OnEditReplaceTexture, this, CommandIds::Menu::EditReplaceTexture);
            
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleTextureLock, this, CommandIds::Menu::EditToggleTextureLock);
            
            Bind(wxEVT_MENU, &MapFrame::OnViewToggleShowGrid, this, CommandIds::Menu::ViewToggleShowGrid);
            Bind(wxEVT_MENU, &MapFrame::OnViewToggleSnapToGrid, this, CommandIds::Menu::ViewToggleSnapToGrid);
            Bind(wxEVT_MENU, &MapFrame::OnViewIncGridSize, this, CommandIds::Menu::ViewIncGridSize);
            Bind(wxEVT_MENU, &MapFrame::OnViewDecGridSize, this, CommandIds::Menu::ViewDecGridSize);
            Bind(wxEVT_MENU, &MapFrame::OnViewSetGridSize, this, CommandIds::Menu::ViewSetGridSize1, CommandIds::Menu::ViewSetGridSize256);

            Bind(wxEVT_MENU, &MapFrame::OnViewMoveCameraToNextPoint, this, CommandIds::Menu::ViewMoveCameraToNextPoint);
            Bind(wxEVT_MENU, &MapFrame::OnViewMoveCameraToPreviousPoint, this, CommandIds::Menu::ViewMoveCameraToPreviousPoint);
            Bind(wxEVT_MENU, &MapFrame::OnViewCenterCameraOnSelection, this, CommandIds::Menu::ViewCenterCameraOnSelection);
            Bind(wxEVT_MENU, &MapFrame::OnViewMoveCameraToPosition, this, CommandIds::Menu::ViewMoveCameraToPosition);

            Bind(wxEVT_MENU, &MapFrame::OnViewSwitchToMapInspector, this, CommandIds::Menu::ViewSwitchToMapInspector);
            Bind(wxEVT_MENU, &MapFrame::OnViewSwitchToEntityInspector, this, CommandIds::Menu::ViewSwitchToEntityInspector);
            Bind(wxEVT_MENU, &MapFrame::OnViewSwitchToFaceInspector, this, CommandIds::Menu::ViewSwitchToFaceInspector);
            
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
            
            Bind(wxEVT_TIMER, &MapFrame::OnAutosaveTimer, this);
            
            Bind(wxEVT_IDLE, &MapFrame::OnIdleSetFocusToMapView, this);
        }
        
        void MapFrame::rebuildMenuBar() {
            wxMenuBar* oldMenuBar = GetMenuBar();
            
            const ActionManager& actionManager = ActionManager::instance();
            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(oldMenuBar);
            assert(recentDocumentsMenu != NULL);
            
            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.removeRecentDocumentMenu(recentDocumentsMenu);
            
            SetMenuBar(NULL);
            delete oldMenuBar;
            
            createMenuBar();
        }
        
        void MapFrame::createMenuBar() {
            const ActionManager& actionManager = ActionManager::instance();
            wxMenuBar* menuBar = actionManager.createMenuBar();
            SetMenuBar(menuBar);
            
            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(menuBar);
            assert(recentDocumentsMenu != NULL);

            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.addRecentDocumentMenu(recentDocumentsMenu);
        }
        
        void MapFrame::updateTitle() {
#ifdef __APPLE__
            SetTitle(m_document->filename());
            OSXSetModified(m_document->modified());
#else
            SetTitle(wxString(m_document->filename()) + wxString(m_document->modified() ? "*" : ""));
#endif
            SetRepresentedFilename(m_document->path().asString());
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
                    View::TrenchBroomApp::instance().updateRecentDocument(path);
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
                View::TrenchBroomApp::instance().updateRecentDocument(path);
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

        class ProcessPastedObjects : public Model::ObjectVisitor {
        private:
            Model::Entity* m_worldspawn;
            Model::Layer* m_layer;
            Model::AddObjectsQuery m_query;
            Model::ObjectList m_selectableObjects;
        public:
            ProcessPastedObjects(Model::Entity* worldspawn, Model::Layer* layer) :
            m_worldspawn(worldspawn),
            m_layer(layer) {
                assert(m_worldspawn != NULL);
                assert(m_layer != NULL);
            }
            
            const Model::AddObjectsQuery& query() const {
                return m_query;
            }
            
            const Model::ObjectList& selectableObjects() const {
                return m_selectableObjects;
            }
        private:
            void doVisit(Model::Entity* entity) {
                // we must make a copy of the brush list here because we might need to remove brushes from the
                // entity later on, which would result in invalid iterators
                const Model::BrushList brushes = entity->brushes();
                if (!entity->worldspawn()) {
                    m_query.addEntity(entity, m_layer);
                    if (brushes.empty())
                        m_selectableObjects.push_back(entity);
                    else
                        VectorUtils::append(m_selectableObjects, brushes);
                } else {
                    m_query.addBrushes(entity->brushes(), m_worldspawn, m_layer);
                    VectorUtils::append(m_selectableObjects, entity->brushes());
                    entity->removeAllBrushes();
                    delete entity;
                }
            }
            
            void doVisit(Model::Brush* brush) {
                m_query.addBrush(brush, m_worldspawn, m_layer);
                m_selectableObjects.push_back(brush);
            }
        };

        void MapFrame::pasteObjects(const Model::ObjectList& objects, const Vec3& delta) {
            assert(!objects.empty());
            
            ProcessPastedObjects pasted(m_document->worldspawn(), m_document->currentLayer());
            Model::Object::accept(objects.begin(), objects.end(), pasted);
            
            const Model::AddObjectsQuery& query = pasted.query();
            const size_t objectCount = query.objectCount();
            const Model::ObjectList& selectableObjects = pasted.selectableObjects();
            
            const UndoableCommandGroup commandGroup(m_controller, String("Paste ") + StringUtils::safePlural(objectCount, "object", "objects"));
            m_controller->deselectAll();
            m_controller->addObjects(query);
            m_controller->selectObjects(selectableObjects);
            if (!delta.null())
                m_controller->moveObjects(selectableObjects, delta, m_document->textureLock());
            
            StringStream logMsg;
            logMsg << "Pasted " << objectCount << StringUtils::safePlural(objectCount, " object", " objects") << " from clipboard";
            logger()->info(logMsg.str());
        }
    }
}
