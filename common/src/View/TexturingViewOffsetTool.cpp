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
#include "Model/BrushVertex.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TexturingView.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TexturingViewOffsetTool::TexturingViewOffsetTool(MapDocumentWPtr document, ControllerWPtr controller, const TexturingViewHelper& helper) :
        ToolImpl(document, controller),
        m_helper(helper) {}
        
        bool TexturingViewOffsetTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            m_lastPoint = computeHitPoint(inputState.pickRay());
            
            controller()->beginUndoableGroup("Move Texture");
            return true;
        }
        
        bool TexturingViewOffsetTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            const Vec2f curPoint = computeHitPoint(inputState.pickRay());
            const Vec2f delta    = curPoint - m_lastPoint;
            const Vec2f snapped  = snapDelta(delta);
            
            if (snapped.null())
                return true;
            
            const Model::BrushFaceList applyTo(1, m_helper.face());
            controller()->setFaceOffset(applyTo, -snapped, true);
            
            m_lastPoint += snapped;
            return true;
        }
        
        void TexturingViewOffsetTool::doEndMouseDrag(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void TexturingViewOffsetTool::doCancelMouseDrag(const InputState& inputState) {
            controller()->rollbackGroup();
        }

        Vec2f TexturingViewOffsetTool::computeHitPoint(const Ray3& ray) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType distance = boundary.intersectWithRay(ray);
            const Vec3 hitPoint = ray.pointAtDistance(distance);
            
            const Mat4x4 transform = face->toTexCoordSystemMatrix(Vec2f::Null, face->scale(), true);
            return Vec2f(transform * hitPoint);
        }

        Vec2f TexturingViewOffsetTool::snapDelta(const Vec2f& delta) const {
            const Model::BrushFace* face = m_helper.face();
            assert(face != NULL);
            
            const Assets::Texture* texture = face->texture();
            if (texture == NULL)
                return delta.rounded();
            
            const Mat4x4 transform = face->toTexCoordSystemMatrix(face->offset() - delta, face->scale(), true);
            
            const Model::BrushVertexList& vertices = face->vertices();
            Vec2f distance = m_helper.computeDistanceFromTextureGrid(transform * vertices[0]->position);
            
            for (size_t i = 1; i < vertices.size(); ++i)
                distance = absMin(distance, m_helper.computeDistanceFromTextureGrid(transform * vertices[i]->position));
            
            return m_helper.snapDelta(delta, distance);
        }
        
    }
}
