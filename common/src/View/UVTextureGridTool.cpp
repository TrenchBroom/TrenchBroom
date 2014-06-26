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

#include "UVTextureGridTool.h"

#include "Model/BrushFace.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType UVViewTextureGridTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType UVViewTextureGridTool::YHandleHit = Hit::freeHitType();

        UVViewTextureGridTool::UVViewTextureGridTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        ToolImpl(document, controller),
        m_helper(helper) {}
        
        void UVViewTextureGridTool::doPick(const InputState& inputState, Hits& hits) {
            static const Hit::HitType HitTypes[] = { XHandleHit, YHandleHit };
            if (m_helper.valid())
                m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, hits);
        }

        bool UVViewTextureGridTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            const Hits& hits = inputState.hits();
            const Hit& xHit = hits.findFirst(XHandleHit, true);
            const Hit& yHit = hits.findFirst(YHandleHit, true);
            
            if (!checkIfDragApplies(inputState, xHit, yHit))
                return false;
            
            m_handle = getScaleHandle(xHit, yHit);
            m_selector = Vec2b(xHit.isMatch(), yHit.isMatch());
            m_lastHitPoint = getHitPoint(inputState.pickRay());

            controller()->beginUndoableGroup(getActionName());
            return true;
        }

        bool UVViewTextureGridTool::doMouseDrag(const InputState& inputState) {
            const Vec2f curPoint = getHitPoint(inputState.pickRay());
            const Vec2f actualDelta = performDrag(curPoint - m_lastHitPoint);
            
            m_lastHitPoint += actualDelta;
            return true;
        }
        
        Vec2i UVViewTextureGridTool::getScaleHandle(const Hit& xHit, const Hit& yHit) const {
            const int x = xHit.isMatch() ? xHit.target<int>() : 0;
            const int y = yHit.isMatch() ? yHit.target<int>() : 0;
            return Vec2i(x, y);
        }
        
        Vec2f UVViewTextureGridTool::getHitPoint(const Ray3& pickRay) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType facePointDist = boundary.intersectWithRay(pickRay);
            const Vec3 facePoint = pickRay.pointAtDistance(facePointDist);
            
            const Mat4x4 toTex = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            return toTex * facePoint;
        }

        void UVViewTextureGridTool::doEndMouseDrag(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void UVViewTextureGridTool::doCancelMouseDrag(const InputState& inputState) {
            controller()->rollbackGroup();
        }
        
        Vec2f UVViewTextureGridTool::getHandlePos() const {
            const Model::BrushFace* face = m_helper.face();
            const Mat4x4 toWorld = face->fromTexCoordSystemMatrix(face->offset(), face->scale(), true);
            const Mat4x4 toTex   = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            
            return Vec2f(toTex * toWorld * Vec3(getScaledTranslatedHandlePos()));
        }
        
        Vec2f UVViewTextureGridTool::getScaledTranslatedHandlePos() const {
            return Vec2f(m_handle * m_helper.stripeSize());
        }
    }
}
