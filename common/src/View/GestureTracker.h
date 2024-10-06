/*
 Copyright (C) 2021 Kristian Duske

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

namespace tb::Renderer
{
class RenderBatch;
class RenderContext;
} // namespace tb::Renderer

namespace tb::View
{
class InputState;

/**
 * Defines the protocol for handling mouse dragging in the tool system.
 */
class GestureTracker
{
public:
  virtual ~GestureTracker();

  /**
   * Called when a modifier key is pressed or released. The given input state represents
   * the state after the key was pressed or released.
   */
  virtual void modifierKeyChange(const InputState& inputState);

  /**
   * Called when the mouse wheel is scrolled.
   */
  virtual void mouseScroll(const InputState& inputState);

  /**
   * Called when a gesture is updated. Sometimes these events are synthesized.
   */
  virtual bool update(const InputState& inputState) = 0;

  /**
   * Called once at the end of a successful gesture. Not called if the gesture is
   * cancelled.
   */
  virtual void end(const InputState& inputState) = 0;

  /**
   * Called once at the end of a canceled gesture.
   */
  virtual void cancel() = 0;

  /**
   * Called prior to a rendering pass.
   */
  virtual void setRenderOptions(
    const InputState& inputState, Renderer::RenderContext& renderContext) const;

  /**
   * Called once during every render pass.
   */
  virtual void render(
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const;
};

} // namespace tb::View
