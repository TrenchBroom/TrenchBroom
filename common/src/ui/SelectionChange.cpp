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

#include "SelectionChange.h"

#include "kdl/vector_utils.h"

#include <vector>

namespace tb::ui
{

const std::vector<mdl::Node*>& SelectionChange::selectedNodes() const
{
  return m_selectedNodes;
}

const std::vector<mdl::Node*>& SelectionChange::deselectedNodes() const
{
  return m_deselectedNodes;
}

const std::vector<mdl::BrushFaceHandle>& SelectionChange::selectedBrushFaces() const
{
  return m_selectedBrushFaces;
}

const std::vector<mdl::BrushFaceHandle>& SelectionChange::deselectedBrushFaces() const
{
  return m_deselectedBrushFaces;
}

void SelectionChange::addSelectedNodes(const std::vector<mdl::Node*>& nodes)
{
  m_selectedNodes = kdl::vec_concat(std::move(m_selectedNodes), nodes);
}

void SelectionChange::addDeselectedNodes(const std::vector<mdl::Node*>& nodes)
{
  m_deselectedNodes = kdl::vec_concat(std::move(m_deselectedNodes), nodes);
}

void SelectionChange::addSelectedBrushFaces(
  const std::vector<mdl::BrushFaceHandle>& faces)
{
  m_selectedBrushFaces = kdl::vec_concat(std::move(m_selectedBrushFaces), faces);
}

void SelectionChange::addDeselectedBrushFaces(
  const std::vector<mdl::BrushFaceHandle>& faces)
{
  m_deselectedBrushFaces = kdl::vec_concat(std::move(m_deselectedBrushFaces), faces);
}

} // namespace tb::ui
