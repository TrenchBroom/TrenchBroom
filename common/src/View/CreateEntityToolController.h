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

#include "View/ToolController.h"

#include <string>

namespace TrenchBroom
{
namespace View
{
class CreateEntityTool;

class CreateEntityToolController : public ToolController
{
protected:
  CreateEntityTool& m_tool;

protected:
  CreateEntityToolController(CreateEntityTool& tool);

public:
  virtual ~CreateEntityToolController() override;

private:
  Tool& tool() override;
  const Tool& tool() const override;

  std::unique_ptr<DropTracker> acceptDrop(
    const InputState& inputState, const std::string& payload) override;

  bool cancel() override;

private:
  virtual std::unique_ptr<DropTracker> createDropTracker(
    const InputState& inputState) const = 0;
};

class CreateEntityToolController2D : public CreateEntityToolController
{
public:
  CreateEntityToolController2D(CreateEntityTool& tool);

private:
  std::unique_ptr<DropTracker> createDropTracker(
    const InputState& inputState) const override;
};

class CreateEntityToolController3D : public CreateEntityToolController
{
public:
  CreateEntityToolController3D(CreateEntityTool& tool);

private:
  std::unique_ptr<DropTracker> createDropTracker(
    const InputState& inputState) const override;
};
} // namespace View
} // namespace TrenchBroom
