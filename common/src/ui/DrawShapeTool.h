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

#include "NotifierConnection.h"
#include "ui/CreateBrushesToolBase.h"
#include "ui/DrawShapeToolExtension.h"

namespace tb::ui
{

class DrawShapeTool : public CreateBrushesToolBase
{
private:
  NotifierConnection m_notifierConnection;

public:
  explicit DrawShapeTool(mdl::Map& map);
  void update(const vm::bbox3d& bounds);

  bool cancel();

private:
  DrawShapeToolExtensionManager m_extensionManager;

  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace tb::ui
