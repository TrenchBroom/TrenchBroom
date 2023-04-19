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

#include <string>

namespace TrenchBroom
{
namespace View
{
namespace ActionContext
{
using Type = size_t;
static const Type View3D = 1u << 0u;
static const Type View2D = 1u << 1u;
static const Type AnyView = View3D | View2D;
static const Type NoTool = 1u << 2u;
static const Type CreateComplexBrushTool = 1u << 3u;
static const Type ClipTool = 1u << 4u;
static const Type RotateTool = 1u << 5u;
static const Type ScaleTool = 1u << 6u;
static const Type ShearTool = 1u << 7u;
static const Type AnyVertexTool = 1u << 8u;
static const Type CreatePrimitiveBrushTool = 1u << 9u;
static const Type AnyTool =
  AnyVertexTool | CreateComplexBrushTool | ClipTool | RotateTool | ScaleTool | ShearTool | CreatePrimitiveBrushTool;
static const Type AnyOrNoTool = AnyTool | NoTool;
static const Type NoSelection = 1u << 9u;
static const Type NodeSelection = 1u << 10u;
static const Type FaceSelection = 1u << 11u;
static const Type AnySelection = NodeSelection | FaceSelection;
static const Type AnyOrNoSelection = AnySelection | NoSelection;
static const Type Any = AnyView | AnyOrNoSelection | AnyOrNoTool;
} // namespace ActionContext

bool actionContextMatches(ActionContext::Type lhs, ActionContext::Type rhs);
bool actionContextMatches(
  ActionContext::Type lhs, ActionContext::Type rhs, ActionContext::Type mask);

std::string actionContextName(ActionContext::Type actionContext);

typedef enum
{
  ActionView_Map2D = 0,
  ActionView_Map3D = 1
} ActionView;

static const size_t NumActionViews = 2;
} // namespace View
} // namespace TrenchBroom
