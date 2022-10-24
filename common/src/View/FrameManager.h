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

#pragma once

#include <QObject>

#include <memory>
#include <vector>

namespace TrenchBroom
{
namespace View
{
class MapDocument;
class MapFrame;

class FrameManager : public QObject
{
  Q_OBJECT
private:
  bool m_singleFrame;
  std::vector<MapFrame*> m_frames;

public:
  explicit FrameManager(bool singleFrame);
  ~FrameManager() override;

  MapFrame* newFrame();
  bool closeAllFrames();

  std::vector<MapFrame*> frames() const;
  MapFrame* topFrame() const;
  bool allFramesClosed() const;

private:
  void onFocusChange(QWidget* old, QWidget* now);
  MapFrame* createOrReuseFrame();
  MapFrame* createFrame(std::shared_ptr<MapDocument> document);
  void removeFrame(MapFrame* frame);

  friend class MapFrame;
};
} // namespace View
} // namespace TrenchBroom
