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

#pragma once

#include "Model/Polyhedron3.h"
#include "View/CreateBrushesToolBase.h"

#include <memory>

namespace TrenchBroom::View
{

class AssembleBrushTool : public CreateBrushesToolBase
{
private:
  std::unique_ptr<Model::Polyhedron3> m_polyhedron;

public:
  explicit AssembleBrushTool(std::weak_ptr<MapDocument> document);

  const Model::Polyhedron3& polyhedron() const;
  void update(const Model::Polyhedron3& polyhedron);

private:
  bool doActivate() override;
  bool doDeactivate() override;
  void doBrushesWereCreated() override;
};

} // namespace TrenchBroom::View
