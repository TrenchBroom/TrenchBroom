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

#include "CreateBrushesToolBase.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushNode.h"
#include "render/BrushRenderer.h"
#include "render/SelectionBoundsRenderer.h"
#include "ui/MapDocument.h"
#include "ui/Transaction.h"

#include "kdl/memory_utils.h"

namespace tb::ui
{

CreateBrushesToolBase::CreateBrushesToolBase(
  const bool initiallyActive, std::weak_ptr<MapDocument> document)
  : Tool{initiallyActive}
  , m_document{std::move(document)}
  , m_brushRenderer{std::make_unique<render::BrushRenderer>()}
{
}

CreateBrushesToolBase::~CreateBrushesToolBase() = default;

const Grid& CreateBrushesToolBase::grid() const
{
  return kdl::mem_lock(m_document)->grid();
}

void CreateBrushesToolBase::createBrushes()
{
  if (!m_brushNodes.empty())
  {
    auto document = kdl::mem_lock(m_document);
    auto nodesToAdd = kdl::vec_transform(std::move(m_brushNodes), [](auto brushNode) {
      return static_cast<mdl::Node*>(brushNode.release());
    });
    clearBrushes();

    auto transaction = Transaction{document, "Create Brush"};
    document->deselectAll();
    auto addedNodes = document->addNodes({{document->parentForNodes(), nodesToAdd}});
    document->selectNodes(addedNodes);
    transaction.commit();

    doBrushesWereCreated();
  }
}

void CreateBrushesToolBase::clearBrushes()
{
  m_brushNodes.clear();
}

void CreateBrushesToolBase::cancel()
{
  clearBrushes();
}

void CreateBrushesToolBase::render(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_brushRenderer->clear();

  if (!m_brushNodes.empty())
  {
    m_brushRenderer->setFaceColor(pref(Preferences::FaceColor));
    m_brushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
    m_brushRenderer->setShowEdges(true);
    m_brushRenderer->setShowOccludedEdges(true);
    m_brushRenderer->setOccludedEdgeColor(Color(
      pref(Preferences::SelectedEdgeColor),
      pref(Preferences::OccludedSelectedEdgeAlpha)));
    m_brushRenderer->setTint(true);
    m_brushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
    m_brushRenderer->setForceTransparent(true);
    m_brushRenderer->setTransparencyAlpha(0.7f);

    auto boundsBuilder = vm::bbox3d::builder{};
    for (const auto& brushNode : m_brushNodes)
    {
      m_brushRenderer->addBrush(brushNode.get());
      boundsBuilder.add(brushNode->logicalBounds());
    }
    m_brushRenderer->render(renderContext, renderBatch);

    auto boundsRenderer = render::SelectionBoundsRenderer{boundsBuilder.bounds()};
    boundsRenderer.render(renderContext, renderBatch);
  }
}

void CreateBrushesToolBase::updateBrushes(
  std::vector<std::unique_ptr<mdl::BrushNode>> brushNodes)
{
  m_brushNodes = std::move(brushNodes);
}

void CreateBrushesToolBase::doBrushesWereCreated() {}

} // namespace tb::ui
