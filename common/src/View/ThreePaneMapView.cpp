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

#include "ThreePaneMapView.h"

#include <QHBoxLayout>

#include "View/CyclingMapView.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"
#include "View/QtUtils.h"
#include "View/Splitter.h"

namespace TrenchBroom::View
{
ThreePaneMapView::ThreePaneMapView(
  bool verticalLayout,
  std::weak_ptr<MapDocument> document,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager,
  Logger* logger,
  QWidget* parent)
  : MultiPaneMapView(parent)
  , m_logger{logger}
  , m_document{std::move(document)}
{
  createGui(verticalLayout, toolBox, mapRenderer, contextManager);
}

ThreePaneMapView::~ThreePaneMapView()
{
  saveWindowState(m_bigSplitter);
  saveWindowState(m_smallSplitter);
}

void ThreePaneMapView::createGui(
  bool verticalLayout,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager)
{
  m_bigSplitter = new Splitter{};
  m_bigSplitter->setObjectName("ThreePaneMapView_HorizontalSplitter");

  m_smallSplitter = new Splitter{};
  m_smallSplitter->setObjectName("ThreePaneMapView_VerticalSplitter");

  m_mapView3D = new MapView3D{m_document, toolBox, mapRenderer, contextManager, m_logger};
  m_mapViewXY = new MapView2D{
    m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY, m_logger};
  m_mapViewZZ = new CyclingMapView{
    m_document, toolBox, mapRenderer, contextManager, CyclingMapView::View_ZZ, m_logger};

  m_mapView3D->linkCamera(m_linkHelper);
  m_mapViewXY->linkCamera(m_linkHelper);
  m_mapViewZZ->linkCamera(m_linkHelper);

  addMapView(m_mapView3D);
  addMapView(m_mapViewXY);
  addMapView(m_mapViewZZ);

  // See comment in CyclingMapView::createGui
  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  setLayout(layout);
  layout->addWidget(m_bigSplitter);

  // Add splitter children
  m_smallSplitter->addWidget(m_mapViewXY);
  m_smallSplitter->addWidget(m_mapViewZZ);

  m_bigSplitter->addWidget(m_mapView3D);
  m_bigSplitter->addWidget(m_smallSplitter);

  // Configure minimum child sizes and initial splitter position at 50%
  m_mapViewXY->setMinimumSize(100, 100);
  m_mapViewZZ->setMinimumSize(100, 100);
  m_mapView3D->setMinimumSize(100, 100);

  m_bigSplitter->setSizes(QList<int>{1, 1});
  m_smallSplitter->setSizes(QList<int>{1, 1});

  restoreWindowState(m_bigSplitter);
  restoreWindowState(m_smallSplitter);

  if (verticalLayout)
  {
    m_bigSplitter->setOrientation(Qt::Horizontal);
    m_smallSplitter->setOrientation(Qt::Vertical);
  }
  else
  {
    m_bigSplitter->setOrientation(Qt::Vertical);
    m_smallSplitter->setOrientation(Qt::Horizontal);
  }
}

void ThreePaneMapView::doMaximizeView(MapView* view)
{
  assert(view == m_mapView3D || view == m_mapViewXY || view == m_mapViewZZ);
  if (view == m_mapView3D)
  {
    m_smallSplitter->hide();
  }
  else if (view == m_mapViewXY)
  {
    m_mapViewZZ->hide();
    m_mapView3D->hide();
  }
  else if (view == m_mapViewZZ)
  {
    m_mapViewXY->hide();
    m_mapView3D->hide();
  }
}

void ThreePaneMapView::doRestoreViews()
{
  for (int i = 0; i < 2; ++i)
  {
    m_bigSplitter->widget(i)->show();
    m_smallSplitter->widget(i)->show();
  }
}
} // namespace TrenchBroom::View
