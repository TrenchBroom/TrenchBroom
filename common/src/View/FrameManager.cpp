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

#include <QApplication>

namespace TrenchBroom {
    namespace View {
        FrameManager::FrameManager(const bool singleFrame) :
        QObject(),
        m_singleFrame(singleFrame) {
            connect(qApp, &QApplication::focusChanged, this, &FrameManager::onFocusChange);
        }

        FrameManager::~FrameManager() = default;

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

        void FrameManager::onFocusChange(QWidget* old, QWidget* now) {
            if (now == nullptr) {
                return;
            }

            // The QApplication::focusChanged signal also notifies us of focus changes between child widgets, so
            // get the top-level widget with QWidget::window()
            auto* frame = dynamic_cast<MapFrame*>(now->window());
            if (frame != nullptr) {
                auto it = std::find(std::begin(m_frames), std::end(m_frames), frame);
                assert(it != std::end(m_frames));
                if (it != std::begin(m_frames)) {
                    assert(topFrame() != frame);
                    m_frames.erase(it);
                    m_frames.push_front(frame);
                }
            }
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
            // FIXME: SetName is something for wx persistence?
            //frame->SetName("MapFrame");
            frame->positionOnScreen(topFrame());

#if 0
            if (m_frames.empty())
                wxPersistenceManager::Get().RegisterAndRestore(frame);
            else
                wxPersistenceManager::Get().Register(frame);
#endif
            m_frames.push_front(frame);

            frame->show();
            frame->raise();
            return frame;
        }

        bool FrameManager::closeAllFrames(const bool force) {
            auto framesCopy = m_frames;
            for (MapFrame* frame : framesCopy) {
                if (!frame->close()) {
                    return false;
                }
            }
            assert(m_frames.empty());
            return true;
        }

        void FrameManager::removeAndDestroyFrame(MapFrame* frame) {
            // this is called from MapFrame::~MapFrame

            FrameList::iterator it = std::find(std::begin(m_frames), std::end(m_frames), frame);
            if (it == std::end(m_frames)) {
                // On OS X, we sometimes get two close events for a frame when terminating the app from the dock.
                return;
            }

            m_frames.erase(it);

            if (m_frames.empty() || qApp->quitOnLastWindowClosed()) {
                AboutDialog::closeAboutDialog();
            }

            // MapFrame uses Qt::WA_DeleteOnClose so we don't delete it here
        }
    }
}
