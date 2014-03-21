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

#include "TexturingViewOffsetTool.h"

#include "Hit.h"
#include "Model/BrushFace.h"
#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TexturingView.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TexturingViewOffsetTool::TexturingViewOffsetTool(MapDocumentWPtr document, ControllerWPtr controller) :
        ToolImpl(document, controller),
        m_face(NULL) {}
        
        bool TexturingViewOffsetTool::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_face == NULL);
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hit& hit = inputState.hits().findFirst(TexturingView::FaceHit, false);
            if (!hit.isMatch())
                return false;
            
            m_face = hit.target<Model::BrushFace*>();
            plane = Plane3(hit.hitPoint(), m_face->boundary().normal);
            initialPoint = hit.hitPoint();

            controller()->beginUndoableGroup("Move Texture");
            return true;
        }
        
        bool TexturingViewOffsetTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_face != NULL);

            const Mat4x4 toTexTransform = m_face->toTexCoordSystemMatrix();
            const Vec3 last = toTexTransform * refPoint;
            const Vec3 cur  = toTexTransform * curPoint;
            
            const Grid& grid = document()->grid();
            const Vec3 offset = grid.snap(cur - last);
            
            if (offset.null())
                return true;
            
            const Model::BrushFaceList applyTo(1, m_face);
            if (offset.x() != 0.0)
                controller()->setFaceXOffset(applyTo, -offset.x(), true);
            if (offset.y() != 0.0)
                controller()->setFaceYOffset(applyTo, -offset.y(), true);
            
            const Mat4x4 fromTexTransform = m_face->fromTexCoordSystemMatrix();
            refPoint = fromTexTransform * (last + offset);
            return true;
        }
        
        void TexturingViewOffsetTool::doEndPlaneDrag(const InputState& inputState) {
            controller()->closeGroup();
            assert(m_face != NULL);
            m_face = NULL;
        }
        
        void TexturingViewOffsetTool::doCancelPlaneDrag(const InputState& inputState) {
            controller()->rollbackGroup();
            assert(m_face != NULL);
            m_face = NULL;
        }
    }
}
