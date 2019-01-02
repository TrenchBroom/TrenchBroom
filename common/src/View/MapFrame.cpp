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
#include "Model/AttributableNode.h"
#include "Model/Brush.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeCollection.h"
#include "Model/PointFile.h"
#include "Model/World.h"
#include "View/ActionManager.h"
#include "View/Autosaver.h"
#include "View/BorderLine.h"
#include "View/CachingLogger.h"
#include "View/ClipTool.h"
#include "View/CommandIds.h"
#include "View/CommandWindowUpdateLocker.h"
#include "View/CompilationDialog.h"
#include "View/Console.h"
#include "View/EdgeTool.h"
#include "View/FaceTool.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/InfoPanel.h"
#include "View/Inspector.h"
#include "View/LaunchGameEngineDialog.h"
#include "View/MapDocument.h"
#include "View/MapFrameDropTarget.h"
#include "View/Menu.h"
#include "View/OpenClipboard.h"
#include "View/RenderView.h"
#include "View/ReplaceTextureDialog.h"
#include "View/SplitterWindow2.h"
#include "View/SwitchableMapViewContainer.h"
#include "View/VertexTool.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <vecmath/util.h>

#include <wx/clipbrd.h>
#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/persist.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/textentry.h>
#include <wx/choice.h>
#include <wx/choicdlg.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>

#include <cassert>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        MapFrame::MapFrame() :
        wxFrame(nullptr, wxID_ANY, "TrenchBroom"),
        m_frameManager(nullptr),
        m_autosaver(nullptr),
        m_autosaveTimer(nullptr),
        m_contextManager(nullptr),
        m_mapView(nullptr),
        m_console(nullptr),
        m_inspector(nullptr),
        m_lastFocus(nullptr),
        m_gridChoice(nullptr),
        m_compilationDialog(nullptr),
        m_updateLocker(nullptr) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        wxFrame(nullptr, wxID_ANY, "TrenchBroom"),
        m_frameManager(nullptr),
        m_autosaver(nullptr),
        m_autosaveTimer(nullptr),
        m_contextManager(nullptr),
        m_mapView(nullptr),
        m_console(nullptr),
        m_inspector(nullptr),
        m_lastFocus(nullptr),
        m_gridChoice(nullptr),
        m_compilationDialog(nullptr),
        m_updateLocker(nullptr) {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocumentSPtr document) {
            ensure(frameManager != nullptr, "frameManager is null");
            ensure(document.get() != nullptr, "document is null");

            m_frameManager = frameManager;
            m_document = document;
            m_autosaver = new Autosaver(m_document);

            m_contextManager = new GLContextManager();

            createGui();
            createToolBar();
            createMenuBar();
            createStatusBar();

            m_document->setParentLogger(logger());
            m_document->setViewEffectsService(m_mapView);

            m_autosaveTimer = new wxTimer(this);
            m_autosaveTimer->Start(1000);

            bindObservers();
            bindEvents();

            clearDropTarget();
            
            m_updateLocker = new CommandWindowUpdateLocker(this, m_document);
#ifdef __APPLE__
            m_updateLocker->Start();
#endif
        }
        
        static RenderView* FindChildRenderView(wxWindow *current) {
            for (wxWindow *child : current->GetChildren()) {
                RenderView *canvas = wxDynamicCast(child, RenderView);
                if (canvas != nullptr)
                    return canvas;
                
                canvas = FindChildRenderView(child);
                if (canvas != nullptr)
                    return canvas;
            }
            return nullptr;
        }

        MapFrame::~MapFrame() {
            // Search for a RenderView (wxGLCanvas subclass) and make it current.
            RenderView* canvas = FindChildRenderView(this);
            if (canvas != nullptr && m_contextManager != nullptr) {
                wxGLContext* mainContext = m_contextManager->mainContext();
                if (mainContext != nullptr)
                    mainContext->SetCurrent(*canvas);
            }

            // The MapDocument's CachingLogger has a pointer to m_console, which
            // is about to be destroyed (DestroyChildren()). Clear the pointer
            // so we don't try to log to a dangling pointer (#1885).
            m_document->setParentLogger(nullptr);

            // Makes IsBeingDeleted() return true
            SendDestroyEvent();

            m_mapView->deactivateTool();
            
            unbindObservers();
            removeRecentDocumentsMenu(GetMenuBar());

            delete m_updateLocker;
            m_updateLocker = nullptr;
            
            delete m_autosaveTimer;
            m_autosaveTimer = nullptr;

            delete m_autosaver;
            m_autosaver = nullptr;

            // The order of deletion here is important because both the document and the children
            // need the context manager (and its embedded VBO) to clean up their resources.

            DestroyChildren(); // Destroy the children first because they might still access document resources.
            
            m_document->setViewEffectsService(nullptr);
            m_document.reset();

            delete m_contextManager;
            m_contextManager = nullptr;
        }

        void MapFrame::positionOnScreen(wxFrame* reference) {
            const wxDisplay display;
            const wxRect displaySize = display.GetClientArea();
            if (reference == nullptr) {
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
                SetSize(std::min(displaySize.GetRight() - position.x, reference->GetSize().x), std::min(displaySize.GetBottom() - position.y, reference->GetSize().y));
            }
        }

        MapDocumentSPtr MapFrame::document() const {
            return m_document;
        }

        Logger* MapFrame::logger() const {
            return m_console;
        }

        void MapFrame::setToolBoxDropTarget() {
            SetDropTarget(nullptr);
            m_mapView->setToolBoxDropTarget();
        }

        void MapFrame::clearDropTarget() {
            m_mapView->clearDropTarget();
            SetDropTarget(new MapFrameDropTarget(m_document, this));
        }

        bool MapFrame::newDocument(Model::GameSPtr game, const Model::MapFormat::Type mapFormat) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->newDocument(mapFormat, MapDocument::DefaultWorldBounds, game);
            return true;
        }

        bool MapFrame::openDocument(Model::GameSPtr game, const Model::MapFormat::Type mapFormat, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path);
            return true;
        }

        bool MapFrame::saveDocument() {
            try {
                if (m_document->persistent()) {
                    m_document->saveDocument();
                    logger()->info("Saved " + m_document->path().asString());
                    return true;
                }
                return saveDocumentAs();
            } catch (const FileSystemException& e) {
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
            } catch (const FileSystemException& e) {
                ::wxMessageBox(e.what(), "", wxOK | wxICON_ERROR, this);
                return false;
            } catch (...) {
                ::wxMessageBox("Unknown error while saving " + m_document->filename(), "", wxOK | wxICON_ERROR, this);
                return false;
            }
        }

        bool MapFrame::exportDocumentAsObj() {
            const IO::Path& originalPath = m_document->path();
            const IO::Path directory = originalPath.deleteLastComponent();
            const IO::Path filename = originalPath.lastComponent().replaceExtension("obj");
            wxString wildcard;
            
            wxFileDialog saveDialog(this, "Export Wavefront OBJ file", directory.asString(), filename.asString(), "Wavefront OBJ files (*.obj)|*.obj", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (saveDialog.ShowModal() == wxID_CANCEL)
                return false;
            
            return exportDocument(Model::WavefrontObj, IO::Path(saveDialog.GetPath().ToStdString()));
        }

        bool MapFrame::exportDocument(const Model::ExportFormat format, const IO::Path& path) {
            try {
                m_document->exportDocumentAs(format, path);
                logger()->info("Exported " + path.asString());
                return true;
            } catch (const FileSystemException& e) {
                ::wxMessageBox(e.what(), "", wxOK | wxICON_ERROR, this);
                return false;
            } catch (...) {
                ::wxMessageBox("Unknown error while exporting " + path.asString(), "", wxOK | wxICON_ERROR, this);
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

#if defined(_WIN32)
		/*
		This and the following method were added to reset the menu bar correctly when the map frame
		regains focus after the preference dialog is closed. Since the map view reports not having focus
		when the activation event is processed, we set up a delayed processing in the next idle event.

		See also issue #1762, which only affects Windows.
		*/
		void MapFrame::OnActivate(wxActivateEvent& event) {
            if (IsBeingDeleted()) return;

			Bind(wxEVT_IDLE, &MapFrame::OnDelayedActivate, this);
			event.Skip();
        }

		void MapFrame::OnDelayedActivate(wxIdleEvent& event) {
			if (IsBeingDeleted()) return;

			Unbind(wxEVT_IDLE, &MapFrame::OnDelayedActivate, this);
			rebuildMenuBar();
			event.Skip();
		}
#endif

		void MapFrame::OnChildFocus(wxChildFocusEvent& event) {
            if (IsBeingDeleted()) return;

            wxWindow* focus = FindFocus();
            if (focus == nullptr)
                focus = event.GetWindow();
            if (focus != m_lastFocus && focus != this) {
                rebuildMenuBar();
                m_lastFocus = focus;
            }

			event.Skip();
        }

        void MapFrame::rebuildMenuBar() {
            wxMenuBar* oldMenuBar = GetMenuBar();
            removeRecentDocumentsMenu(oldMenuBar);
            createMenuBar();
            oldMenuBar->Destroy();
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
            ensure(recentDocumentsMenu != nullptr, "recentDocumentsMenu is null");

            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.addRecentDocumentMenu(recentDocumentsMenu);
        }

        void MapFrame::removeRecentDocumentsMenu(wxMenuBar* menuBar) {
            const ActionManager& actionManager = ActionManager::instance();
            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(menuBar);
            ensure(recentDocumentsMenu != nullptr, "recentDocumentsMenu is null");

            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.removeRecentDocumentMenu(recentDocumentsMenu);
        }

        void MapFrame::updateRecentDocumentsMenu() {
            if (m_document->path().isAbsolute())
                View::TrenchBroomApp::instance().updateRecentDocument(m_document->path());
        }

        void MapFrame::createGui() {
            setWindowIcon(this);

            m_hSplitter = new SplitterWindow2(this);
            m_hSplitter->setSashGravity(1.0);
            m_hSplitter->SetName("MapFrameHSplitter");

            m_vSplitter = new SplitterWindow2(m_hSplitter);
            m_vSplitter->setSashGravity(1.0);
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
            wxToolBar* toolBar = CreateToolBar(wxTB_DEFAULT_STYLE | 
#if !defined _WIN32
				wxTB_NODIVIDER | 
#endif
				wxTB_FLAT);
            toolBar->SetMargins(2, 2);
            toolBar->AddRadioTool(CommandIds::Menu::EditDeactivateTool, "Default Tool", IO::loadImageResource("NoTool.png"), wxNullBitmap, "Disable Current Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleCreateComplexBrushTool, "Brush Tool", IO::loadImageResource("BrushTool.png"), wxNullBitmap, "Brush Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleClipTool, "Clip Tool", IO::loadImageResource("ClipTool.png"), wxNullBitmap, "Clip Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleVertexTool, "Vertex Tool", IO::loadImageResource("VertexTool.png"), wxNullBitmap, "Vertex Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleEdgeTool, "Edge Tool", IO::loadImageResource("EdgeTool.png"), wxNullBitmap, "Edge Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleFaceTool, "Face Tool", IO::loadImageResource("FaceTool.png"), wxNullBitmap, "Face Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleRotateObjectsTool, "Rotate Tool", IO::loadImageResource("RotateTool.png"), wxNullBitmap, "Rotate Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleScaleObjectsTool, "Scale Tool", IO::loadImageResource("ScaleTool.png"), wxNullBitmap, "Scale Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleShearObjectsTool, "Shear Tool", IO::loadImageResource("ShearTool.png"), wxNullBitmap, "Shear Tool");
            toolBar->AddSeparator();
            toolBar->AddTool(wxID_DUPLICATE, "Duplicate Objects", IO::loadImageResource("DuplicateObjects.png"), wxNullBitmap, wxITEM_NORMAL, "Duplicate Objects");
            toolBar->AddTool(CommandIds::Actions::FlipObjectsHorizontally, "Flip Horizontally", IO::loadImageResource("FlipHorizontally.png"), wxNullBitmap, wxITEM_NORMAL, "Flip Horizontally");
            toolBar->AddTool(CommandIds::Actions::FlipObjectsVertically, "Flip Vertically", IO::loadImageResource("FlipVertically.png"), wxNullBitmap, wxITEM_NORMAL, "Flip Vertically");
            toolBar->AddSeparator();
            toolBar->AddCheckTool(CommandIds::Menu::EditToggleTextureLock, "Texture Lock", textureLockBitmap(), wxNullBitmap, "Toggle Texture Lock");
            toolBar->AddCheckTool(CommandIds::Menu::EditToggleUVLock, "UV Lock", UVLockBitmap(), wxNullBitmap, "Toggle UV Lock");
            toolBar->AddSeparator();

            const wxString gridSizes[12] = { "Grid 0.125", "Grid 0.25", "Grid 0.5", "Grid 1", "Grid 2", "Grid 4", "Grid 8", "Grid 16", "Grid 32", "Grid 64", "Grid 128", "Grid 256" };
            m_gridChoice = new wxChoice(toolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, 12, gridSizes);
            m_gridChoice->SetSelection(indexForGridSize(m_document->grid().size()));
            toolBar->AddControl(m_gridChoice);
            
            toolBar->Realize();
        }
        
        void MapFrame::createStatusBar() {
            m_statusBar = CreateStatusBar();
        }
        
        static Model::AttributableNode* commonEntityForBrushList(const Model::BrushList& list) {
            if (list.empty())
                return nullptr;
            
            Model::AttributableNode* firstEntity = list.front()->entity();
            bool multipleEntities = false;
            
            for (const Model::Brush* brush : list) {
                if (brush->entity() != firstEntity) {
                    multipleEntities = true;
                }
            }

            if (multipleEntities) {
                return nullptr;
            } else {
                return firstEntity;
            }
        }
        
        static String commonClassnameForEntityList(const Model::EntityList& list) {
            if (list.empty())
                return "";
            
            const String firstClassname = list.front()->classname();
            bool multipleClassnames = false;
            
            for (const Model::Entity* entity : list) {
                if (entity->classname() != firstClassname) {
                    multipleClassnames = true;
                }
            }
            
            if (multipleClassnames) {
                return "";
            } else {
                return firstClassname;
            }
        }
        
        static String numberWithSuffix(size_t count, const String &singular, const String &plural) {
            return std::to_string(count) + " " + StringUtils::safePlural(count, singular, plural);
        }
        
        static wxString describeSelection(const MapDocument* document) {
            const wxString DblArrow = wxString(" ") + wxString(wxUniChar(0x00BB)) + wxString(" ");
            const wxString Arrow = wxString(" ") + wxString(wxUniChar(0x203A)) + wxString(" ");
            
            wxString result;
            
            // current layer
            result << document->currentLayer()->name() << DblArrow;
            
            // open groups
            std::list<Model::Group*> groups;
            for (Model::Group* group = document->currentGroup(); group != nullptr; group = group->group()) {
                groups.push_front(group);
            }
            for (Model::Group* group : groups) {
                result << group->name() << Arrow;
            }
            
            // build a vector of strings describing the things that are selected
            StringList tokens;
            
            const auto &selectedNodes = document->selectedNodes();
            
            // selected brushes
            if (!selectedNodes.brushes().empty()) {
                Model::AttributableNode *commonEntity = commonEntityForBrushList(selectedNodes.brushes());
                
                // if all selected brushes are from the same entity, print the entity name
                String token = numberWithSuffix(selectedNodes.brushes().size(), "brush", "brushes");
                if (commonEntity) {
                    token += " (" + commonEntity->classname() + ")";
                } else {
                    token += " (multiple entities)";
                }
                tokens.push_back(token);
            }

            // selected brush faces
            if (document->hasSelectedBrushFaces()) {
                const auto token = numberWithSuffix(document->selectedBrushFaces().size(), "face", "faces");
                tokens.push_back(token);
            }

            // entities
            if (!selectedNodes.entities().empty()) {
                String commonClassname = commonClassnameForEntityList(selectedNodes.entities());
                
                String token = numberWithSuffix(selectedNodes.entities().size(), "entity", "entities");
                if (commonClassname != "") {
                    token += " (" + commonClassname + ")";
                } else {
                    token += " (multiple classnames)";
                }
                tokens.push_back(token);
            }
            
            // groups
            if (!selectedNodes.groups().empty()) {
                tokens.push_back(numberWithSuffix(selectedNodes.groups().size(), "group", "groups"));
            }
            
            // layers
            if (!selectedNodes.layers().empty()) {
                tokens.push_back(numberWithSuffix(selectedNodes.layers().size(), "layer", "layers"));
            }
            
            if (tokens.empty()) {
                tokens.push_back("nothing");
            }
            
            // now, turn `tokens` into a comma-separated string
            result << StringUtils::join(tokens, ", ", ", and ", " and ") << " selected";
            
            return result;
        }
        
        void MapFrame::updateStatusBar() {
            m_statusBar->SetStatusText(describeSelection(m_document.get()));
        }
        
        void MapFrame::bindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapFrame::preferenceDidChange);

            m_document->documentWasClearedNotifier.addObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentModificationStateDidChangeNotifier.addObserver(this, &MapFrame::documentModificationStateDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &MapFrame::selectionDidChange);
            m_document->currentLayerDidChangeNotifier.addObserver(this, &MapFrame::currentLayerDidChange);
            m_document->groupWasOpenedNotifier.addObserver(this, &MapFrame::groupWasOpened);
            m_document->groupWasClosedNotifier.addObserver(this, &MapFrame::groupWasClosed);
            
            Grid& grid = m_document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapFrame::gridDidChange);
        }

        void MapFrame::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            assertResult(prefs.preferenceDidChangeNotifier.removeObserver(this, &MapFrame::preferenceDidChange));

            m_document->documentWasClearedNotifier.removeObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentModificationStateDidChangeNotifier.removeObserver(this, &MapFrame::documentModificationStateDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &MapFrame::selectionDidChange);
            m_document->currentLayerDidChangeNotifier.removeObserver(this, &MapFrame::currentLayerDidChange);
            m_document->groupWasOpenedNotifier.removeObserver(this, &MapFrame::groupWasOpened);
            m_document->groupWasClosedNotifier.removeObserver(this, &MapFrame::groupWasClosed);
            
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
            m_gridChoice->SetSelection(indexForGridSize(grid.size()));
        }
        
        void MapFrame::selectionDidChange(const Selection& selection) {
            updateStatusBar();
        }
        
        void MapFrame::currentLayerDidChange(const TrenchBroom::Model::Layer* layer) {
            updateStatusBar();
        }
        
        void MapFrame::groupWasOpened(Model::Group* group) {
            updateStatusBar();
        }
        
        void MapFrame::groupWasClosed(Model::Group* group) {
            updateStatusBar();
        }

        void MapFrame::bindEvents() {
            Bind(wxEVT_MENU, &MapFrame::OnFileSave, this, wxID_SAVE);
            Bind(wxEVT_MENU, &MapFrame::OnFileSaveAs, this, wxID_SAVEAS);
            Bind(wxEVT_MENU, &MapFrame::OnFileExportObj, this, CommandIds::Menu::FileExportObj);
            Bind(wxEVT_MENU, &MapFrame::OnFileLoadPointFile, this, CommandIds::Menu::FileLoadPointFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileReloadPointFile, this, CommandIds::Menu::FileReloadPointFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileUnloadPointFile, this, CommandIds::Menu::FileUnloadPointFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileLoadPortalFile, this, CommandIds::Menu::FileLoadPortalFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileReloadPortalFile, this, CommandIds::Menu::FileReloadPortalFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileUnloadPortalFile, this, CommandIds::Menu::FileUnloadPortalFile);
            Bind(wxEVT_MENU, &MapFrame::OnFileReloadTextureCollections, this, CommandIds::Menu::FileReloadTextureCollections);
            Bind(wxEVT_MENU, &MapFrame::OnFileReloadEntityDefinitions, this, CommandIds::Menu::FileReloadEntityDefinitions);
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
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleScaleObjectsTool, this, CommandIds::Menu::EditToggleScaleObjectsTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleShearObjectsTool, this, CommandIds::Menu::EditToggleShearObjectsTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleVertexTool, this, CommandIds::Menu::EditToggleVertexTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleEdgeTool, this, CommandIds::Menu::EditToggleEdgeTool);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleFaceTool, this, CommandIds::Menu::EditToggleFaceTool);

            Bind(wxEVT_MENU, &MapFrame::OnEditCsgConvexMerge, this, CommandIds::Menu::EditCsgConvexMerge);
            Bind(wxEVT_MENU, &MapFrame::OnEditCsgSubtract, this, CommandIds::Menu::EditCsgSubtract);
            Bind(wxEVT_MENU, &MapFrame::OnEditCsgIntersect, this, CommandIds::Menu::EditCsgIntersect);
            Bind(wxEVT_MENU, &MapFrame::OnEditCsgHollow, this, CommandIds::Menu::EditCsgHollow);
            
            Bind(wxEVT_MENU, &MapFrame::OnEditReplaceTexture, this, CommandIds::Menu::EditReplaceTexture);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleTextureLock, this, CommandIds::Menu::EditToggleTextureLock);
            Bind(wxEVT_MENU, &MapFrame::OnEditToggleUVLock, this, CommandIds::Menu::EditToggleUVLock);
            Bind(wxEVT_MENU, &MapFrame::OnEditSnapVerticesToInteger, this, CommandIds::Menu::EditSnapVerticesToInteger);
            Bind(wxEVT_MENU, &MapFrame::OnEditSnapVerticesToGrid, this, CommandIds::Menu::EditSnapVerticesToGrid);

            Bind(wxEVT_MENU, &MapFrame::OnViewToggleShowGrid, this, CommandIds::Menu::ViewToggleShowGrid);
            Bind(wxEVT_MENU, &MapFrame::OnViewToggleSnapToGrid, this, CommandIds::Menu::ViewToggleSnapToGrid);
            Bind(wxEVT_MENU, &MapFrame::OnViewIncGridSize, this, CommandIds::Menu::ViewIncGridSize);
            Bind(wxEVT_MENU, &MapFrame::OnViewDecGridSize, this, CommandIds::Menu::ViewDecGridSize);
            Bind(wxEVT_MENU, &MapFrame::OnViewSetGridSize, this, CommandIds::Menu::ViewSetGridSize0Point125, CommandIds::Menu::ViewSetGridSize256);

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

            Bind(wxEVT_MENU, &MapFrame::OnRunCompile, this, CommandIds::Menu::RunCompile);
            Bind(wxEVT_MENU, &MapFrame::OnRunLaunch, this, CommandIds::Menu::RunLaunch);
            
            Bind(wxEVT_MENU, &MapFrame::OnDebugPrintVertices, this, CommandIds::Menu::DebugPrintVertices);
            Bind(wxEVT_MENU, &MapFrame::OnDebugCreateBrush, this, CommandIds::Menu::DebugCreateBrush);
            Bind(wxEVT_MENU, &MapFrame::OnDebugCreateCube, this, CommandIds::Menu::DebugCreateCube);
            Bind(wxEVT_MENU, &MapFrame::OnDebugClipBrush, this, CommandIds::Menu::DebugClipWithFace);
            Bind(wxEVT_MENU, &MapFrame::OnDebugCopyJSShortcutMap, this, CommandIds::Menu::DebugCopyJSShortcuts);
            Bind(wxEVT_MENU, &MapFrame::OnDebugCrash, this, CommandIds::Menu::DebugCrash);
            Bind(wxEVT_MENU, &MapFrame::OnDebugThrowExceptionDuringCommand, this, CommandIds::Menu::DebugThrowExceptionDuringCommand);
            Bind(wxEVT_MENU, &MapFrame::OnDebugSetWindowSize, this, CommandIds::Menu::DebugSetWindowSize);

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

#if defined(_WIN32)
            Bind(wxEVT_ACTIVATE, &MapFrame::OnActivate, this);
#endif

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

        void MapFrame::OnFileExportObj(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            exportDocumentAsObj();
        }

        void MapFrame::OnFileLoadPointFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            wxString defaultDir;
            if (!m_document->path().isEmpty())
                defaultDir = m_document->path().deleteLastComponent().asString();
            wxFileDialog browseDialog(this, "Load Point File", defaultDir, wxEmptyString, "Point files (*.pts)|*.pts|Any files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

            if (browseDialog.ShowModal() == wxID_OK)
                m_document->loadPointFile(IO::Path(browseDialog.GetPath().ToStdString()));
        }

        void MapFrame::OnFileReloadPointFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canReloadPointFile()) {
                m_document->reloadPointFile();
            }
        }

        void MapFrame::OnFileUnloadPointFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (canUnloadPointFile())
                m_document->unloadPointFile();
        }
        
        void MapFrame::OnFileLoadPortalFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            wxString defaultDir;
            if (!m_document->path().isEmpty()) {
                defaultDir = m_document->path().deleteLastComponent().asString();
            }
            wxFileDialog browseDialog(this, "Load Portal File", defaultDir, wxEmptyString, "Portal files (*.prt)|*.prt|Any files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            
            if (browseDialog.ShowModal() == wxID_OK) {
                m_document->loadPortalFile(IO::Path(browseDialog.GetPath().ToStdString()));
            }
        }

        void MapFrame::OnFileReloadPortalFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canReloadPortalFile()) {
                m_document->reloadPortalFile();
            }
        }

        void MapFrame::OnFileUnloadPortalFile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canUnloadPortalFile()) {
                m_document->unloadPortalFile();
            }
        }

        void MapFrame::OnFileReloadTextureCollections(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->reloadTextureCollections();
        }

        void MapFrame::OnFileReloadEntityDefinitions(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->reloadEntityDefinitions();
        }

        void MapFrame::OnFileClose(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Close();
        }

        void MapFrame::OnEditUndo(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (canUndo()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                undo();
            }
        }

        void MapFrame::OnEditRedo(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (canRedo()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                redo();
            }
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

            if (canCut()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                copyToClipboard();
                Transaction transaction(m_document, "Cut");
                m_document->deleteObjects();
            }
        }

        void MapFrame::OnEditCopy(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canCopy()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                copyToClipboard();
            }
        }

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

            if (canPaste()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                const vm::bbox3 referenceBounds = m_document->referenceBounds();
                Transaction transaction(m_document);
                if (paste() == PT_Node && m_document->hasSelectedNodes()) {
                    const vm::bbox3 bounds = m_document->selectionBounds();
                    const vm::vec3 delta = m_mapView->pasteObjectsDelta(bounds, referenceBounds);
                    m_document->translateObjects(delta);
                }
            }
        }

        void MapFrame::OnEditPasteAtOriginalPosition(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canPaste()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                paste();
            }
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

            if (canDelete()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                if (m_mapView->clipToolActive())
                    m_mapView->clipTool()->removeLastPoint();
                else if (m_mapView->vertexToolActive())
                    m_mapView->vertexTool()->removeSelection();
                else if (m_mapView->edgeToolActive())
                    m_mapView->edgeTool()->removeSelection();
                else if (m_mapView->faceToolActive())
                    m_mapView->faceTool()->removeSelection();
                else if (!m_mapView->anyToolActive())
                    m_document->deleteObjects();
            }
        }

        void MapFrame::OnEditDuplicate(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDuplicate()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->duplicateObjects();
            }
        }

        void MapFrame::OnEditSelectAll(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectAllNodes();
            }
        }

        void MapFrame::OnEditSelectSiblings(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelectSiblings()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectSiblings();
            }
        }

        void MapFrame::OnEditSelectTouching(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelectByBrush()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectTouching(true);
            }
        }

        void MapFrame::OnEditSelectInside(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelectByBrush()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectInside(true);
            }
        }

        void MapFrame::OnEditSelectTall(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canSelectTall()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->selectTall();
            }
        }

        void MapFrame::OnEditSelectByLineNumber(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSelect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
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

            if (canDeselect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->deselectAll();
            }
        }

        void MapFrame::OnEditGroupSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canGroup()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                const String name = queryGroupName(this);
                if (!name.empty())
                    m_document->groupSelection(name);
            }
        }

        void MapFrame::OnEditUngroupSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canUngroup()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->ungroupSelection();
            }
        }

        void MapFrame::OnEditReplaceTexture(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            ReplaceTextureDialog dialog(this, m_document, *m_contextManager);
            dialog.CenterOnParent();
            dialog.ShowModal();
        }

        void MapFrame::OnEditDeactivateTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_mapView->deactivateTool();
        }

        void MapFrame::OnEditToggleCreateComplexBrushTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleCreateComplexBrushTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleCreateComplexBrushTool();
            }
        }

        void MapFrame::OnEditToggleClipTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleClipTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleClipTool();
            }
        }

        void MapFrame::OnEditToggleRotateObjectsTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleRotateObjectsTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleRotateObjectsTool();
            }
        }

        void MapFrame::OnEditToggleScaleObjectsTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleScaleObjectsTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleScaleObjectsTool();
            }
        }

        void MapFrame::OnEditToggleShearObjectsTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleShearObjectsTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleShearObjectsTool();
            }
        }
        
        void MapFrame::OnEditToggleVertexTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleVertexTools()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleVertexTool();
            }
        }
        
        void MapFrame::OnEditToggleEdgeTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleVertexTools()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleEdgeTool();
            }
        }
        
        void MapFrame::OnEditToggleFaceTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canToggleVertexTools()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleFaceTool();
            }
        }

        void MapFrame::OnEditCsgConvexMerge(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDoCsgConvexMerge()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                if (m_mapView->vertexToolActive()) {
                    m_mapView->vertexTool()->csgConvexMerge();
                } else if (m_mapView->edgeToolActive()) {
                    m_mapView->edgeTool()->csgConvexMerge();
                } else if (m_mapView->faceToolActive()) {
                    m_mapView->faceTool()->csgConvexMerge();
                } else {
                    m_document->csgConvexMerge();
                }
            }
        }

        void MapFrame::OnEditCsgSubtract(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canDoCsgSubtract()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->csgSubtract();
            }
        }

        void MapFrame::OnEditCsgIntersect(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canDoCsgIntersect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->csgIntersect();
            }
        }

        void MapFrame::OnEditCsgHollow(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canDoCsgHollow()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->csgHollow();
            }
        }

        void MapFrame::OnEditToggleTextureLock(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            PreferenceManager::instance().set(Preferences::TextureLock, !pref(Preferences::TextureLock));
            PreferenceManager::instance().saveChanges();
            
            GetToolBar()->SetToolNormalBitmap(CommandIds::Menu::EditToggleTextureLock, textureLockBitmap());
        }

        wxBitmap MapFrame::textureLockBitmap() {
            if (pref(Preferences::TextureLock)) {
                return IO::loadImageResource("TextureLockOn.png");
            } else {
                return IO::loadImageResource("TextureLockOff.png");
            }
        }

        void MapFrame::OnEditToggleUVLock(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            PreferenceManager::instance().set(Preferences::UVLock, !pref(Preferences::UVLock));
            PreferenceManager::instance().saveChanges();

            GetToolBar()->SetToolNormalBitmap(CommandIds::Menu::EditToggleUVLock, UVLockBitmap());
        }

        wxBitmap MapFrame::UVLockBitmap() {
            if (pref(Preferences::UVLock)) {
                return IO::loadImageResource("UVLockOn.png");
            } else {
                return IO::loadImageResource("UVLockOff.png");
            }
        }

        void MapFrame::OnEditSnapVerticesToInteger(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canSnapVertices()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->snapVertices(1u);
            }
        }
        
        void MapFrame::OnEditSnapVerticesToGrid(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canSnapVertices()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->snapVertices(m_document->grid().actualSize());
            }
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

            if (canIncGridSize()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->grid().incSize();
            }
        }

        void MapFrame::OnViewDecGridSize(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canDecGridSize()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->grid().decSize();
            }
        }

        void MapFrame::OnViewSetGridSize(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->grid().setSize(gridSizeForMenuId(event.GetId()));
        }

        void MapFrame::OnViewMoveCameraToNextPoint(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canMoveCameraToNextPoint()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->moveCameraToNextTracePoint();
            }
        }

        void MapFrame::OnViewMoveCameraToPreviousPoint(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canMoveCameraToPreviousPoint()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->moveCameraToPreviousTracePoint();
            }
        }

        void MapFrame::OnViewFocusCameraOnSelection(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (canFocusCamera()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->focusCameraOnSelection(true);
            }
        }

        void MapFrame::OnViewMoveCameraToPosition(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxTextEntryDialog dialog(this, "Enter a position (x y z) for the camera.", "Move Camera", "0.0 0.0 0.0");
            if (dialog.ShowModal() == wxID_OK) {
                const wxString str = dialog.GetValue();
                const vm::vec3 position = vm::vec3::parse(str.ToStdString());
                m_mapView->moveCameraToPosition(position, true);
            }
        }
        
        void MapFrame::OnViewHideSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canHide()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->hideSelection();
            }
        }
        
        void MapFrame::OnViewIsolateSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (canIsolate()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->isolate(m_document->selectedNodes().nodes());
            }
        }
        
        void MapFrame::OnViewShowHiddenObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            m_document->showAll();
        }

        void MapFrame::OnViewSwitchToMapInspector(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            switchToInspectorPage(Inspector::InspectorPage_Map);
        }

        void MapFrame::OnViewSwitchToEntityInspector(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            switchToInspectorPage(Inspector::InspectorPage_Entity);
        }

        void MapFrame::OnViewSwitchToFaceInspector(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            switchToInspectorPage(Inspector::InspectorPage_Face);
        }

        void MapFrame::switchToInspectorPage(const Inspector::InspectorPage page) {
            ensureInspectorVisible();
            m_inspector->switchToPage(page);
        }

        void MapFrame::ensureInspectorVisible() {
            if (m_hSplitter->isMaximized(m_vSplitter))
                m_hSplitter->restore();
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

        void MapFrame::OnRunCompile(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            if (m_compilationDialog == nullptr) {
                m_compilationDialog = new CompilationDialog(this);
                m_compilationDialog->Show();
            } else {
                m_compilationDialog->Raise();
            }
        }

        void MapFrame::compilationDialogWillClose() {
            m_compilationDialog = nullptr;
        }

        void MapFrame::OnRunLaunch(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            LaunchGameEngineDialog dialog(this, m_document);
            dialog.ShowModal();
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
                std::vector<vm::vec3> positions;
                vm::vec3::parseAll(str.ToStdString(), std::back_inserter(positions));
                m_document->createBrush(positions);
            }
        }

        void MapFrame::OnDebugCreateCube(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            wxTextEntryDialog dialog(this, "Enter bounding box size", "Create Cube", "");
            if (dialog.ShowModal() == wxID_OK) {
                const auto str = dialog.GetValue();
                double size; str.ToDouble(&size);
                const vm::bbox3 bounds(size / 2.0);
                const auto posArray = bounds.vertices();
                const auto posList = std::vector<vm::vec3>(std::begin(posArray), std::end(posArray));
                m_document->createBrush(posList);
            }
        }
        
        void MapFrame::OnDebugClipBrush(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            wxTextEntryDialog dialog(this, "Enter face points ( x y z ) ( x y z ) ( x y z )", "Clip Brush", "");
            if (dialog.ShowModal() == wxID_OK) {
                const wxString str = dialog.GetValue();
                std::vector<vm::vec3> points;
                vm::vec3::parseAll(str.ToStdString(), std::back_inserter(points));
                assert(points.size() == 3);
                m_document->clipBrushes(points[0], points[1], points[2]);
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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
        static void debugSegfault() {
            volatile void *test = nullptr;
            printf("%p\n", *((void **)test));
        }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

        static void debugException() {
            Exception e;
            throw e;
        }

        void MapFrame::OnDebugCrash(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            wxString crashTypes[2] = { "Null pointer dereference", "Unhandled exception" };

            wxSingleChoiceDialog d(nullptr, "Choose a crash type", "Crash", 2, crashTypes);
            if (d.ShowModal() == wxID_OK) {
                const int idx = d.GetSelection();
                if (idx == 0) {
                    debugSegfault();
                } else if (idx == 1) {
                    debugException();
                }
            }
        }

        void MapFrame::OnDebugThrowExceptionDuringCommand(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->throwExceptionDuringCommand();
        }

        void MapFrame::OnDebugSetWindowSize(wxCommandEvent& event) {
            wxTextEntryDialog dialog(this, "Enter Size (W H)", "Window Size", "1920 1080");
            if (dialog.ShowModal() == wxID_OK) {
                const auto str = dialog.GetValue();
                const auto size = vm::vec2i::parse(str.ToStdString());
                SetSize(size.x(), size.y());
            }
        }

        void MapFrame::OnFlipObjectsHorizontally(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canFlipObjects()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->flipObjects(vm::direction::left);
            }
        }

        void MapFrame::OnFlipObjectsVertically(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_mapView->canFlipObjects()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->flipObjects(vm::direction::up);
            }
        }

        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const ActionManager& actionManager = ActionManager::instance();

            switch (event.GetId()) {
                case wxID_OPEN:
                case wxID_SAVE:
                case wxID_SAVEAS:
                case wxID_CLOSE:
                case CommandIds::Menu::FileExportObj:
                case CommandIds::Menu::FileOpenRecent:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::FileLoadPointFile:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::FileReloadPointFile:
                    event.Enable(canReloadPointFile());
                    break;
                case CommandIds::Menu::FileUnloadPointFile:
                    event.Enable(canUnloadPointFile());
                    break;
                case CommandIds::Menu::FileLoadPortalFile:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::FileReloadPortalFile:
                    event.Enable(canReloadPortalFile());
                    break;
                case CommandIds::Menu::FileUnloadPortalFile:
                    event.Enable(canUnloadPortalFile());
                    break;
                case CommandIds::Menu::FileReloadTextureCollections:
                case CommandIds::Menu::FileReloadEntityDefinitions:
                    event.Enable(true);
                    break;
                case wxID_UNDO: {
                    const ActionMenuItem* item = actionManager.findMenuItem(wxID_UNDO);
                    ensure(item != nullptr, "item is null");
                    if (canUndo()) {
                        event.Enable(true);
                        const auto textEdit = findFocusedTextCtrl() != nullptr;
                        const auto name = textEdit ? "" : m_document->lastCommandName();
                        event.SetText(item->menuString(name, m_mapView->viewportHasFocus()));
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
                        const auto textEdit = findFocusedTextCtrl() != nullptr;
                        const auto name = textEdit ? "" : m_document->nextCommandName();
                        event.SetText(item->menuString(name, m_mapView->viewportHasFocus()));
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
                case CommandIds::Menu::EditToggleScaleObjectsTool:
                    event.Check(m_mapView->scaleObjectsToolActive());
                    event.Enable(m_mapView->canToggleScaleObjectsTool());
                    break;
                case CommandIds::Menu::EditToggleShearObjectsTool:
                    event.Check(m_mapView->shearObjectsToolActive());
                    event.Enable(m_mapView->canToggleShearObjectsTool());
                    break;
                case CommandIds::Menu::EditToggleVertexTool:
                    event.Check(m_mapView->vertexToolActive());
                    event.Enable(m_mapView->canToggleVertexTools());
                    break;
                case CommandIds::Menu::EditToggleEdgeTool:
                    event.Check(m_mapView->edgeToolActive());
                    event.Enable(m_mapView->canToggleVertexTools());
                    break;
                case CommandIds::Menu::EditToggleFaceTool:
                    event.Check(m_mapView->faceToolActive());
                    event.Enable(m_mapView->canToggleVertexTools());
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
                case CommandIds::Menu::EditCsgHollow:
                    event.Enable(canDoCsgHollow());
                    break;
                case CommandIds::Menu::EditSnapVerticesToInteger:
                case CommandIds::Menu::EditSnapVerticesToGrid:
                    event.Enable(canSnapVertices());
                    break;
                case CommandIds::Menu::EditReplaceTexture:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditToggleTextureLock:
                    event.Enable(true);
                    event.Check(pref(Preferences::TextureLock));
                    break;
                case CommandIds::Menu::EditToggleUVLock:
                    event.Enable(true);
                    event.Check(pref(Preferences::UVLock));
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
                case CommandIds::Menu::ViewSetGridSize0Point125:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == -3);
                    break;
                case CommandIds::Menu::ViewSetGridSize0Point25:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == -2);
                    break;
                case CommandIds::Menu::ViewSetGridSize0Point5:
                    event.Enable(true);
                    event.Check(m_document->grid().size() == -1);
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
                    event.Check(!m_vSplitter->isMaximized(m_mapView));
                    break;
                case CommandIds::Menu::ViewToggleInspector:
                    event.Enable(true);
                    event.Check(!m_hSplitter->isMaximized(m_vSplitter));
                    break;
                case CommandIds::Menu::RunCompile:
                    event.Enable(canCompile());
                    break;
                case CommandIds::Menu::RunLaunch:
                    event.Enable(canLaunch());
                    break;
                case CommandIds::Menu::DebugPrintVertices:
                case CommandIds::Menu::DebugCreateBrush:
                case CommandIds::Menu::DebugCreateCube:
                case CommandIds::Menu::DebugCopyJSShortcuts:
                case CommandIds::Menu::DebugCrash:
                case CommandIds::Menu::DebugThrowExceptionDuringCommand:
                case CommandIds::Menu::DebugSetWindowSize:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::DebugClipWithFace:
                    event.Enable(m_document->selectedNodes().hasOnlyBrushes());
                    break;
                case CommandIds::Actions::FlipObjectsHorizontally:
                case CommandIds::Actions::FlipObjectsVertically:
                    event.Enable(m_mapView->canFlipObjects());
                    break;
                default:
                    event.Enable(event.GetId() >= CommandIds::Menu::FileRecentDocuments && event.GetId() < CommandIds::Menu::FileRecentDocuments + 10);
                    break;
            }
        }

        void MapFrame::OnToolBarSetGridSize(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_document->grid().setSize(gridSizeForIndex(event.GetSelection()));
        }

        bool MapFrame::canUnloadPointFile() const {
            return m_document->isPointFileLoaded();
        }

        bool MapFrame::canReloadPointFile() const {
            return m_document->canReloadPointFile();
        }

        bool MapFrame::canUnloadPortalFile() const {
            return m_document->isPortalFileLoaded();
        }

        bool MapFrame::canReloadPortalFile() const {
            return m_document->canReloadPortalFile();
        }

        bool MapFrame::canUndo() const {
            auto textCtrl = findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                return true; // textCtrl->CanUndo();
            } else {
                return m_document->canUndoLastCommand();
            }
        }

        void MapFrame::undo() {
            assert(canUndo());

            auto textCtrl = findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                textCtrl->Undo();
            } else {
                if (!m_mapView->cancelMouseDrag() && !m_inspector->cancelMouseDrag()) {
                    m_document->undoLastCommand();
                }
            }
        }

        bool MapFrame::canRedo() const {
            auto textCtrl = findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                return true; // textCtrl->CanRedo();
            } else {
                return m_document->canRedoNextCommand();
            }
        }

        void MapFrame::redo() {
            assert(canRedo());

            auto textCtrl = findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                textCtrl->Redo();
            } else {
                m_document->redoNextCommand();
            }
        }

        wxTextCtrl* MapFrame::findFocusedTextCtrl() const {
            return dynamic_cast<wxTextCtrl*>(FindFocus());
        }

        bool MapFrame::canCut() const {
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canCopy() const {
            return m_document->hasSelectedNodes() || m_document->hasSelectedBrushFaces();
        }

        bool MapFrame::canPaste() const {
            if (!m_mapView->isCurrent())
                return false;
            
            OpenClipboard openClipboard;
            return wxTheClipboard->IsOpened() && wxTheClipboard->IsSupported(wxDF_TEXT);
        }

        bool MapFrame::canDelete() const {
            if (m_mapView->clipToolActive())
                return m_mapView->clipTool()->canRemoveLastPoint();
            else if (m_mapView->vertexToolActive())
                return m_mapView->vertexTool()->canRemoveSelection();
            else if (m_mapView->edgeToolActive())
                return m_mapView->edgeTool()->canRemoveSelection();
            else if (m_mapView->faceToolActive())
                return m_mapView->faceTool()->canRemoveSelection();
            else
                return canCut();
        }

        bool MapFrame::canDuplicate() const {
            return m_document->hasSelectedNodes();
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
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canUngroup() const {
            return m_document->selectedNodes().hasOnlyGroups() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canHide() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canIsolate() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canDoCsgConvexMerge() const {
            return (m_document->hasSelectedBrushFaces() && m_document->selectedBrushFaces().size() > 1) ||
                   (m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1) ||
                   (m_mapView->vertexToolActive() && m_mapView->vertexTool()->canDoCsgConvexMerge()) ||
                   (m_mapView->edgeToolActive() && m_mapView->edgeTool()->canDoCsgConvexMerge()) ||
                   (m_mapView->faceToolActive() && m_mapView->faceTool()->canDoCsgConvexMerge());
        }

        bool MapFrame::canDoCsgSubtract() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() >= 1;
        }

        bool MapFrame::canDoCsgIntersect() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1;
        }

        bool MapFrame::canDoCsgHollow() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() >= 1;
        }

        bool MapFrame::canSnapVertices() const {
            return m_document->selectedNodes().hasOnlyBrushes();
        }

        bool MapFrame::canDecGridSize() const {
            return m_document->grid().size() > Grid::MinSize;
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

        bool MapFrame::canCompile() const {
            return m_document->persistent();
        }

        bool MapFrame::canLaunch() const {
            return m_document->persistent();
        }

        void MapFrame::OnClose(wxCloseEvent& event) {
            if (!IsBeingDeleted()) {
                if (m_compilationDialog != nullptr && !m_compilationDialog->Close()) {
                    event.Veto();
                } else {
                    ensure(m_frameManager != nullptr, "frameManager is null");
                    if (event.CanVeto() && !confirmOrDiscardChanges())
                        event.Veto();
                    else
                        m_frameManager->removeAndDestroyFrame(this);
                }
            }
        }

        void MapFrame::OnAutosaveTimer(wxTimerEvent& event) {
            if (IsBeingDeleted()) return;

            m_autosaver->triggerAutosave(logger());
        }
        
        int MapFrame::indexForGridSize(const int gridSize) {
            return gridSize - Grid::MinSize;
        }
        
        int MapFrame::gridSizeForIndex(const int index) {
            const int size = index + Grid::MinSize;
            assert(size <= Grid::MaxSize);
            assert(size >= Grid::MinSize);
            return size;
        }
        
        int MapFrame::gridSizeForMenuId(const int menuId) {
            const int size = menuId - CommandIds::Menu::ViewSetGridSize1;
            assert(size <= Grid::MaxSize);
            assert(size >= Grid::MinSize);
            return size;
        }
    }
}
