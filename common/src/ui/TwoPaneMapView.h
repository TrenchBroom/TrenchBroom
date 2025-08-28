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

class QSplitter;

namespace tb::render
{
class MapRenderer;
}

namespace tb::ui
{
class CyclingMapView;
class GLContextManager;
class MapDocument;
class MapView3D;
class MapViewToolBox;

class TwoPaneMapView : public MultiPaneMapView
{
  Q_OBJECT
private:
  MapDocument& m_document;

  QSplitter* m_splitter = nullptr;
  MapView3D* m_mapView3D = nullptr;
  CyclingMapView* m_mapView2D = nullptr;

public:
  TwoPaneMapView(
    MapDocument& document,
    MapViewToolBox& toolBox,
    render::MapRenderer& mapRenderer,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);
  ~TwoPaneMapView() override;

private:
  void createGui(
    MapViewToolBox& toolBox,
    render::MapRenderer& mapRenderer,
    GLContextManager& contextManager);

private: // implement MultiPaneMapView subclassing interface
  void maximizeView(MapView* view) override;
  void restoreViews() override;
};

} // namespace tb::ui
