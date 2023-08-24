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

#include "TwoPaneMapView.h"

#include <QHBoxLayout>

#include "View/CyclingMapView.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView3D.h"
#include "View/QtUtils.h"
#include "View/Splitter.h"

namespace TrenchBroom::View
{
TwoPaneMapView::TwoPaneMapView(
  std::weak_ptr<MapDocument> document,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager,
  Logger* logger,
  QWidget* parent)
  : MultiPaneMapView{parent}
  , m_logger{logger}
  , m_document(std::move(document))
{
  createGui(toolBox, mapRenderer, contextManager);
}

TwoPaneMapView::~TwoPaneMapView()
{
  saveWindowState(m_splitter);
}

void TwoPaneMapView::createGui(
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager)
{

  // See comment in CyclingMapView::createGui
  m_splitter = new Splitter{};
  m_splitter->setObjectName("TwoPaneMapView_Splitter");

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  setLayout(layout);
  layout->addWidget(m_splitter);

  m_mapView3D = new MapView3D{m_document, toolBox, mapRenderer, contextManager, m_logger};
  m_mapView2D = new CyclingMapView{
    m_document, toolBox, mapRenderer, contextManager, CyclingMapView::View_2D, m_logger};

  m_mapView3D->linkCamera(m_linkHelper);
  m_mapView2D->linkCamera(m_linkHelper);

  addMapView(m_mapView3D);
  addMapView(m_mapView2D);

  m_splitter->addWidget(m_mapView3D);
  m_splitter->addWidget(m_mapView2D);

  // Configure minimum child sizes and initial splitter position at 50%
  m_mapView2D->setMinimumSize(100, 100);
  m_mapView3D->setMinimumSize(100, 100);
  m_splitter->setSizes(QList<int>{1, 1});

  restoreWindowState(m_splitter);
}

void TwoPaneMapView::doMaximizeView(MapView* view)
{
  assert(view == m_mapView2D || view == m_mapView3D);
  if (view == m_mapView2D)
  {
    m_mapView3D->hide();
  }
  if (view == m_mapView3D)
  {
    m_mapView2D->hide();
  }
}

void TwoPaneMapView::doRestoreViews()
{
  m_mapView3D->show();
  m_mapView2D->show();
}
} // namespace TrenchBroom::View
