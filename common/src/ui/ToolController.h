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

#include "ToolChain.h"
#include "ui/InputState.h"

#include <memory>
#include <string>

namespace tb::mdl
{
class Hit;
class PickResult;
} // namespace tb::mdl

namespace tb::render
{
class RenderBatch;
class RenderContext;
} // namespace tb::render

namespace tb::ui
{
class GestureTracker;
class DropTracker;
class InputState;
class Tool;

class ToolController
{
public:
  virtual ~ToolController();

  virtual Tool& tool() = 0;
  virtual const Tool& tool() const = 0;
  bool toolActive() const;

  virtual void pick(const InputState& inputState, mdl::PickResult& pickResult);

  virtual void modifierKeyChange(const InputState& inputState);

  virtual void mouseDown(const InputState& inputState);
  virtual void mouseUp(const InputState& inputState);
  virtual bool mouseClick(const InputState& inputState);
  virtual bool mouseDoubleClick(const InputState& inputState);
  virtual void mouseMove(const InputState& inputState);
  virtual void mouseScroll(const InputState& inputState);

  virtual std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState);
  virtual std::unique_ptr<GestureTracker> acceptGesture(const InputState& inputState);

  virtual bool shouldAcceptDrop(
    const InputState& inputState, const std::string& payload) const;
  virtual std::unique_ptr<DropTracker> acceptDrop(
    const InputState& inputState, const std::string& payload);

  virtual void setRenderOptions(
    const InputState& inputState, render::RenderContext& renderContext) const;
  virtual void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch);

  virtual bool cancel();

protected:
  void refreshViews();
};

class ToolControllerGroup : public ToolController
{
private:
  ToolChain m_chain;

public:
  ToolControllerGroup();
  ~ToolControllerGroup() override;

protected:
  void addController(std::unique_ptr<ToolController> controller);

public:
  void pick(const InputState& inputState, mdl::PickResult& pickResult) override;

  void modifierKeyChange(const InputState& inputState) override;

  void mouseDown(const InputState& inputState) override;
  void mouseUp(const InputState& inputState) override;
  bool mouseClick(const InputState& inputState) override;
  bool mouseDoubleClick(const InputState& inputState) override;
  void mouseMove(const InputState& inputState) override;
  void mouseScroll(const InputState& inputState) override;

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override;
  std::unique_ptr<DropTracker> acceptDrop(
    const InputState& inputState, const std::string& payload) override;

  void setRenderOptions(
    const InputState& inputState, render::RenderContext& renderContext) const override;
  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;

  bool cancel() override;

private: // subclassing interface
  virtual bool doShouldHandleMouseDrag(const InputState& inputState) const;
  virtual bool doShouldAcceptDrop(
    const InputState& inputState, const std::string& payload) const;
};

} // namespace tb::ui
