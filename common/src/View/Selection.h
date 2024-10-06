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

#pragma once

#include "mdl/BrushFaceHandle.h"

#include <vector>

namespace tb::mdl
{
class Node;
}

namespace tb::View
{

class Selection
{
private:
  std::vector<mdl::Node*> m_selectedNodes;
  std::vector<mdl::Node*> m_deselectedNodes;
  std::vector<mdl::BrushFaceHandle> m_selectedBrushFaces;
  std::vector<mdl::BrushFaceHandle> m_deselectedBrushFaces;

public:
  const std::vector<mdl::Node*>& selectedNodes() const;
  const std::vector<mdl::Node*>& deselectedNodes() const;
  const std::vector<mdl::BrushFaceHandle>& selectedBrushFaces() const;
  const std::vector<mdl::BrushFaceHandle>& deselectedBrushFaces() const;

  void addSelectedNodes(const std::vector<mdl::Node*>& nodes);
  void addDeselectedNodes(const std::vector<mdl::Node*>& nodes);
  void addSelectedBrushFaces(const std::vector<mdl::BrushFaceHandle>& faces);
  void addDeselectedBrushFaces(const std::vector<mdl::BrushFaceHandle>& faces);
};

} // namespace tb::View
