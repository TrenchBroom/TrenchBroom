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
#include "Preferences.h"
#include "PreferenceManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/ResourceUtils.h"
#include "Model/EditorContext.h"
#include "Model/Node.h"
#include "Model/NodeCollection.h"
#include "Model/PointFile.h"
#include "View/ActionManager.h"
#include "View/Autosaver.h"
#include "View/BorderLine.h"
#include "View/CachingLogger.h"
#include "View/CommandIds.h"
#include "View/Console.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/InfoPanel.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapFrameDropTarget.h"
#include "View/Menu.h"
#include "View/ReplaceTextureFrame.h"
#include "View/SplitterWindow2.h"
#include "View/SwitchableMapViewContainer.h"
#include "View/ViewUtils.h"

#include <wx/clipbrd.h>
#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/persist.h>
#include <wx/sizer.h>
#include <wx/timer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MapFrame::MapFrame() :
        wxFrame(NULL, wxID_ANY, "TrenchBroom"),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_contextManager(NULL),
        m_mapView(NULL),
        m_console(NULL),
        m_inspector(NULL),
        m_lastFocus(NULL),
        m_gridChoice(NULL) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        wxFrame(NULL, wxID_ANY, "TrenchBroom"),
        m_frameManager(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_contextManager(NULL),
        m_mapView(NULL),
        m_console(NULL),
        m_inspector(NULL),
        m_lastFocus(NULL),
        m_gridChoice(NULL)  {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocumentSPtr document) {
            assert(frameManager != NULL);
            assert(document != NULL);

            m_frameManager = frameManager;
            m_document = document;
            m_autosaver = new Autosaver(m_document);

            m_contextManager = new GLContextManager();

            createGui();
            createToolBar();
            createMenuBar();

            m_document->setParentLogger(logger());
            m_document->setViewEffectsService(m_mapView);

            m_autosaveTimer = new wxTimer(this);
            m_autosaveTimer->Start(1000);

            bindObservers();
            bindEvents();

            clearDropTarget();
        }

        MapFrame::~MapFrame() {
            unbindObservers();
            removeRecentDocumentsMenu(GetMenuBar());

            delete m_autosaveTimer;
            m_autosaveTimer = NULL;

            delete m_autosaver;
            m_autosaver = NULL;

            // The order of deletion here is important because both the document and the children
            // need the context manager (and its embedded VBO) to clean up their resources.

            m_document->setViewEffectsService(NULL);
            m_document.reset();
            DestroyChildren();

            delete m_contextManager;
            m_contextManager = NULL;
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

        Logger* MapFrame::logger() const {
            return m_console;
        }

        void MapFrame::setToolBoxDropTarget() {
            SetDropTarget(NULL);
            m_mapView->setToolBoxDropTarget();
        }

        void MapFrame::clearDropTarget() {
            m_mapView->clearDropTarget();
            SetDropTarget(new MapFrameDropTarget(m_document, this));
        }

        bool MapFrame::newDocument(Model::GamePtr game, const Model::MapFormat::Type mapFormat) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->newDocument(mapFormat, MapDocument::DefaultWorldBounds, game);
            return true;
        }

        bool MapFrame::openDocument(Model::GamePtr game, const Model::MapFormat::Type mapFormat, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path);
            return true;
        }

        bool MapFrame::saveDocument() {
            try {
                const IO::Path& path = m_document->path();
                if (path.isAbsolute() && IO::Disk::fileExists(IO::Disk::fixPath(path))) {
                    m_document->saveDocument();
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
                const IO::Path& originalPath = m_document->path();
                const IO::Path directory = originalPath.deleteLastComponent();
                const IO::Path fileName = originalPath.lastComponent();
                wxFileDialog saveDialog(this, "Save map file", directory.asString(), fileName.asString(), "Map files (*.map)|*.map", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                if (saveDialog.ShowModal() == wxID_CANCEL)
                    return false;

                const IO::Path path(saveDialog.GetPath().ToStdString());
                m_document->saveDocumentAs(path);
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

        void MapFrame::updateTitle() {
#ifdef __APPLE__
            SetTitle(m_document->filename());
            OSXSetModified(m_document->modified());
#else
            SetTitle(wxString(m_document->filename()) + wxString(m_document->modified() ? "*" : "") + wxString(" - TrenchBroom"));
#endif
            SetRepresentedFilename(m_document->path().asString());
        }

        void MapFrame::OnChildFocus(wxChildFocusEvent& event) {
            if (IsBeingDeleted()) return;

            wxWindow* focus = FindFocus();
            if (focus != m_lastFocus && focus != this) {
                rebuildMenuBar();
                m_lastFocus = focus;
            }
        }

        void MapFrame::rebuildMenuBar() {
            wxMenuBar* oldMenuBar = GetMenuBar();
            removeRecentDocumentsMenu(oldMenuBar);
            createMenuBar();
#ifndef __LINUX__
#ifndef wxUSE_IDLEMENUUPDATES
            // Don't delete the old menu bar on Ubuntu. It will leak, but otherwise we crash on the next idle update.
            oldMenuBar->Destroy();
#endif
#endif
        }

        void MapFrame::createMenuBar() {
            const ActionManager& actionManager = ActionManager::instance();
            wxMenuBar* menuBar = actionManager.createMenuBar(m_mapView->viewportHasFocus());
            SetMenuBar(menuBar);
            addRecentDocumentsMenu(menuBar);
        }

        void MapFrame::addRecentDocumentsMenu(wxMenuBar* menuBar) {
            const ActionManager& actionManager = ActionManager::instance();
            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(menuBar);
            assert(recentDocumentsMenu != NULL);

            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.addRecentDocumentMenu(recentDocumentsMenu);
        }

        void MapFrame::removeRecentDocumentsMenu(wxMenuBar* menuBar) {
            const ActionManager& actionManager = ActionManager::instance();
            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(menuBar);
            assert(recentDocumentsMenu != NULL);

            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.removeRecentDocumentMenu(recentDocumentsMenu);
        }

        void MapFrame::updateRecentDocumentsMenu() {
            if (m_document->path().isAbsolute())
                View::TrenchBroomApp::instance().updateRecentDocument(m_document->path());
        }

        void MapFrame::createGui() {
            m_hSplitter = new SplitterWindow2(this);
            m_hSplitter->setSashGravity(1.0f);
            m_hSplitter->SetName("MapFrameHSplitter");

            m_vSplitter = new SplitterWindow2(m_hSplitter);
            m_vSplitter->setSashGravity(1.0f);
            m_vSplitter->SetName("MapFrameVSplitter");

            InfoPanel* infoPanel = new InfoPanel(m_vSplitter, m_document);
            m_console = infoPanel->console();
            m_mapView = new SwitchableMapViewContainer(m_vSplitter, m_console, m_document, *m_contextManager);
            m_inspector = new Inspector(m_hSplitter, m_document, *m_contextManager);

            m_mapView->connectTopWidgets(m_inspector);

            m_vSplitter->splitHorizontally(m_mapView, infoPanel, wxSize(100, 100), wxSize(100, 100));
            m_hSplitter->splitVertically(m_vSplitter, m_inspector, wxSize(100, 100), wxSize(350, 100));

            wxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
#if !defined __APPLE__
            frameSizer->Add(new BorderLine(this), 1, wxEXPAND);
#endif
            frameSizer->Add(m_hSplitter, 1, wxEXPAND);

            SetSizer(frameSizer);

            wxPersistenceManager::Get().RegisterAndRestore(m_hSplitter);
            wxPersistenceManager::Get().RegisterAndRestore(m_vSplitter);
        }

        void MapFrame::createToolBar() {
            wxToolBar* toolBar = CreateToolBar(wxTB_DEFAULT_STYLE | wxTB_NODIVIDER | wxTB_FLAT);
            toolBar->SetMargins(2, 2);
            toolBar->AddRadioTool(CommandIds::Menu::EditDeactivateTool, "Default Tool", IO::loadImageResource("NoTool.png"), wxNullBitmap, "Disable Current Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleCreateComplexBrushTool, "Brush Tool", IO::loadImageResource("BrushTool.png"), wxNullBitmap, "Brush Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleClipTool, "Clip Tool", IO::loadImageResource("ClipTool.png"), wxNullBitmap, "Clip Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleVertexTool, "Vertex Tool", IO::loadImageResource("VertexTool.png"), wxNullBitmap, "Vertex Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleRotateObjectsTool, "Rotate Tool", IO::loadImageResource("RotateTool.png"), wxNullBitmap, "Rotate Tool");
            toolBar->AddSeparator();
            toolBar->AddTool(wxID_DUPLICATE, "Duplicate Objects", IO::loadImageResource("DuplicateObjects.png"), wxNullBitmap, wxITEM_NORMAL, "Duplicate Objects");
            toolBar->AddTool(CommandIds::Actions::FlipObjectsHorizontally, "Flip Horizontally", IO::loadImageResource("FlipHorizontally.png"), wxNullBitmap, wxITEM_NORMAL, "Flip Horizontally");
            toolBar->AddTool(CommandIds::Actions::FlipObjectsVertically, "Flip Vertically", IO::loadImageResource("FlipVertically.png"), wxNullBitmap, wxITEM_NORMAL, "Flip Vertically");
            toolBar->AddSeparator();
            toolBar->AddCheckTool(CommandIds::Menu::EditToggleTextureLock, "Texture Lock", textureLockBitmap(), wxNullBitmap, "Toggle Texture Lock");
            toolBar->AddSeparator();

            const wxString gridSizes[9] = { "Grid 1", "Grid 2", "Grid 4", "Grid 8", "Grid 16", "Grid 32", "Grid 64", "Grid 128", "Grid 256" };
            m_gridChoice = new wxChoice(toolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, 9, gridSizes);
            m_gridChoice->SetSelection(static_cast<int>(m_document->grid().size()));
            toolBar->AddControl(m_gridChoice);

            toolBar->Realize();
        }

        void MapFrame::bindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapFrame::preferenceDidChange);

            m_document->documentWasClearedNotifier.addObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentModificationStateDidChangeNotifier.addObserver(this, &MapFrame::documentModificationStateDidChange);

            Grid& grid = m_document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapFrame::gridDidChange);
        }

        void MapFrame::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapFrame::preferenceDidChange);

            m_document->documentWasClearedNotifier.removeObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentModificationStateDidChangeNotifier.removeObserver(this, &MapFrame::documentModificationStateDidChange);

            Grid& grid = m_document->grid();
            grid.gridDidChangeNotifier.removeObserver(this, &MapFrame::gridDidChange);
        }

        void MapFrame::documentWasCleared(View::MapDocument* document) {
            updateTitle();
        }

        void MapFrame::documentDidChange(View::MapDocument* document) {
            updateTitle();
            updateRecentDocumentsMenu();
        }

        void MapFrame::documentModificationStateDidChange() {
            updateTitle();
        }

        void MapFrame::preferenceDidChange(const IO::Path& path) {
            const ActionManager& actionManager = ActionManager::instance();
            if (actionManager.isMenuShortcutPreference(path)) {
                rebuildMenuBar();
            } else if (path == Preferences::MapViewLayout.path())
                m_mapView->switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
        }

        void MapFrame::gridDidChange() {
            const Grid& grid = m_document->grid();
            m_gridChoice->SetSelection(static_cast<int>(grid.size()));
        }

        void MapFrame::bindEvents() {
            Bind(wxEVT_MENU, &MapFrame::OnFileSave, this, wxID_SAVE);
            Bind(wxEVT_MENU, &MapFrame::OnFileSaveAs, this, wxID_SAVEAS);
            Bind(wxEVT_MENU, &MapFrame::OnFileLoadPointFile, this, CommandIds::Menu::FileLoadPointFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileUnloadPointFile, this, CommandIds::Menu::FileUnloadPointFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileClose, this, wxID_CLOSE);

            Bind(wxEVT_MENU, &MapFrame::OnEditUndo, this, wxID_UNDO);
            Bind(wxEVT_MENU, &MapFrame::OnEditRedo, this, wxID_REDO);
            Bind(wxEVT_MENU, &MapFrame::OnEditRepeat, this, CommandIds::Menu::EditRepeat);
            Bind(wxEVT_MENU, &MapFrame::OnEditClearRepeat, this, CommandIds::Menu::EditClearRepeat);

            Bind(wxEVT_MENU, &MapFrame::OnEditCut, this, wxID_CUT);
            Bind(wxEVT_MENU, &MapFrame::OnEditCopy, this, wxID_COPY);
            Bind(wxEVT_MENU, &MapFrame::OnEditPaste, this, wxID_PASTE);
            Bind(wxEVT_MENU, &MapFrame::OnEditPasteAtOriginalPosition, this, CommandIds::Menu::EditPasteAtOriginalPosition);
            Bind(wxEVT_MENU, &MapFrame::OnEditDuplicate, this, wxID_DUPLICATE);
            Bind(wxEVT_MENU, &MapFrame::OnEditDelete, this, wxID_DELETE);

            Bind(wxEVT_MENU, &MapFrame::OnEditSelectAll, this, CommandIds::Menu::EditSelectAll);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectSiblings, this, CommandIds::Menu::EditSelectSiblings);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectTouching, this, CommandIds::Menu::EditSelectTouching);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectInside, this, CommandIds::Menu::EditSelectInside);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectTall, this, CommandIds::Menu::EditSelectTall);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectByLineNumber, this, CommandIds::Menu::EditSelectByFilePosition);
            Bind(wxEVT_MENU, &MapFrame::OnEditSelectNone, this, CommandIds::Menu::EditSelectNone);

            Bind(wxEVT_MENU, &MapFrame::OnEditGroupSelectedObjects, this, CommandIds::Menu::EditGroupSelection);
            Bind(wxEVT_MENU, &MapFrame::OnEditUngroupSelectedObjects, this, CommandIds::Menu::EditUngroupSelection);

            Bind(wxEVT_MENU, &MapFrame::OnEditDeactivateTool, this, CommandIds::Menu::EditDeactivateTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleCreateComplexBrushTool, this, CommandIds::Menu::EditToggleCreateComplexBrushTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleClipTool, this, CommandIds::Menu::EditToggleClipTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleRotateObjectsTool, this, CommandIds::Menu::EditToggleRotateObjectsTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleVertexTool, this, CommandIds::Menu::EditToggleVertexTool);

            Bind(wxEVT_MENU, &MapFrame::OnEditCsgConvexMerge, this, CommandIds::Menu::EditCsgConvexMerge);
            Bind(wxEVT_MENU, &MapFrame::OnEditCsgSubtract, this, CommandIds::Menu::EditCsgSubtract);
            Bind(wxEVT_MENU, &MapFrame::OnEditCsgIntersect, this, CommandIds::Menu::EditCsgIntersect);
            
            Bind(wxEVT_MENU, &MapFrame::OnEditReplaceTexture, this, CommandIds::Menu::EditReplaceTexture);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleTextureLock, this, CommandIds::Menu::EditToggleTextureLock);
            Bind(wxEVT_MENU, &MapFrame::OnEditSnapVertices, this, CommandIds::Menu::EditSnapVertices);

            Bind(wxEVT_MENU, &MapFrame::OnViewToggleShowGrid, this, CommandIds::Menu::ViewToggleShowGrid);
            Bind(wxEVT_MENU, &MapFrame::OnViewToggleSnapToGrid, this, CommandIds::Menu::ViewToggleSnapToGrid);
            Bind(wxEVT_MENU, &MapFrame::OnViewIncGridSize, this, CommandIds::Menu::ViewIncGridSize);
            Bind(wxEVT_MENU, &MapFrame::OnViewDecGridSize, this, CommandIds::Menu::ViewDecGridSize);
            Bind(wxEVT_MENU, &MapFrame::OnViewSetGridSize, this, CommandIds::Menu::ViewSetGridSize1, CommandIds::Menu::ViewSetGridSize256);

            Bind(wxEVT_MENU, &MapFrame::OnViewMoveCameraToNextPoint, this, CommandIds::Menu::ViewMoveCameraToNextPoint);
            Bind(wxEVT_MENU, &MapFrame::OnViewMoveCameraToPreviousPoint, this, CommandIds::Menu::ViewMoveCameraToPreviousPoint);
            Bind(wxEVT_MENU, &MapFrame::OnViewFocusCameraOnSelection, this, CommandIds::Menu::ViewFocusCameraOnSelection);
            Bind(wxEVT_MENU, &MapFrame::OnViewMoveCameraToPosition, this, CommandIds::Menu::ViewMoveCameraToPosition);
            
            Bind(wxEVT_MENU, &MapFrame::OnViewHideSelectedObjects, this, CommandIds::Menu::ViewHideSelection);
            Bind(wxEVT_MENU, &MapFrame::OnViewIsolateSelectedObjects, this, CommandIds::Menu::ViewIsolateSelection);
            Bind(wxEVT_MENU, &MapFrame::OnViewShowHiddenObjects, this, CommandIds::Menu::ViewUnhideAll);

            Bind(wxEVT_MENU, &MapFrame::OnViewSwitchToMapInspector, this, CommandIds::Menu::ViewSwitchToMapInspector);
            Bind(wxEVT_MENU, &MapFrame::OnViewSwitchToEntityInspector, this, CommandIds::Menu::ViewSwitchToEntityInspector);
            Bind(wxEVT_MENU, &MapFrame::OnViewSwitchToFaceInspector, this, CommandIds::Menu::ViewSwitchToFaceInspector);

            Bind(wxEVT_MENU, &MapFrame::OnViewToggleMaximizeCurrentView, this, CommandIds::Menu::ViewToggleMaximizeCurrentView);
            Bind(wxEVT_MENU, &MapFrame::OnViewToggleInfoPanel, this, CommandIds::Menu::ViewToggleInfoPanel);
            Bind(wxEVT_MENU, &MapFrame::OnViewToggleInspector, this, CommandIds::Menu::ViewToggleInspector);

            Bind(wxEVT_MENU, &MapFrame::OnDebugPrintVertices, this, CommandIds::Menu::DebugPrintVertices);
            Bind(wxEVT_MENU, &MapFrame::OnDebugCreateBrush, this, CommandIds::Menu::DebugCreateBrush);
            Bind(wxEVT_MENU, &MapFrame::OnDebugCopyJSShortcutMap, this, CommandIds::Menu::DebugCopyJSShortcuts);
            
            Bind(wxEVT_MENU, &MapFrame::OnFlipObjectsHorizontally, this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_MENU, &MapFrame::OnFlipObjectsVertically, this, CommandIds::Actions::FlipObjectsVertically);

            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVEAS);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CLOSE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_UNDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_REDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CUT);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_COPY);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_PASTE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_DUPLICATE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Menu::Lowest, CommandIds::Menu::Highest);

            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Actions::FlipObjectsVertically);

            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
            Bind(wxEVT_TIMER, &MapFrame::OnAutosaveTimer, this);
            Bind(wxEVT_CHILD_FOCUS, &MapFrame::OnChildFocus, this);

            m_gridChoice->Bind(wxEVT_CHOICE, &MapFrame::OnToolBarSetGridSize, this);
        }

        void MapFrame::OnFileSave(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            saveDocument();
        }

        void MapFrame::OnFileSaveAs(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            saveDocumentAs();
        }

        void MapFrame::OnFileLoadPointFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (canLoadPointFile())
                m_document->loadPointFile();
        }

        void MapFrame::OnFileUnloadPointFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (canUnloadPointFile())
                m_document->unloadPointFile();
        }

        void MapFrame::OnFileClose(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Close();
        }

        void MapFrame::OnEditUndo(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canUndo())
                m_document->undoLastCommand();
        }

        void MapFrame::OnEditRedo(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canRedo())
                m_document->redoNextCommand();
        }

        void MapFrame::OnEditRepeat(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->repeatLastCommands();
        }

        void MapFrame::OnEditClearRepeat(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->clearRepeatableCommands();
        }

        void MapFrame::OnEditCut(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canCut()) {
                copyToClipboard();
                Transaction transaction(m_document, "Cut");
                m_document->deleteObjects();
            }
        }

        void MapFrame::OnEditCopy(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canCopy())
                copyToClipboard();
        }

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

        void MapFrame::copyToClipboard() {
            OpenClipboard openClipboard;
            if (wxTheClipboard->IsOpened()) {
                String str;
                if (m_document->hasSelectedNodes())
                    str = m_document->serializeSelectedNodes();
                else if (m_document->hasSelectedBrushFaces())
                    str = m_document->serializeSelectedBrushFaces();
                wxTheClipboard->SetData(new wxTextDataObject(str));
            }
        }

        void MapFrame::OnEditPaste(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canPaste()) {
                const BBox3 referenceBounds = m_document->referenceBounds();
                Transaction transaction(m_document);
                if (paste() == PT_Node && m_document->hasSelectedNodes()) {
                    const BBox3 bounds = m_document->selectionBounds();
                    const Vec3 delta = m_mapView->pasteObjectsDelta(bounds, referenceBounds);
                    m_document->translateObjects(delta);
                }
            }
        }

        void MapFrame::OnEditPasteAtOriginalPosition(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canPaste())
                paste();
        }

        PasteType MapFrame::paste() {
            OpenClipboard openClipboard;
            if (!wxTheClipboard->IsOpened() || !wxTheClipboard->IsSupported(wxDF_TEXT)) {
                logger()->error("Clipboard is empty");
                return PT_Failed;
            }

            wxTextDataObject textData;
            if (!wxTheClipboard->GetData(textData)) {
                logger()->error("Could not get clipboard contents");
                return PT_Failed;
            }

            const String text = textData.GetText().ToStdString();
            return m_document->paste(text);
        }

        void MapFrame::OnEditDelete(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDelete())
                m_document->deleteObjects();
        }

        void MapFrame::OnEditDuplicate(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDuplicate())
                m_document->duplicateObjects();
        }

        void MapFrame::OnEditSelectAll(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelect())
                m_document->selectAllNodes();
        }

        void MapFrame::OnEditSelectSiblings(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelectSiblings())
                m_document->selectSiblings();
        }

        void MapFrame::OnEditSelectTouching(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelectByBrush())
                m_document->selectTouching(true);
        }

        void MapFrame::OnEditSelectInside(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelectByBrush())
                m_document->selectInside(true);
        }

        void MapFrame::OnEditSelectTall(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canSelectTall())
                m_mapView->selectTall();
        }

        void MapFrame::OnEditSelectByLineNumber(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelect()) {
                const wxString string = wxGetTextFromUser("Enter a comma- or space separated list of line numbers.", "Select by Line Numbers", "", this);
                if (string.empty())
                    return;

                std::vector<size_t> positions;
                wxStringTokenizer tokenizer(string, ", ");
                while (tokenizer.HasMoreTokens()) {
                    const wxString token = tokenizer.NextToken();
                    long position;
                    if (token.ToLong(&position) && position > 0) {
                        positions.push_back(static_cast<size_t>(position));
                    }
                }

                m_document->selectNodesWithFilePosition(positions);
            }
        }

        void MapFrame::OnEditSelectNone(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDeselect())
                m_document->deselectAll();
        }

        void MapFrame::OnEditGroupSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canGroup()) {
                const String name = queryGroupName(this);
                if (!name.empty())
                    m_document->groupSelection(name);
            }
        }

        void MapFrame::OnEditUngroupSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canUngroup())
                m_document->ungroupSelection();
        }

        void MapFrame::OnEditReplaceTexture(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            ReplaceTextureFrame* frame = new ReplaceTextureFrame(this, m_document, *m_contextManager);
            frame->CenterOnParent();
            frame->Show();
        }

        void MapFrame::OnEditDeactivateTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_mapView->deactivateTool();
        }

        void MapFrame::OnEditToggleCreateComplexBrushTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_mapView->toggleCreateComplexBrushTool();
        }

        void MapFrame::OnEditToggleClipTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_mapView->toggleClipTool();
        }

        void MapFrame::OnEditToggleRotateObjectsTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_mapView->toggleRotateObjectsTool();
        }

        void MapFrame::OnEditToggleVertexTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_mapView->toggleVertexTool();
        }

        void MapFrame::OnEditCsgConvexMerge(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDoCsgConvexMerge())
                m_document->csgConvexMerge();
        }

        void MapFrame::OnEditCsgSubtract(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canDoCsgSubtract())
                m_document->csgSubtract();
        }

        void MapFrame::OnEditCsgIntersect(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canDoCsgIntersect())
                m_document->csgIntersect();
        }

        void MapFrame::OnEditToggleTextureLock(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->setTextureLock(!m_document->textureLock());
            GetToolBar()->SetToolNormalBitmap(CommandIds::Menu::EditToggleTextureLock, textureLockBitmap());
        }

        wxBitmap MapFrame::textureLockBitmap() {
            if (m_document->textureLock())
                return IO::loadImageResource("TextureLockOn.png");
            return IO::loadImageResource("TextureLockOff.png");
        }

        void MapFrame::OnEditSnapVertices(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSnapVertices())
                m_document->snapVertices();
        }

        void MapFrame::OnViewToggleShowGrid(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->grid().toggleVisible();
        }

        void MapFrame::OnViewToggleSnapToGrid(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->grid().toggleSnap();
        }

        void MapFrame::OnViewIncGridSize(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canIncGridSize())
                m_document->grid().incSize();
        }

        void MapFrame::OnViewDecGridSize(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDecGridSize())
                m_document->grid().decSize();
        }

        void MapFrame::OnViewSetGridSize(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const size_t size = static_cast<size_t>(event.GetId() - CommandIds::Menu::ViewSetGridSize1);
            assert(size < Grid::MaxSize);
            m_document->grid().setSize(size);
        }

        void MapFrame::OnViewMoveCameraToNextPoint(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canMoveCameraToNextPoint())
                m_mapView->moveCameraToNextTracePoint();
        }

        void MapFrame::OnViewMoveCameraToPreviousPoint(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canMoveCameraToPreviousPoint())
                m_mapView->moveCameraToPreviousTracePoint();
        }

        void MapFrame::OnViewFocusCameraOnSelection(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canFocusCamera())
                m_mapView->focusCameraOnSelection();
        }

        void MapFrame::OnViewMoveCameraToPosition(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxTextEntryDialog dialog(this, "Enter a position (x y z) for the camera.", "Move Camera", "0.0 0.0 0.0");
            if (dialog.ShowModal() == wxID_OK) {
                const wxString str = dialog.GetValue();
                const Vec3 position = Vec3::parse(str.ToStdString());
                m_mapView->moveCameraToPosition(position);
            }
        }
        
        void MapFrame::OnViewHideSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canHide())
                m_document->hideSelection();
        }
        
        void MapFrame::OnViewIsolateSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canIsolate())
                m_document->isolate(m_document->selectedNodes().nodes());
        }
        
        void MapFrame::OnViewShowHiddenObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            m_document->showAll();
        }

        void MapFrame::OnViewSwitchToMapInspector(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_inspector->switchToPage(Inspector::InspectorPage_Map);
        }

        void MapFrame::OnViewSwitchToEntityInspector(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_inspector->switchToPage(Inspector::InspectorPage_Entity);
        }

        void MapFrame::OnViewSwitchToFaceInspector(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_inspector->switchToPage(Inspector::InspectorPage_Face);
        }

        void MapFrame::OnViewToggleMaximizeCurrentView(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
         
            m_mapView->toggleMaximizeCurrentView();
        }

        void MapFrame::OnViewToggleInfoPanel(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_vSplitter->isMaximized(m_mapView))
                m_vSplitter->restore();
            else
                m_vSplitter->maximize(m_mapView);
        }

        void MapFrame::OnViewToggleInspector(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_hSplitter->isMaximized(m_vSplitter))
                m_hSplitter->restore();
            else
                m_hSplitter->maximize(m_vSplitter);
        }

        void MapFrame::OnDebugPrintVertices(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            m_document->printVertices();
        }

        void MapFrame::OnDebugCreateBrush(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            wxTextEntryDialog dialog(this, "Enter a list of at least 4 points (x y z) (x y z) ...", "Create Brush", "");
            if (dialog.ShowModal() == wxID_OK) {
                const wxString str = dialog.GetValue();
                const Vec3::List positions = Vec3::parseList(str.ToStdString());
                m_document->createBrush(positions);
            }
        }

        void MapFrame::OnDebugCopyJSShortcutMap(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            OpenClipboard openClipboard;
            if (wxTheClipboard->IsOpened()) {
                const String str = ActionManager::instance().getJSTable();
                wxTheClipboard->SetData(new wxTextDataObject(str));
            }

        }

        void MapFrame::OnFlipObjectsHorizontally(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            m_mapView->flipObjects(Math::Direction_Left);
        }

        void MapFrame::OnFlipObjectsVertically(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            m_mapView->flipObjects(Math::Direction_Up);
        }

        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const ActionManager& actionManager = ActionManager::instance();

            switch (event.GetId()) {
                case wxID_OPEN:
                case wxID_SAVE:
                case wxID_SAVEAS:
                case wxID_CLOSE:
                case CommandIds::Menu::FileOpenRecent:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::FileLoadPointFile:
                    event.Enable(canLoadPointFile());
                    break;
                case CommandIds::Menu::FileUnloadPointFile:
                    event.Enable(canUnloadPointFile());
                    break;
                case wxID_UNDO: {
                    const ActionMenuItem* item = actionManager.findMenuItem(wxID_UNDO);
                    assert(item != NULL);
                    if (canUndo()) {
                        event.Enable(true);
                        event.SetText(item->menuString(m_document->lastCommandName(), m_mapView->viewportHasFocus()));
                    } else {
                        event.Enable(false);
                        event.SetText(item->menuString("", m_mapView->viewportHasFocus()));
                    }
                    break;
                }
                case wxID_REDO: {
                    const ActionMenuItem* item = actionManager.findMenuItem(wxID_REDO);
                    if (canRedo()) {
                        event.Enable(true);
                        event.SetText(item->menuString(m_document->nextCommandName(), m_mapView->viewportHasFocus()));
                    } else {
                        event.Enable(false);
                        event.SetText(item->menuString("", m_mapView->viewportHasFocus()));
                    }
                    break;
                }
                case CommandIds::Menu::EditRepeat:
                case CommandIds::Menu::EditClearRepeat:
                    event.Enable(true);
                    break;
                case wxID_CUT:
                    event.Enable(canCut());
                    break;
                case wxID_COPY:
                    event.Enable(canCopy());
                    break;
                case wxID_PASTE:
                case CommandIds::Menu::EditPasteAtOriginalPosition:
                    event.Enable(canPaste());
                    break;
                case wxID_DUPLICATE:
                    event.Enable(canDuplicate());
                    break;
                case wxID_DELETE:
                    event.Enable(canDelete());
                    break;
                case CommandIds::Menu::EditSelectAll:
                    event.Enable(canSelect());
                    break;
                case CommandIds::Menu::EditSelectSiblings:
                    event.Enable(canSelectSiblings());
                    break;
                case CommandIds::Menu::EditSelectTouching:
                case CommandIds::Menu::EditSelectInside:
                    event.Enable(canSelectByBrush());
                    break;
                case CommandIds::Menu::EditSelectTall:
                    event.Enable(canSelectTall());
                    break;
                case CommandIds::Menu::EditSelectByFilePosition:
                    event.Enable(canSelect());
                    break;
                case CommandIds::Menu::EditSelectNone:
                    event.Enable(canDeselect());
                    break;
                case CommandIds::Menu::EditGroupSelection:
                    event.Enable(canGroup());
                    break;
                case CommandIds::Menu::EditUngroupSelection:
                    event.Enable(canUngroup());
                    break;
                case CommandIds::Menu::EditDeactivateTool:
                    event.Check(!m_mapView->anyToolActive());
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditToggleCreateComplexBrushTool:
                    event.Check(m_mapView->createComplexBrushToolActive());
                    event.Enable(m_mapView->canToggleCreateComplexBrushTool());
                    break;
                case CommandIds::Menu::EditToggleClipTool:
                    event.Check(m_mapView->clipToolActive());
                    event.Enable(m_mapView->canToggleClipTool());
                    break;
                case CommandIds::Menu::EditToggleRotateObjectsTool:
                    event.Check(m_mapView->rotateObjectsToolActive());
                    event.Enable(m_mapView->canToggleRotateObjectsTool());
                    break;
                case CommandIds::Menu::EditToggleVertexTool:
                    event.Check(m_mapView->vertexToolActive());
                    event.Enable(m_mapView->canToggleVertexTool());
                    break;
                case CommandIds::Menu::EditCsgConvexMerge:
                    event.Enable(canDoCsgConvexMerge());
                    break;
                case CommandIds::Menu::EditCsgSubtract:
                    event.Enable(canDoCsgSubtract());
                    break;
                case CommandIds::Menu::EditCsgIntersect:
                    event.Enable(canDoCsgIntersect());
                    break;
                case CommandIds::Menu::EditSnapVertices:
                    event.Enable(canSnapVertices());
                    break;
                case CommandIds::Menu::EditReplaceTexture:
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
                    event.Enable(canIncGridSize());
                    break;
                case CommandIds::Menu::ViewDecGridSize:
                    event.Enable(canDecGridSize());
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
                case CommandIds::Menu::ViewMoveCameraToNextPoint:
                    event.Enable(canMoveCameraToNextPoint());
                    break;
                case CommandIds::Menu::ViewMoveCameraToPreviousPoint:
                    event.Enable(canMoveCameraToPreviousPoint());
                    break;
                case CommandIds::Menu::ViewFocusCameraOnSelection:
                    event.Enable(canFocusCamera());
                    break;
                case CommandIds::Menu::ViewMoveCameraToPosition:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewHideSelection:
                    event.Enable(canHide());
                    break;
                case CommandIds::Menu::ViewIsolateSelection:
                    event.Enable(canIsolate());
                    break;
                case CommandIds::Menu::ViewUnhideAll:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewSwitchToMapInspector:
                case CommandIds::Menu::ViewSwitchToEntityInspector:
                case CommandIds::Menu::ViewSwitchToFaceInspector:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewToggleMaximizeCurrentView:
                    event.Enable(m_mapView->canMaximizeCurrentView());
                    event.Check(m_mapView->currentViewMaximized());
                    break;
                case CommandIds::Menu::ViewToggleInfoPanel:
                    event.Enable(true);
                    event.Check(m_vSplitter->isMaximized(m_mapView));
                    break;
                case CommandIds::Menu::ViewToggleInspector:
                    event.Enable(true);
                    event.Check(m_hSplitter->isMaximized(m_vSplitter));
                    break;
                case CommandIds::Menu::DebugPrintVertices:
                case CommandIds::Menu::DebugCreateBrush:
                case CommandIds::Menu::DebugCopyJSShortcuts:
                    event.Enable(true);
                    break;
                case CommandIds::Actions::FlipObjectsHorizontally:
                case CommandIds::Actions::FlipObjectsVertically:
                    event.Enable(m_mapView->canFlipObjects());
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

        void MapFrame::OnToolBarSetGridSize(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const size_t size = static_cast<size_t>(event.GetSelection());
            assert(size < Grid::MaxSize);
            m_document->grid().setSize(size);
        }

        bool MapFrame::canLoadPointFile() const {
            return m_document->canLoadPointFile();
        }

        bool MapFrame::canUnloadPointFile() const {
            return m_document->isPointFileLoaded();
        }

        bool MapFrame::canUndo() const {
            return m_document->canUndoLastCommand();
        }

        bool MapFrame::canRedo() const {
            return m_document->canRedoNextCommand();
        }

        bool MapFrame::canCut() const {
            return canDelete();
        }

        bool MapFrame::canCopy() const {
            return m_document->hasSelectedNodes() || m_document->hasSelectedBrushFaces();
        }

        bool MapFrame::canPaste() const {
            OpenClipboard openClipboard;
            return wxTheClipboard->IsOpened() && wxTheClipboard->IsSupported(wxDF_TEXT);
        }

        bool MapFrame::canDelete() const {
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canDuplicate() const {
            return m_document->hasSelectedNodes() && !m_mapView->clipToolActive() && !m_mapView->vertexToolActive();
        }

        bool MapFrame::canSelectSiblings() const {
            return canChangeSelection() && m_document->hasSelectedNodes();
        }

        bool MapFrame::canSelectByBrush() const {
            return canChangeSelection() && m_document->selectedNodes().hasOnlyBrushes();
        }

        bool MapFrame::canSelectTall() const {
            return canChangeSelection() && m_document->selectedNodes().hasOnlyBrushes() && m_mapView->canSelectTall();
        }

        bool MapFrame::canSelect() const {
            return canChangeSelection();
        }

        bool MapFrame::canDeselect() const {
            return canChangeSelection() && m_document->hasSelectedNodes();
        }

        bool MapFrame::canChangeSelection() const {
            return m_document->editorContext().canChangeSelection();
        }

        bool MapFrame::canGroup() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canUngroup() const {
            return m_document->selectedNodes().hasOnlyGroups();
        }

        bool MapFrame::canHide() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canIsolate() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canDoCsgConvexMerge() const {
            return (m_document->hasSelectedBrushFaces() && m_document->selectedBrushFaces().size() > 1) ||
                   (m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1);
        }

        bool MapFrame::canDoCsgSubtract() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1;
        }

        bool MapFrame::canDoCsgIntersect() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1;
        }

        bool MapFrame::canSnapVertices() const {
            return m_document->selectedNodes().hasOnlyBrushes();
        }

        bool MapFrame::canDecGridSize() const {
            return m_document->grid().size() > 0;
        }

        bool MapFrame::canIncGridSize() const {
            return m_document->grid().size() < Grid::MaxSize;
        }

        bool MapFrame::canMoveCameraToNextPoint() const {
            return m_mapView->canMoveCameraToNextTracePoint();
        }

        bool MapFrame::canMoveCameraToPreviousPoint() const {
            return m_mapView->canMoveCameraToPreviousTracePoint();
        }

        bool MapFrame::canFocusCamera() const {
            return m_document->hasSelectedNodes();
        }

        void MapFrame::OnClose(wxCloseEvent& event) {
            if (IsBeingDeleted()) return;

            if (!IsBeingDeleted()) {
                assert(m_frameManager != NULL);
                if (event.CanVeto() && !confirmOrDiscardChanges())
                    event.Veto();
                else
                    m_frameManager->removeAndDestroyFrame(this);
            }
        }

        void MapFrame::OnAutosaveTimer(wxTimerEvent& event) {
            if (IsBeingDeleted()) return;

            m_autosaver->triggerAutosave(logger());
        }
    }
}
