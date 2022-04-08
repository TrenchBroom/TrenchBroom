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

#include "PushSelection.h"

#include "Model/MapFacade.h"
#include "Model/NodeCollection.h"

#include <cassert>

namespace TrenchBroom {
namespace Model {
PushSelection::PushSelection(MapFacade* facade)
  : m_facade(nullptr) {
  initialize(facade);
}

PushSelection::PushSelection(MapFacade& facade)
  : PushSelection(&facade) {}

PushSelection::~PushSelection() {
  m_facade->deselectAll();
  if (!m_nodes.empty())
    m_facade->selectNodes(m_nodes);
  else if (!m_faces.empty())
    m_facade->selectBrushFaces(m_faces);
}

void PushSelection::initialize(MapFacade* facade) {
  assert(facade != nullptr);
  m_facade = facade;
  m_nodes = m_facade->selectedNodes().nodes();
  m_faces = m_facade->selectedBrushFaces();
}
} // namespace Model
} // namespace TrenchBroom
