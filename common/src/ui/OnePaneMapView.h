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

#include "ui/MultiPaneMapView.h"

namespace tb::ui
{
class CyclingMapView;
class GLContextManager;
class MapDocument;
class MapViewToolBox;

class OnePaneMapView : public MultiPaneMapView
{
private:
  MapDocument& m_document;

  CyclingMapView* m_mapView = nullptr;

public:
  explicit OnePaneMapView(
    MapDocument& document,
    MapViewToolBox& toolBox,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);

private:
  void createGui(MapViewToolBox& toolBox, GLContextManager& contextManager);

private: // implement MultiPaneMapView subclassing interface
  void maximizeView(MapView* view) override;
  void restoreViews() override;
};

} // namespace tb::ui
