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

#include "ui/ActionContext.h"

namespace tb
{
namespace mdl
{
class Map;
} // namespace mdl

namespace ui
{
class MapDocument;
class MapFrame;
class MapViewBase;

class ActionExecutionContext
{
private:
  ActionContext::Type m_actionContext;
  MapFrame* m_frame;
  MapViewBase* m_mapView;

public:
  ActionExecutionContext(MapFrame* mapFrame, MapViewBase* mapView);

  bool hasDocument() const;
  bool hasActionContext(ActionContext::Type actionContext) const;

  const MapFrame& frame() const;
  MapFrame& frame();

  const MapViewBase& view() const;
  MapViewBase& view();

  const MapDocument& document() const;
  MapDocument& document();

  mdl::Map& map();
  const mdl::Map& map() const;
};

} // namespace ui
} // namespace tb
