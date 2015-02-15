/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "CreateBrushToolAdapter3D.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "View/CreateBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateBrushToolAdapter3D::CreateBrushToolAdapter3D(CreateBrushTool* tool, const Grid& grid) :
        m_tool(tool),
        m_grid(grid) {
            assert(tool != NULL);
        }

        CreateBrushToolAdapter3D::~CreateBrushToolAdapter3D() {}

        Tool* CreateBrushToolAdapter3D::doGetTool() {
            return m_tool;
        }
        
        void CreateBrushToolAdapter3D::doModifierKeyChange(const InputState& inputState) {
        }
        
        bool CreateBrushToolAdapter3D::doMouseClick(const InputState& inputState) {
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            if (inputState.modifierKeysPressed(ModifierKeys::MKShift)) {
                const Model::BrushFace* face = Model::hitToFace(hit);
                const Model::BrushVertexList& vertices = face->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Model::BrushVertex* vertex = vertices[i];
                    m_tool->addPoint(vertex->position);
                }
            } else {
                const Model::BrushFace* face = Model::hitToFace(hit);
                const Vec3 snapped = m_grid.snap(hit.hitPoint(), face->boundary());
                m_tool->addPoint(snapped);
            }
            
            return true;
        }
        
        void CreateBrushToolAdapter3D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
        }
        
        void CreateBrushToolAdapter3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateBrushToolAdapter3D::doCancel() {
            return m_tool->clear();
        }
    }
}
