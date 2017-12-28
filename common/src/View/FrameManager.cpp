/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FrameManager.h"

#include "Exceptions.h"
#include "Macros.h"
#include "View/AboutDialog.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/MapFrame.h"

#include <cassert>

#include <wx/app.h>
#include <wx/display.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>

namespace TrenchBroom {
    namespace View {
        FrameManager::FrameManager(const bool singleFrame) :
        m_singleFrame(singleFrame) {}

        FrameManager::~FrameManager() {
            closeAllFrames(true);
        }

        MapFrame* FrameManager::newFrame() {
            return createOrReuseFrame();
        }

        FrameList FrameManager::frames() const {
            return m_frames;
        }

        MapFrame *FrameManager::topFrame() const {
            return m_frames.empty() ? nullptr : m_frames.front();
        }

        bool FrameManager::closeAllFrames() {
            return closeAllFrames(false);
        }

        bool FrameManager::allFramesClosed() const {
            return m_frames.empty();
        }

        void FrameManager::OnFrameActivate(wxActivateEvent& event) {
            if (event.GetActive()) {
                MapFrame* frame = static_cast<MapFrame*>(event.GetEventObject());

                FrameList::iterator it = std::find(std::begin(m_frames), std::end(m_frames), frame);
                assert(it != std::end(m_frames));
                if (it != std::begin(m_frames)) {
                    assert(topFrame() != frame);
                    m_frames.erase(it);
                    m_frames.push_front(frame);
                }
            }
            event.Skip();
        }

        MapFrame* FrameManager::createOrReuseFrame() {
            assert(!m_singleFrame || m_frames.size() <= 1);
            if (!m_singleFrame || m_frames.empty()) {
                MapDocumentSPtr document = MapDocumentCommandFacade::newMapDocument();
                createFrame(document);
            }
            return topFrame();
        }

        MapFrame* FrameManager::createFrame(MapDocumentSPtr document) {
            MapFrame* frame = new MapFrame(this, document);
            frame->SetName("MapFrame");
            frame->positionOnScreen(topFrame());

            if (m_frames.empty())
                wxPersistenceManager::Get().RegisterAndRestore(frame);
            else
                wxPersistenceManager::Get().Register(frame);

            m_frames.push_front(frame);

            frame->Bind(wxEVT_ACTIVATE, &FrameManager::OnFrameActivate, this);
            frame->Show();
            frame->Raise();
            return frame;
        }

        bool FrameManager::closeAllFrames(const bool force) {
            MapFrame* lastFrame = nullptr;
            unused(lastFrame);
            while (!m_frames.empty()) {
                MapFrame* frame = m_frames.back();
                assert(frame != lastFrame);
                if (!frame->Close(force))
                    return false;
            }
            assert(m_frames.empty());
            return true;
        }

        void FrameManager::removeAndDestroyFrame(MapFrame* frame) {
            FrameList::iterator it = std::find(std::begin(m_frames), std::end(m_frames), frame);
            if (it == std::end(m_frames))
                // On OS X, we sometimes get two close events for a frame when terminating the app from the dock.
                return;

            m_frames.erase(it);

            if (m_frames.empty() || wxTheApp->GetExitOnFrameDelete())
                AboutDialog::closeAboutDialog();

            frame->Unbind(wxEVT_ACTIVATE, &FrameManager::OnFrameActivate, this);
            frame->Destroy();
        }
    }
}
