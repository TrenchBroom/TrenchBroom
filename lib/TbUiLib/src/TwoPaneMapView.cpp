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

#include "ui/TwoPaneMapView.h"

#include <QHBoxLayout>

#include "ui/CyclingMapView.h"
#include "ui/MapDocument.h"
#include "ui/MapView3D.h"
#include "ui/Splitter.h"
#include "ui/WidgetState.h"

#include "kd/contracts.h"

namespace tb::ui
{
TwoPaneMapView::TwoPaneMapView(
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

TwoPaneMapView::~TwoPaneMapView()
{
  saveWidgetState(m_splitter);
}

void TwoPaneMapView::createGui(
  AppController& appController,
  MapViewToolBox& toolBox,
  gl::ContextManager& contextManager)
{
  // See comment in CyclingMapView::createGui
  m_splitter = new Splitter{DrawKnob::No};
  m_splitter->setObjectName("TwoPaneMapView_Splitter");

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  setLayout(layout);
  layout->addWidget(m_splitter);

  m_mapView3D = new MapView3D{appController, m_document, toolBox, contextManager};
  m_mapView2D = new CyclingMapView{
    appController, m_document, toolBox, contextManager, CyclingMapView::View_2D};

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

  restoreWidgetState(m_splitter);
}

void TwoPaneMapView::maximizeView(MapView* view)
{
  contract_pre(view == m_mapView2D || view == m_mapView3D);

  if (view == m_mapView2D)
  {
    m_mapView3D->hide();
  }
  if (view == m_mapView3D)
  {
    m_mapView2D->hide();
  }
}

void TwoPaneMapView::restoreViews()
{
  m_mapView3D->show();
  m_mapView2D->show();
}

} // namespace tb::ui
