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

#include "mdl/HitType.h"
#include "ui/Tool.h"
#include "ui/ToolController.h"

namespace tb
{
namespace mdl
{
class PickResult;
}

namespace render
{
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{
class GestureTracker;
class UVViewHelper;

class UVOriginTool : public ToolController, public Tool
{
public:
  static const mdl::HitType::Type XHandleHitType;
  static const mdl::HitType::Type YHandleHitType;

  static const double MaxPickDistance;
  static const float OriginHandleRadius;

private:
  UVViewHelper& m_helper;

public:
  explicit UVOriginTool(UVViewHelper& helper);

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, mdl::PickResult& pickResult) override;

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override;

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;

  bool cancel() override;
};

} // namespace ui
} // namespace tb
