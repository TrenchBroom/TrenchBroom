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
            return true;
        }
        
        bool TexturingViewOffsetTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_face != NULL);

            const Vec3 last = m_face->transformToTexCoordSystem(refPoint);
            const Vec3 cur  = m_face->transformToTexCoordSystem(curPoint);
            
            const Grid& grid = document()->grid();
            const Vec3 offset = grid.snap(cur - last);
            
            if (offset.null())
                return true;
            
            controller()->beginUndoableGroup("Move Texture");
            const Model::BrushFaceList applyTo(1, m_face);
            if (offset.x() != 0.0)
                controller()->setFaceXOffset(applyTo, -offset.x(), true);
            if (offset.y() != 0.0)
                controller()->setFaceYOffset(applyTo, -offset.y(), true);
            controller()->closeGroup();
            
            refPoint = m_face->transformFromTexCoordSystem(last + offset);
            return true;
        }
        
        void TexturingViewOffsetTool::doEndPlaneDrag(const InputState& inputState) {
            assert(m_face != NULL);
            m_face = NULL;
        }
        
        void TexturingViewOffsetTool::doCancelPlaneDrag(const InputState& inputState) {
            assert(m_face != NULL);
            m_face = NULL;
        }
    }
}
