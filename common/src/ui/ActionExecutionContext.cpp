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

#include "ActionExecutionContext.h"

#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"

#include "kd/const_overload.h"
#include "kd/contracts.h"

namespace tb::ui
{

ActionExecutionContext::ActionExecutionContext(
  AppController& appController, MapFrame* mapFrame, MapViewBase* mapView)
  : m_actionContext(mapView != nullptr ? mapView->actionContext() : ActionContext::Any)
  , m_appController{appController}
  , m_frame{mapFrame}
  , m_mapView{mapView}
{
  contract_pre(m_frame == nullptr || m_mapView != nullptr);
}

bool ActionExecutionContext::hasDocument() const
{
  return m_frame != nullptr;
}

bool ActionExecutionContext::hasActionContext(
  const ActionContext::Type actionContext) const
{
  if (actionContext == ActionContext::Any || m_actionContext == ActionContext::Any)
  {
    return true;
  }

  if (hasDocument())
  {
    return actionContextMatches(m_actionContext, actionContext);
  }
  return false;
}

const AppController& ActionExecutionContext::appController() const
{
  return m_appController;
}

AppController& ActionExecutionContext::appController()
{
  return KDL_CONST_OVERLOAD(appController());
}

const MapFrame& ActionExecutionContext::frame() const
{
  contract_pre(hasDocument());

  return *m_frame;
}

MapFrame& ActionExecutionContext::frame()
{
  return KDL_CONST_OVERLOAD(frame());
}

const MapViewBase& ActionExecutionContext::view() const
{
  contract_pre(hasDocument());
  contract_pre(m_mapView != nullptr);

  return *m_mapView;
}

MapViewBase& ActionExecutionContext::view()
{
  return KDL_CONST_OVERLOAD(view());
}

const MapDocument& ActionExecutionContext::document() const
{
  return frame().document();
}

MapDocument& ActionExecutionContext::document()
{
  return KDL_CONST_OVERLOAD(document());
}

const mdl::Map& ActionExecutionContext::map() const
{
  return document().map();
}

mdl::Map& ActionExecutionContext::map()
{
  return KDL_CONST_OVERLOAD(map());
}

} // namespace tb::ui
