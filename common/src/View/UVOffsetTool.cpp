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

#include "UVOffsetTool.h"

#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/UVView.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        UVOffsetTool::UVOffsetTool(MapDocumentWPtr document, const UVViewHelper& helper) :
        ToolAdapterBase(),
        Tool(true),
        m_document(document),
        m_helper(helper) {}
        
        Tool* UVOffsetTool::doGetTool() {
            return this;
        }
        
        bool UVOffsetTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            m_lastPoint = computeHitPoint(inputState.pickRay());

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Move Texture");
            return true;
        }
        
        bool UVOffsetTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            const Vec2f curPoint = computeHitPoint(inputState.pickRay());
            const Vec2f delta    = curPoint - m_lastPoint;
            const Vec2f snapped  = snapDelta(delta);

            const Model::BrushFace* face = m_helper.face();
            const Vec2f corrected = (face->offset() - snapped).corrected(4, 0.0f);
            
            if (corrected == face->offset())
                return true;
            
            Model::ChangeBrushFaceAttributesRequest request;
            request.setOffset(corrected);

            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
            
            m_lastPoint += snapped;
            return true;
        }
        
        void UVOffsetTool::doEndMouseDrag(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
        }
        
        void UVOffsetTool::doCancelMouseDrag() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
        }

        Vec2f UVOffsetTool::computeHitPoint(const Ray3& ray) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType distance = boundary.intersectWithRay(ray);
            const Vec3 hitPoint = ray.pointAtDistance(distance);
            
            const Mat4x4 transform = face->toTexCoordSystemMatrix(Vec2f::Null, face->scale(), true);
            return Vec2f(transform * hitPoint);
        }

        Vec2f UVOffsetTool::snapDelta(const Vec2f& delta) const {
            const Model::BrushFace* face = m_helper.face();
            assert(face != NULL);
            
            const Assets::Texture* texture = face->texture();
            if (texture == NULL)
                return delta.rounded();
            
            const Mat4x4 transform = face->toTexCoordSystemMatrix(face->offset() - delta, face->scale(), true);
            
            Vec2f distance = Vec2f::Max;
            const Model::BrushFace::VertexList vertices = face->vertices();
            Model::BrushFace::VertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                const Model::BrushVertex* vertex = *it;
                const Vec2f temp = m_helper.computeDistanceFromTextureGrid(transform * vertex->position());
                distance = absMin(distance, temp);
            }
            
            return m_helper.snapDelta(delta, -distance);
        }
        
        bool UVOffsetTool::doCancel() {
            return false;
        }
    }
}
