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

#include "PushSelection.h"

#include "mdl/Map.h"
#include "mdl/Selection.h"

#include <cassert>

namespace tb::mdl
{

PushSelection::PushSelection(Map& map)
  : m_map{map}
  , m_nodes{m_map.selection().nodes}
  , m_faces{m_map.selection().brushFaces}
{
}

PushSelection::~PushSelection()
{
  m_map.deselectAll();
  if (!m_nodes.empty() && m_map.selection().nodes != m_nodes)
  {
    m_map.selectNodes(m_nodes);
  }
  else if (!m_faces.empty() && m_map.selection().brushFaces != m_faces)
  {
    m_map.selectBrushFaces(m_faces);
  }
}

} // namespace tb::mdl
