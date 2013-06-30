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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FrameManager.h"

#include "Exceptions.h"
#include "View/MapFrame.h"

#include <cassert>

#include <wx/display.h>

namespace TrenchBroom {
    namespace View {
        FrameManager::FrameManager(const bool singleFrame) :
        m_singleFrame(singleFrame),
        m_topFrame(NULL) {}
        
        FrameManager::~FrameManager() {
            closeAllFrames();
        }

        MapFrame* FrameManager::newFrame() {
            return createOrReuseFrame();
        }
        
        void FrameManager::closeAllFrames() {
            MapFrame* lastFrame = NULL;
            while (!m_frames.empty()) {
                MapFrame* frame = m_frames.front();
                assert(frame != lastFrame);
                frame->Close(true);
            }
            assert(m_frames.empty());
        }

        FrameList FrameManager::frames() const {
            return m_frames;
        }

        bool FrameManager::allFramesClosed() const {
            return m_frames.empty();
        }

        void FrameManager::OnFrameActivate(wxActivateEvent& event) {
            if (event.GetActive())
                m_topFrame = static_cast<MapFrame*>(event.GetEventObject());
            event.Skip();
        }

        MapFrame* FrameManager::createOrReuseFrame() {
            assert(!m_singleFrame || m_frames.size() <= 1);
            if (!m_singleFrame || m_frames.empty()) {
                MapDocument::Ptr document = MapDocument::newMapDocument();
                MapFrame* frame = createFrame(document);
                m_frames.push_back(frame);
            }
            return m_frames.back();
        }

        MapFrame* FrameManager::createFrame(MapDocument::Ptr document) {
            MapFrame* frame = new MapFrame(this, document);
            frame->positionOnScreen(m_topFrame);
            frame->Bind(wxEVT_ACTIVATE, &FrameManager::OnFrameActivate, this);
            frame->Show();
            frame->Raise();
            return frame;
        }

        void FrameManager::removeAndDestroyFrame(MapFrame* frame) {
            FrameList::iterator it = std::find(m_frames.begin(), m_frames.end(), frame);
            assert(it != m_frames.end());
            m_frames.erase(it);

            if (m_topFrame == frame)
                m_topFrame = NULL;

            frame->Unbind(wxEVT_ACTIVATE, &FrameManager::OnFrameActivate, this);
            frame->Destroy();
        }
    }
}
