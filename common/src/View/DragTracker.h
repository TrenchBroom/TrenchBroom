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

namespace TrenchBroom
{
namespace Renderer
{
class RenderBatch;
class RenderContext;
} // namespace Renderer

namespace View
{
class InputState;

/**
 * Defines the protocol for handling mouse dragging in the tool system.
 */
class DragTracker
{
public:
  virtual ~DragTracker();

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
   * Called when a drag took place. This does not always have to correspond to a mouse
   * movement; sometimes these events are synthesized.
   */
  virtual bool drag(const InputState& inputState) = 0;

  /**
   * Called once at the end of a successful drag. Not called if the drag is cancelled.
   */
  virtual void end(const InputState& inputState) = 0;

  /**
   * Called once at the end of a canceled drag.
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
} // namespace View
} // namespace TrenchBroom
