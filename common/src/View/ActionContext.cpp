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

#include "ActionContext.h"

#include <kdl/string_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom
{
namespace View
{
bool actionContextMatches(ActionContext::Type lhs, ActionContext::Type rhs)
{
  return actionContextMatches(lhs, rhs, ActionContext::AnyView)
         && actionContextMatches(lhs, rhs, ActionContext::AnyOrNoTool)
         && actionContextMatches(lhs, rhs, ActionContext::AnyOrNoSelection);
}

bool actionContextMatches(
  const ActionContext::Type lhs,
  const ActionContext::Type rhs,
  const ActionContext::Type mask)
{
  return (lhs & rhs & mask) != 0;
}

std::string actionContextName(const ActionContext::Type actionContext)
{
  if (actionContext == ActionContext::Any)
  {
    return "any";
  }

  std::vector<std::string> actionContexts;
  if ((actionContext & ActionContext::AnyView) == ActionContext::AnyView)
  {
    actionContexts.emplace_back("any view");
  }
  else if (actionContext & ActionContext::View3D)
  {
    actionContexts.emplace_back("3D view");
  }
  else if (actionContext & ActionContext::View2D)
  {
    actionContexts.emplace_back("2D view");
  }

  if (
    (actionContext & ActionContext::AnyOrNoSelection) == ActionContext::AnyOrNoSelection)
  {
    actionContexts.emplace_back("any or no selection");
  }
  else if (
    (actionContext & ActionContext::AnyOrNoSelection) == ActionContext::AnySelection)
  {
    actionContexts.emplace_back("any selection");
  }
  else
  {
    if (actionContext & ActionContext::NoSelection)
    {
      actionContexts.emplace_back("no selection");
    }
    if (actionContext & ActionContext::NodeSelection)
    {
      actionContexts.emplace_back("objects selected");
    }
    if (actionContext & ActionContext::FaceSelection)
    {
      actionContexts.emplace_back("faces selected");
    }
  }

  if ((actionContext & ActionContext::AnyOrNoTool) == ActionContext::AnyOrNoTool)
  {
    actionContexts.emplace_back("any or no tool");
  }
  else if ((actionContext & ActionContext::AnyOrNoTool) == ActionContext::AnyTool)
  {
    actionContexts.emplace_back("any tool");
  }
  else
  {
    if (actionContext & ActionContext::NoTool)
    {
      actionContexts.emplace_back("no tool");
    }
    if (actionContext & ActionContext::CreateComplexBrushTool)
    {
      actionContexts.emplace_back("brush tool");
    }
    if (actionContext & ActionContext::CreatePrimitiveBrushTool)
    {
      actionContexts.emplace_back("primitive brush tool");
    }
    if (actionContext & ActionContext::ClipTool)
    {
      actionContexts.emplace_back("clip tool");
    }
    if (actionContext & ActionContext::RotateTool)
    {
      actionContexts.emplace_back("rotate tool");
    }
    if (actionContext & ActionContext::AnyVertexTool)
    {
      actionContexts.emplace_back("any vertex tool");
    }
  }

  return kdl::str_join(actionContexts, ", ");
}
} // namespace View
} // namespace TrenchBroom
