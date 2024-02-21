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

#include "Macros.h"
#include "View/Tool.h"

#include <memory>

namespace TrenchBroom::Model
{
class BrushNode;
}

namespace TrenchBroom::Renderer
{
class BrushRenderer;
class RenderBatch;
class RenderContext;
} // namespace TrenchBroom::Renderer

namespace TrenchBroom::View
{
class Grid;
class MapDocument;

class CreateBrushesToolBase : public Tool
{
protected:
  std::weak_ptr<MapDocument> m_document;

private:
  std::vector<std::unique_ptr<Model::BrushNode>> m_brushNodes;
  std::unique_ptr<Renderer::BrushRenderer> m_brushRenderer;

public:
  CreateBrushesToolBase(bool initiallyActive, std::weak_ptr<MapDocument> document);
  ~CreateBrushesToolBase() override;

public:
  const Grid& grid() const;

  void createBrushes();
  void clearBrushes();
  void cancel();

  void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

protected:
  void updateBrushes(std::vector<std::unique_ptr<Model::BrushNode>> brushNodes);

private:
  virtual void doBrushesWereCreated();

  deleteCopyAndMove(CreateBrushesToolBase);
};

} // namespace TrenchBroom::View
