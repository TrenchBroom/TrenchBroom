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

#include "CreateBrushToolBase.h"

#include "Model/BrushNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/SelectionBoundsRenderer.h"
#include "View/MapDocument.h"

#include "kdl/memory_utils.h"

namespace TrenchBroom::View
{

CreateBrushToolBase::CreateBrushToolBase(
  const bool initiallyActive, std::weak_ptr<MapDocument> document)
  : Tool{initiallyActive}
  , m_document{std::move(document)}
  , m_brushRenderer{std::make_unique<Renderer::BrushRenderer>()}
{
}

CreateBrushToolBase::~CreateBrushToolBase() = default;

const Grid& CreateBrushToolBase::grid() const
{
  return kdl::mem_lock(m_document)->grid();
}

void CreateBrushToolBase::createBrush()
{
  if (m_brushNode)
  {
    auto document = kdl::mem_lock(m_document);

    auto transaction = Transaction{document, "Create Brush"};
    document->deselectAll();
    auto addedNodes =
      document->addNodes({{document->parentForNodes(), {m_brushNode.release()}}});
    document->selectNodes(addedNodes);
    transaction.commit();

    doBrushWasCreated();
  }
}

void CreateBrushToolBase::cancel()
{
  m_brushNode.release();
}

void CreateBrushToolBase::render(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  if (m_brushNode)
  {
    renderBrush(renderContext, renderBatch);
  }
}

void CreateBrushToolBase::renderBrush(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  ensure(m_brushNode, "brush is not null");

  m_brushRenderer->setFaceColor(pref(Preferences::FaceColor));
  m_brushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
  m_brushRenderer->setShowEdges(true);
  m_brushRenderer->setShowOccludedEdges(true);
  m_brushRenderer->setOccludedEdgeColor(Color(
    pref(Preferences::SelectedEdgeColor), pref(Preferences::OccludedSelectedEdgeAlpha)));
  m_brushRenderer->setTint(true);
  m_brushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
  m_brushRenderer->setForceTransparent(true);
  m_brushRenderer->setTransparencyAlpha(0.7f);

  m_brushRenderer->clear();
  m_brushRenderer->addBrush(m_brushNode.get());
  m_brushRenderer->render(renderContext, renderBatch);

  auto boundsRenderer = Renderer::SelectionBoundsRenderer{m_brushNode->logicalBounds()};
  boundsRenderer.render(renderContext, renderBatch);
}

void CreateBrushToolBase::updateBrush(std::unique_ptr<Model::BrushNode> brushNode)
{
  m_brushNode = std::move(brushNode);
}

void CreateBrushToolBase::doBrushWasCreated() {}

} // namespace TrenchBroom::View
