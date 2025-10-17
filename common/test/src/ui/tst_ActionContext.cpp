/*
 Copyright (C) 2022 Kristian Duske

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

#include "ui/ActionContext.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{

TEST_CASE("actionContextMatches_WithMask")
{
  using T =
    std::tuple<ActionContext::Type, ActionContext::Type, ActionContext::Type, bool>;
  const auto [lhs, rhs, mask, expected] = GENERATE(values<T>({
    // clang-format off
    {ActionContext::View3D,     ActionContext::View2D,  ActionContext::AnyView, false},
    {ActionContext::View3D,     ActionContext::View3D,  ActionContext::AnyView, true},
    {ActionContext::View3D,     ActionContext::View3D,  ActionContext::AnyTool, false},
    {ActionContext::AnyView,    ActionContext::View2D,  ActionContext::AnyView, true},
    {ActionContext::AnyView,    ActionContext::View3D,  ActionContext::AnyView, true},
    {ActionContext::AnyView,    ActionContext::View3D,  ActionContext::AnyTool, false},
    {ActionContext::RotateTool, ActionContext::Any,     ActionContext::AnyTool, true},
    {ActionContext::View3D,     ActionContext::AnyTool, ActionContext::AnyTool, false},
    // clang-format on
  }));

  CAPTURE(actionContextName(lhs), actionContextName(rhs), actionContextName(mask));

  CHECK(actionContextMatches(lhs, rhs, mask) == expected);
}

TEST_CASE("actionContextMatches")
{
  using T = std::tuple<ActionContext::Type, ActionContext::Type, bool>;
  const auto [lhs, rhs, expected] = GENERATE(values<T>({
    // clang-format off
    // required context                                                                 actual context
    {ActionContext::Any,                                                                ActionContext::View2D | ActionContext::NoSelection | ActionContext::NoTool,       true},
    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       true},
    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     true},
    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   true},
    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, true},

    {ActionContext::View2D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       false},
    {ActionContext::View2D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     false},
    {ActionContext::View2D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::View2D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, false},

    {ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       true},
    {ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     false},
    {ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,        ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, false},

    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::NoTool,       ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       true},
    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::NoTool,       ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     false},
    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::NoTool,       ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::NoTool,       ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, false},

    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       true},
    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     true},
    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   true},
    {ActionContext::Any,                                                                ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, true},

    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::NoTool,     ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::NoTool,     ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     true},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::NoTool,     ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::NoTool,     ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, false},

    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::NoTool,      ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       false},
    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::NoTool,      ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     true},
    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::NoTool,      ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::NoTool,      ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, false},

    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::RotateTool,   ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       false},
    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::RotateTool,   ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     false},
    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::RotateTool,   ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   true},
    {ActionContext::AnyView | ActionContext::NoSelection | ActionContext::RotateTool,   ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, false},

    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, true},

    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::RotateTool,  ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       false},
    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::RotateTool,  ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     false},
    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::RotateTool,  ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::AnyView | ActionContext::AnySelection | ActionContext::RotateTool,  ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, true},

    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyTool,    ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool,       false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyTool,    ActionContext::View3D | ActionContext::NodeSelection | ActionContext::NoTool,     false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyTool,    ActionContext::View3D | ActionContext::NoSelection | ActionContext::RotateTool,   false},
    {ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyTool,    ActionContext::View3D | ActionContext::NodeSelection | ActionContext::RotateTool, true},
    // clang-format on
  }));

  CAPTURE(actionContextName(lhs), actionContextName(rhs));

  CHECK(actionContextMatches(lhs, rhs) == expected);
}

} // namespace tb::ui
