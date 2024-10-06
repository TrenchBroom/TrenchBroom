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

#include <string>

namespace tb::View
{

namespace ActionContext
{
using Type = size_t;
constexpr Type View3D = 1u << 0u;
constexpr Type View2D = 1u << 1u;
constexpr Type AnyView = View3D | View2D;
constexpr Type NoTool = 1u << 2u;
constexpr Type AssembleBrushTool = 1u << 3u;
constexpr Type ClipTool = 1u << 4u;
constexpr Type RotateTool = 1u << 5u;
constexpr Type ScaleTool = 1u << 6u;
constexpr Type ShearTool = 1u << 7u;
constexpr Type AnyVertexTool = 1u << 8u;
constexpr Type AnyTool =
  AnyVertexTool | AssembleBrushTool | ClipTool | RotateTool | ScaleTool | ShearTool;
constexpr Type AnyOrNoTool = AnyTool | NoTool;
constexpr Type NoSelection = 1u << 9u;
constexpr Type NodeSelection = 1u << 10u;
constexpr Type FaceSelection = 1u << 11u;
constexpr Type AnySelection = NodeSelection | FaceSelection;
constexpr Type AnyOrNoSelection = AnySelection | NoSelection;
constexpr Type Any = AnyView | AnyOrNoSelection | AnyOrNoTool;
} // namespace ActionContext

bool actionContextMatches(ActionContext::Type lhs, ActionContext::Type rhs);
bool actionContextMatches(
  ActionContext::Type lhs, ActionContext::Type rhs, ActionContext::Type mask);

std::string actionContextName(ActionContext::Type actionContext);

enum class ActionView
{
  Map2D = 0,
  Map3D = 1
};

static const size_t NumActionViews = 2;

} // namespace tb::View
