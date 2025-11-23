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

#include "Macros.h"
#include "ui/Tool.h"

#include <memory>

namespace tb
{
namespace mdl
{
class BrushNode;
class Grid;
class Map;
} // namespace mdl

namespace render
{
class BrushRenderer;
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{

class CreateBrushesToolBase : public Tool
{
protected:
  mdl::Map& m_map;

private:
  std::vector<std::unique_ptr<mdl::BrushNode>> m_brushNodes;
  std::unique_ptr<render::BrushRenderer> m_brushRenderer;

public:
  CreateBrushesToolBase(bool initiallyActive, mdl::Map& map);
  ~CreateBrushesToolBase() override;

public:
  const mdl::Grid& grid() const;

  void createBrushes();
  void clearBrushes();
  void cancel();

  void render(render::RenderContext& renderContext, render::RenderBatch& renderBatch);

protected:
  void updateBrushes(std::vector<std::unique_ptr<mdl::BrushNode>> brushNodes);

private:
  virtual void doBrushesWereCreated();

  deleteCopyAndMove(CreateBrushesToolBase);
};

} // namespace ui
} // namespace tb
