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
  AppController& appController,
  MapDocument& document,
  MapViewToolBox& toolBox,
  gl::ContextManager& contextManager,
  QWidget* parent)
  : MultiPaneMapView{parent}
  , m_document{document}
{
  createGui(appController, toolBox, contextManager);
}

void OnePaneMapView::createGui(
  AppController& appController,
  MapViewToolBox& toolBox,
  gl::ContextManager& contextManager)
{
  m_mapView = new CyclingMapView{
    appController, m_document, toolBox, contextManager, CyclingMapView::View_ALL};
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
