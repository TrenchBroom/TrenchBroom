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

#include "View/Tool.h"
#include "View/ToolController.h"

#include <memory>

namespace TrenchBroom
{
namespace Renderer
{
class OrthographicCamera;
}

namespace View
{
class GestureTracker;

class UVCameraTool : public ToolController, public Tool
{
private:
  Renderer::OrthographicCamera& m_camera;

public:
  explicit UVCameraTool(Renderer::OrthographicCamera& camera);

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void mouseScroll(const InputState& inputState) override;

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override;

  bool cancel() override;
};
} // namespace View
} // namespace TrenchBroom
