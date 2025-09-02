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

#include "mdl/Polyhedron3.h"
#include "ui/CreateBrushesToolBase.h"

namespace tb::ui
{

class AssembleBrushTool : public CreateBrushesToolBase
{
private:
  std::unique_ptr<mdl::Polyhedron3> m_polyhedron;

public:
  explicit AssembleBrushTool(mdl::Map& map);

  const mdl::Polyhedron3& polyhedron() const;
  void update(const mdl::Polyhedron3& polyhedron);

private:
  bool doActivate() override;
  bool doDeactivate() override;
  void doBrushesWereCreated() override;
};

} // namespace tb::ui
