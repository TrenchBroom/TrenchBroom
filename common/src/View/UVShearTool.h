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

#include "Model/HitType.h"
#include "View/Tool.h"
#include "View/ToolController.h"

#include <memory>

namespace TrenchBroom
{
namespace View
{
class DragTracker;
class MapDocument;
class UVViewHelper;

class UVShearTool : public ToolController, public Tool
{
private:
  static const Model::HitType::Type XHandleHitType;
  static const Model::HitType::Type YHandleHitType;

private:
  std::weak_ptr<MapDocument> m_document;
  UVViewHelper& m_helper;

public:
  UVShearTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper);

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, Model::PickResult& pickResult) override;

  std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override;

  bool cancel() override;
};
} // namespace View
} // namespace TrenchBroom
