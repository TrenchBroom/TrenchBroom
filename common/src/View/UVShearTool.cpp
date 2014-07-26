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

#include "UVShearTool.h"

#include "Model/BrushFace.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType UVShearTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType UVShearTool::YHandleHit = Hit::freeHitType();
        
        UVShearTool::UVShearTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        ToolImpl(document, controller),
        m_helper(helper) {}

        void UVShearTool::doPick(const InputState& inputState, Hits& hits) {
            static const Hit::HitType HitTypes[] = { XHandleHit, YHandleHit };
            if (m_helper.valid())
                m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, hits);
        }
        
        bool UVShearTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());
            
            if (!inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hits& hits = inputState.hits();
            const Hit& xHit = hits.findFirst(XHandleHit, true);
            const Hit& yHit = hits.findFirst(YHandleHit, true);
            
            if (!(xHit.isMatch() ^ yHit.isMatch()))
                return false;
            
            m_selector = Vec2b(xHit.isMatch(), yHit.isMatch());

            const Model::BrushFace* face = m_helper.face();
            m_xAxis = face->textureXAxis();
            m_yAxis = face->textureYAxis();
            m_initialHit = m_lastHit = getHit(inputState.pickRay());
            
            controller()->beginUndoableGroup("Shear texture");
            return true;
        }
        
        bool UVShearTool::doMouseDrag(const InputState& inputState) {
            const Vec2f currentHit = getHit(inputState.pickRay());
            const Vec2f delta = currentHit - m_lastHit;

            Model::BrushFace* face = m_helper.face();
            const Vec3 origin = face->textureCoords(m_helper.origin());
            const Vec2f oldCoords = face->textureCoords(origin);
            
            const Model::BrushFaceList applyTo(1, face);
            if (m_selector[0]) {
                const Vec2f factors = Vec2f(-delta.y() / m_initialHit.x(), 0.0f);
                controller()->shearTextures(applyTo, factors, origin);
            } else if (m_selector[1]) {
                const Vec2f factors = Vec2f(0.0f, -delta.x() / m_initialHit.y());
                controller()->shearTextures(applyTo, factors, origin);
            }
            
            const Vec2f newCoords = face->textureCoords(origin);
            const Vec2f newOffset = face->offset() - oldCoords + newCoords;
            controller()->setFaceOffset(applyTo, newOffset, false);
            
            m_lastHit = currentHit;
            return true;
        }
        
        void UVShearTool::doEndMouseDrag(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void UVShearTool::doCancelMouseDrag(const InputState& inputState) {
            controller()->rollbackGroup();
            controller()->closeGroup();
        }

        Vec2f UVShearTool::getHit(const Ray3& pickRay) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType hitPointDist = boundary.intersectWithRay(pickRay);
            const Vec3 hitPoint = pickRay.pointAtDistance(hitPointDist);
            const Vec3 hitVec = hitPoint - m_helper.origin();
            
            return Vec2f(hitVec.dot(m_xAxis),
                         hitVec.dot(m_yAxis));
        }
        
        void UVShearTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
        }
    }
}
