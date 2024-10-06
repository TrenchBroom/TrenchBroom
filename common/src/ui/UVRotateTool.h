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

#include <memory>

namespace tb::mdl
{
class PickResult;
}

namespace tb::render
{
class RenderBatch;
class RenderContext;
} // namespace tb::render

namespace tb::ui
{
class GestureTracker;
class MapDocument;
class UVViewHelper;

class UVRotateTool : public ToolController, public Tool
{
public:
  static const mdl::HitType::Type AngleHandleHitType;

private:
  std::weak_ptr<MapDocument> m_document;
  UVViewHelper& m_helper;

public:
  UVRotateTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper);

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

} // namespace tb::ui
