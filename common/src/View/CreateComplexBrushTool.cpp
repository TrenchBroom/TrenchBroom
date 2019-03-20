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
#include "Polyhedron.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/RenderService.h"
#include "Renderer/SelectionBoundsRenderer.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        CreateComplexBrushTool::CreateComplexBrushTool(MapDocumentWPtr document) :
        CreateBrushToolBase(false, document) {}

        const Polyhedron3& CreateComplexBrushTool::polyhedron() const {
            return m_polyhedron;
        }

        void CreateComplexBrushTool::update(const Polyhedron3& polyhedron) {
            m_polyhedron = polyhedron;
            if (m_polyhedron.closed()) {
                MapDocumentSPtr document = lock(m_document);
                const Model::BrushBuilder builder(document->world(), document->worldBounds());
                Model::Brush* brush = builder.createBrush(m_polyhedron, document->currentTextureName());
                updateBrush(brush);
            } else {
                updateBrush(nullptr);
            }
        }

        bool CreateComplexBrushTool::doActivate() {
            update(Polyhedron3());
            return true;
        }

        bool CreateComplexBrushTool::doDeactivate() {
            update(Polyhedron3());
            return true;
        }

        void CreateComplexBrushTool::doBrushWasCreated() {
            update(Polyhedron3());
        }
    }
}
