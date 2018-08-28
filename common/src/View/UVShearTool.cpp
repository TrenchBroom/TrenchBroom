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

#include "UVShearTool.h"

#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/PickResult.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType UVShearTool::XHandleHit = Model::Hit::freeHitType();
        const Model::Hit::HitType UVShearTool::YHandleHit = Model::Hit::freeHitType();
        
        UVShearTool::UVShearTool(MapDocumentWPtr document, UVViewHelper& helper) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_helper(helper) {}

        Tool* UVShearTool::doGetTool() {
            return this;
        }
        
        void UVShearTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            static const Model::Hit::HitType HitTypes[] = { XHandleHit, YHandleHit };
            if (m_helper.valid())
                m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, pickResult);
        }
        
        bool UVShearTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& xHit = pickResult.query().type(XHandleHit).occluded().first();
            const Model::Hit& yHit = pickResult.query().type(YHandleHit).occluded().first();
            
            if (!(xHit.isMatch() ^ yHit.isMatch()))
                return false;
            
            m_selector = vec2b(xHit.isMatch(), yHit.isMatch());

            const Model::BrushFace* face = m_helper.face();
            m_xAxis = face->textureXAxis();
            m_yAxis = face->textureYAxis();
            m_initialHit = m_lastHit = getHit(inputState.pickRay());
            
            // #1350: Don't allow shearing if the shear would result in very large changes. This happens if
            // the shear handle to be dragged is very close to one of the texture axes.
            if (Math::zero(m_initialHit.x(), 6.0f) ||
                Math::zero(m_initialHit.y(), 6.0f))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Shear Texture");
            return true;
        }
        
        bool UVShearTool::doMouseDrag(const InputState& inputState) {
            const vec2f currentHit = getHit(inputState.pickRay());
            const vec2f delta = currentHit - m_lastHit;

            Model::BrushFace* face = m_helper.face();
            const vec3 origin = m_helper.origin();
            const vec2f oldCoords = face->toTexCoordSystemMatrix(vec2f::zero, face->scale(), true) * origin;
            
            MapDocumentSPtr document = lock(m_document);
            if (m_selector[0]) {
                const vec2f factors = vec2f(-delta.y() / m_initialHit.x(), 0.0f);
                if (!isZero(factors))
                    document->shearTextures(factors);
            } else if (m_selector[1]) {
                const vec2f factors = vec2f(0.0f, -delta.x() / m_initialHit.y());
                if (!isZero(factors))
                    document->shearTextures(factors);
            }
            
            const vec2f newCoords = face->toTexCoordSystemMatrix(vec2f::zero, face->scale(), true) * origin;
            const vec2f newOffset = face->offset() + oldCoords - newCoords;

            Model::ChangeBrushFaceAttributesRequest request;
            request.setOffset(newOffset);
            document->setFaceAttributes(request);
            
            m_lastHit = currentHit;
            return true;
        }
        
        void UVShearTool::doEndMouseDrag(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
        }
        
        void UVShearTool::doCancelMouseDrag() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
        }

        vec2f UVShearTool::getHit(const Ray3& pickRay) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType hitPointDist = boundary.intersectWithRay(pickRay);
            const vec3 hitPoint = pickRay.pointAtDistance(hitPointDist);
            const vec3 hitVec = hitPoint - m_helper.origin();
            
            return vec2f(dot(hitVec, m_xAxis),
                         dot(hitVec, m_yAxis));
        }
        
        bool UVShearTool::doCancel() {
            return false;
        }
    }
}
