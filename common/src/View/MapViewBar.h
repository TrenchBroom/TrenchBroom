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

#include "View/ContainerBar.h"

#include <memory>

class QStackedLayout;
class QLabel;

namespace TrenchBroom::View
{
class MapDocument;
class ViewPopupEditor;

class MapViewBar : public ContainerBar
{
  Q_OBJECT
private:
  QStackedLayout* m_toolBook = nullptr;
  ViewPopupEditor* m_viewEditor = nullptr;

public:
  explicit MapViewBar(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

  QStackedLayout* toolBook();

private:
  void createGui(std::weak_ptr<MapDocument> document);
};
} // namespace TrenchBroom::View
