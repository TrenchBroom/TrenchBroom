/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreateBrushTool.h"

#include "Model/Brush.h"
#include "Model/Face.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Model/Picker.h"
#include "Model/Texture.h"
#include "Renderer/Camera.h"
#include "Utility/Grid.h"
#include "View/EditorView.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        void CreateBrushTool::updateBounds(const Vec3f& currentPoint) {
            m_bounds.min = m_bounds.max = m_initialPoint;
            m_bounds.mergeWith(currentPoint);
            
            Utility::Grid& grid = document().grid();
            m_bounds.min = grid.snapDown(m_bounds.min);
            m_bounds.max = grid.snapUp(m_bounds.max);
        }

        bool CreateBrushTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            assert(m_brush == NULL);
            
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Utility::Grid& grid = document().grid();

            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, view().filter()));
            if (hit != NULL) {
                const Vec3f& normal = hit->face().boundary().normal.firstAxis();
                initialPoint = hit->hitPoint();
                plane = Plane(normal, initialPoint + normal * grid.actualSize());
            } else {
                Renderer::Camera& camera = view().camera();
                const Vec3f normal = -camera.direction().firstAxis();
                initialPoint = camera.defaultPoint(inputState.pickRay().direction);
                plane = Plane(normal, initialPoint + normal * grid.actualSize());
            }
            
            m_initialPoint = initialPoint;
            updateBounds(m_initialPoint);
            
            return true;
        }
        
        void CreateBrushTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            assert(m_brush != NULL);
            updateBounds(curPoint);
        }
        
        void CreateBrushTool::handleEndPlaneDrag(InputState& inputState) {
            assert(m_brush != NULL);
            m_brush = NULL;
        }

        CreateBrushTool::CreateBrushTool(View::DocumentViewHolder& documentViewHolder) :
        PlaneDragTool(documentViewHolder, true),
        m_brush(NULL) {}
    }
}