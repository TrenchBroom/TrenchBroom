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

#include "CreateComplexBrushTool.h"

#include "Exceptions.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushError.h"
#include "Model/BrushNode.h"
#include "Model/Game.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/result.h>

namespace TrenchBroom {
namespace View {
CreateComplexBrushTool::CreateComplexBrushTool(std::weak_ptr<MapDocument> document)
  : CreateBrushToolBase(false, document)
  , m_polyhedron(std::make_unique<Model::Polyhedron3>()) {}

const Model::Polyhedron3& CreateComplexBrushTool::polyhedron() const {
  return *m_polyhedron;
}

void CreateComplexBrushTool::update(const Model::Polyhedron3& polyhedron) {
  *m_polyhedron = polyhedron;
  if (m_polyhedron->closed()) {
    auto document = kdl::mem_lock(m_document);
    const auto game = document->game();
    const Model::BrushBuilder builder(
      document->world()->mapFormat(), document->worldBounds(), game->defaultFaceAttribs());

    builder.createBrush(*m_polyhedron, document->currentTextureName())
      .and_then([&](Model::Brush&& b) {
        updateBrush(new Model::BrushNode(std::move(b)));
      })
      .handle_errors([&](const Model::BrushError e) {
        updateBrush(nullptr);
        document->error() << "Could not update brush: " << e;
      });
  } else {
    updateBrush(nullptr);
  }
}

bool CreateComplexBrushTool::doActivate() {
  update(Model::Polyhedron3());
  return true;
}

bool CreateComplexBrushTool::doDeactivate() {
  update(Model::Polyhedron3());
  return true;
}

void CreateComplexBrushTool::doBrushWasCreated() {
  update(Model::Polyhedron3());
}
} // namespace View
} // namespace TrenchBroom
