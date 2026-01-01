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

#include "ui/Tool.h"
#include "ui/ToolController.h"

#include <memory>

namespace tb::ui
{
class GestureTracker;
class MapDocument;

/**
 * Functionality summary:
 *
 * Modifier combinations:
 * - Alt:       transfer material and alignment from selected
 * - Alt+Shift: transfer material and alignment (rotation method) from selected
 * - Alt+Ctrl:  transfer material (but not alignment) from selected
 *
 * Actions:
 * - LMB Click: applies to clicked faces
 * - LMB Drag: applies to all faces dragged over
 * - LMB Double click: applies to all faces of target brush
 */
class SetBrushFaceAttributesTool : public ToolController, public Tool
{
private:
  MapDocument& m_document;

public:
  explicit SetBrushFaceAttributesTool(MapDocument& document);

private:
  Tool& tool() override;
  const Tool& tool() const override;

  bool mouseClick(const InputState& inputState) override;
  bool mouseDoubleClick(const InputState& inputState) override;

  bool cancel() override;

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override;

  void copyAttributesFromSelection(const InputState& inputState, bool applyToBrush);
  bool canCopyAttributesFromSelection(const InputState& inputState) const;
};

} // namespace tb::ui
