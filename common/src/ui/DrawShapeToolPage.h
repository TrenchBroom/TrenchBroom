/*
 Copyright (C) 2023 Kristian Duske

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

#include <QWidget>

#include "Notifier.h"
#include "NotifierConnection.h"

class QStackedLayout;
class QToolButton;

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{
class DrawShapeToolExtensionManager;

class DrawShapeToolPage : public QWidget
{
  Q_OBJECT
private:
  mdl::Map& m_map;
  DrawShapeToolExtensionManager& m_extensionManager;

  QToolButton* m_extensionButton = nullptr;
  QStackedLayout* m_extensionPages = nullptr;

  NotifierConnection m_notifierConnection;

public:
  Notifier<> applyParametersNotifier;

  explicit DrawShapeToolPage(
    mdl::Map& map,
    DrawShapeToolExtensionManager& extensionManager,
    QWidget* parent = nullptr);

private:
  void createGui();
  void currentExtensionDidChange(size_t index);
};

} // namespace ui
} // namespace tb
