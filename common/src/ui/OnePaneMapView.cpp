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

#include "OnePaneMapView.h"

#include <QGridLayout>

#include "ui/CyclingMapView.h"
#include "ui/MapDocument.h"

namespace tb::ui
{
OnePaneMapView::OnePaneMapView(
  MapDocument& document,
  MapViewToolBox& toolBox,
  render::MapRenderer& mapRenderer,
  GLContextManager& contextManager,
  QWidget* parent)
  : MultiPaneMapView{parent}
  , m_document{document}
{
  createGui(toolBox, mapRenderer, contextManager);
}

void OnePaneMapView::createGui(
  MapViewToolBox& toolBox,
  render::MapRenderer& mapRenderer,
  GLContextManager& contextManager)
{
  m_mapView = new CyclingMapView{
    m_document, toolBox, mapRenderer, contextManager, CyclingMapView::View_ALL};
  m_mapView->linkCamera(m_linkHelper);
  addMapView(m_mapView);

  auto* layout = new QGridLayout{};
  layout->addWidget(m_mapView, 0, 0, 1, 1);
  setLayout(layout);
}

void OnePaneMapView::maximizeView(MapView*)
{
  // nothing to do
}

void OnePaneMapView::restoreViews()
{
  // nothing to do
}

} // namespace tb::ui
