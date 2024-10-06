/*
 Copyright (C) 2010 Kristian Duske

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

#include <QApplication>

#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/MapFrame.h"

#include <cassert>
#include <memory>

namespace tb::View
{

FrameManager::FrameManager(const bool singleFrame)
  : m_singleFrame{singleFrame}
{
  connect(qApp, &QApplication::focusChanged, this, &FrameManager::onFocusChange);
}

FrameManager::~FrameManager() = default;

MapFrame* FrameManager::newFrame()
{
  return createOrReuseFrame();
}

std::vector<MapFrame*> FrameManager::frames() const
{
  return m_frames;
}

MapFrame* FrameManager::topFrame() const
{
  return m_frames.empty() ? nullptr : m_frames.front();
}

bool FrameManager::closeAllFrames()
{
  auto framesCopy = m_frames;
  for (auto* frame : framesCopy)
  {
    if (!frame->close())
    {
      return false;
    }
  }
  assert(m_frames.empty());
  return true;
}

bool FrameManager::allFramesClosed() const
{
  return m_frames.empty();
}

void FrameManager::onFocusChange(QWidget* /* old */, QWidget* now)
{
  if (now)
  {
    // The QApplication::focusChanged signal also notifies us of focus changes between
    // child widgets, so get the top-level widget with QWidget::window()
    if (auto* frame = dynamic_cast<MapFrame*>(now->window()))
    {
      if (auto it = std::find(m_frames.begin(), m_frames.end(), frame);
          it != m_frames.end() && it != m_frames.begin())
      {
        assert(topFrame() != frame);
        m_frames.erase(it);
        m_frames.insert(m_frames.begin(), frame);
      }
    }
  }
}

MapFrame* FrameManager::createOrReuseFrame()
{
  assert(!m_singleFrame || m_frames.size() <= 1);
  if (!m_singleFrame || m_frames.empty())
  {
    auto document = MapDocumentCommandFacade::newMapDocument();
    createFrame(std::move(document));
  }
  return topFrame();
}

MapFrame* FrameManager::createFrame(std::shared_ptr<MapDocument> document)
{
  auto* frame = new MapFrame{*this, std::move(document)};
  frame->positionOnScreen(topFrame());
  m_frames.insert(m_frames.begin(), frame);

  frame->show();
  frame->raise();
  return frame;
}

void FrameManager::removeFrame(MapFrame* frame)
{
  // This is called from MapFrame::closeEvent
  if (auto it = std::find(m_frames.begin(), m_frames.end(), frame); it != m_frames.end())
  {
    m_frames.erase(it);
    // MapFrame uses Qt::WA_DeleteOnClose so we don't need to delete it here
  }
}

} // namespace tb::View
