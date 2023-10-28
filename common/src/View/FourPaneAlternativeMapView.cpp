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

#include "FourPaneAlternativeMapView.h"

#include <QHBoxLayout>
#include <QSettings>

#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"
#include "View/QtUtils.h"
#include "View/Splitter.h"

namespace TrenchBroom::View
{
FourPaneAlternativeMapView::FourPaneAlternativeMapView(
  bool verticalLayout,
  std::weak_ptr<MapDocument> document,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager,
  Logger* logger,
  QWidget* parent)
  : MultiPaneMapView{parent}
  , m_logger{logger}
  , m_document{std::move(document)}
{
  createGui(verticalLayout, toolBox, mapRenderer, contextManager);
}

FourPaneAlternativeMapView::~FourPaneAlternativeMapView()
{
  saveWindowState(m_bigSplitter);
  saveWindowState(m_smallSplitter);
}

void FourPaneAlternativeMapView::createGui(
  bool verticalLayout,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager)
{
  m_bigSplitter = new Splitter{};
  m_bigSplitter->setObjectName("FourPaneAlternativeMapView_BigSplitter");

  m_smallSplitter = new Splitter{};
  m_smallSplitter->setObjectName("FourPaneAlternativeMapView_SmallSplitter");

  m_mapView3D = new MapView3D{m_document, toolBox, mapRenderer, contextManager, m_logger};
  m_mapViewXY = new MapView2D{
    m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY, m_logger};
  m_mapViewXZ = new MapView2D{
    m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XZ, m_logger};
  m_mapViewYZ = new MapView2D{
    m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_YZ, m_logger};

  m_mapView3D->linkCamera(m_linkHelper);
  m_mapViewXY->linkCamera(m_linkHelper);
  m_mapViewXZ->linkCamera(m_linkHelper);
  m_mapViewYZ->linkCamera(m_linkHelper);

  addMapView(m_mapView3D);
  addMapView(m_mapViewXY);
  addMapView(m_mapViewXZ);
  addMapView(m_mapViewYZ);

  // See comment in CyclingMapView::createGui
  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  setLayout(layout);
  layout->addWidget(m_bigSplitter);

  // left and right columns
  m_bigSplitter->addWidget(m_mapView3D);
  m_bigSplitter->addWidget(m_smallSplitter);

  // add children
  m_smallSplitter->addWidget(m_mapViewXY);
  m_smallSplitter->addWidget(m_mapViewXZ);
  m_smallSplitter->addWidget(m_mapViewYZ);

  // Configure minimum child sizes and initial splitter position at 50%
  m_mapView3D->setMinimumSize(100, 100);
  m_mapViewYZ->setMinimumSize(100, 100);
  m_mapViewXY->setMinimumSize(100, 100);
  m_mapViewXZ->setMinimumSize(100, 100);

  m_bigSplitter->setSizes(QList<int>{1, 1});
  m_smallSplitter->setSizes(QList<int>{1, 1, 1});

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

void FourPaneAlternativeMapView::doMaximizeView(MapView* view)
{
  assert(
    view == m_mapView3D || view == m_mapViewXY || view == m_mapViewXZ
    || view == m_mapViewYZ);

  m_mapView3D->hide();
  m_mapViewXY->hide();
  m_mapViewXZ->hide();
  m_mapViewYZ->hide();

  dynamic_cast<MapViewBase*>(view)->show();
}

void FourPaneAlternativeMapView::doRestoreViews()
{
  m_mapView3D->show();
  m_mapViewYZ->show();
  m_mapViewXY->show();
  m_mapViewXZ->show();
}
} // namespace TrenchBroom::View
