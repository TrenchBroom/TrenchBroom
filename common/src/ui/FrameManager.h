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

#pragma once

#include <QObject>

#include "Result.h"

#include "vm/bbox.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb
{
namespace mdl
{
enum class MapFormat;

struct EnvironmentConfig;
struct GameInfo;
} // namespace mdl

namespace ui
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

  Result<void> createDocument(
    const mdl::EnvironmentConfig& environmentConfig,
    const mdl::GameInfo& gameInfo,
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    kdl::task_manager& taskManager);

  Result<void> loadDocument(
    const mdl::EnvironmentConfig& environmentConfig,
    const mdl::GameInfo& gameInfo,
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    std::filesystem::path path,
    kdl::task_manager& taskManager);

  bool closeAllFrames();

  std::vector<MapFrame*> frames() const;
  MapFrame* topFrame() const;
  bool allFramesClosed() const;

private:
  void onFocusChange(QWidget* old, QWidget* now);

  bool shouldCreateFrameForDocument() const;
  MapFrame* createFrame(std::unique_ptr<MapDocument> document);
  void removeFrame(MapFrame* frame);

  friend class MapFrame;
};

} // namespace ui
} // namespace tb
