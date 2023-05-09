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

#include "OnePaneMapView.h"

#include <QGridLayout>

#include "View/CyclingMapView.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

namespace TrenchBroom
{
namespace View
{
OnePaneMapView::OnePaneMapView(
  Logger* logger,
  std::weak_ptr<MapDocument> document,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager,
  QWidget* parent)
  : MultiMapView(parent)
  , m_logger(logger)
  , m_document(document)
  , m_mapView(nullptr)
{
  createGui(toolBox, mapRenderer, contextManager);
}

void OnePaneMapView::createGui(
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& mapRenderer,
  GLContextManager& contextManager)
{
  m_mapView = new CyclingMapView(
    m_document,
    toolBox,
    mapRenderer,
    contextManager,
    CyclingMapView::View_ALL,
    m_logger,
    this);
  m_mapView->linkCamera(m_linkHelper);
  addMapView(m_mapView);

  auto* layout = new QGridLayout();
  layout->addWidget(m_mapView, 0, 0, 1, 1);
  setLayout(layout);
}
} // namespace View
} // namespace TrenchBroom
