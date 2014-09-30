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
#include "IO/DiskFileSystem.h"
#include "Renderer/MapRenderer.h"
#include "View/Autosaver.h"
#include "View/CachingLogger.h"
#include "View/Console.h"
#include "View/MapDocument.h"
#include "View/MapView3D.h"

#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/timer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MapFrame::MapFrame() :
        wxFrame(NULL, wxID_ANY, "MapFrame"),
        m_frameManager(NULL),
        m_mapRenderer(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_console(NULL) {}
        
        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        wxFrame(NULL, wxID_ANY, "MapFrame"),
        m_frameManager(NULL),
        m_mapRenderer(NULL),
        m_autosaver(NULL),
        m_autosaveTimer(NULL),
        m_console(NULL) {
            Create(frameManager, document);
        }
        
        void MapFrame::Create(FrameManager* frameManager, MapDocumentSPtr document) {
            assert(frameManager != NULL);
            assert(document != NULL);
            
            m_frameManager = frameManager;
            m_document = document;
            m_mapRenderer = new Renderer::MapRenderer(m_document);
            m_autosaver = new Autosaver(m_document);

            createGui();
            
            m_document->setParentLogger(logger());

            m_autosaveTimer = new wxTimer(this);
            m_autosaveTimer->Start(1000);
            
            bindObservers();
        }
        
        MapFrame::~MapFrame() {
            unbindObservers();
            
            // this might lead to crashes because my children, including the map views, will be
            // deleted after the map renderer is deleted
            // possible solution: force deletion of all children here?
            
            delete m_mapRenderer;
            m_mapRenderer = NULL;
            
            delete m_autosaveTimer;
            m_autosaveTimer = NULL;
            
            delete m_autosaver;
            m_autosaver = NULL;
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
        
        bool MapFrame::newDocument(Model::GamePtr game, const Model::MapFormat::Type mapFormat) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->newDocument(MapDocument::DefaultWorldBounds, game, mapFormat);
            return true;
        }
        
        bool MapFrame::openDocument(Model::GamePtr game, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->loadDocument(MapDocument::DefaultWorldBounds, game, path);
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
                wxFileDialog saveDialog(this, "Save map file", "", "", "Map files (*.map)|*.map", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
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
            SetTitle(wxString(m_document->filename()) + wxString(m_document->modified() ? "*" : ""));
#endif
            SetRepresentedFilename(m_document->path().asString());
        }

        void MapFrame::createGui() {
            m_console = new Console(this);
            MapView3D* mapView = new MapView3D(this, m_console, m_document, *m_mapRenderer);
            
            wxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
            frameSizer->Add(mapView, 1, wxEXPAND);
            frameSizer->Add(m_console, 0, wxEXPAND);
            frameSizer->SetItemMinSize(m_console, wxSize(wxDefaultCoord, 200));
            
            SetSizer(frameSizer);
        }

        void MapFrame::bindObservers() {
            m_document->documentWasClearedNotifier.addObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.addObserver(this, &MapFrame::documentDidChange);
        }
        
        void MapFrame::unbindObservers() {
            m_document->documentWasClearedNotifier.removeObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.removeObserver(this, &MapFrame::documentDidChange);
        }

        void MapFrame::documentWasCleared(View::MapDocument* document) {
            updateTitle();
        }
        
        void MapFrame::documentDidChange(View::MapDocument* document) {
            updateTitle();
            View::TrenchBroomApp::instance().updateRecentDocument(m_document->path());
        }

        void MapFrame::OnAutosaveTimer(wxTimerEvent& event) {
            m_autosaver->triggerAutosave(logger());
        }
    }
}
