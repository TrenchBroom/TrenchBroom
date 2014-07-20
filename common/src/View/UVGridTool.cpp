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

#include "UVGridTool.h"

#include "Model/BrushFace.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/UVViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType UVGridTool::XHandleHit = Hit::freeHitType();
        const Hit::HitType UVGridTool::YHandleHit = Hit::freeHitType();

        UVGridTool::UVGridTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper) :
        ToolImpl(document, controller),
        m_helper(helper) {}
        
        void UVGridTool::doPick(const InputState& inputState, Hits& hits) {
            static const Hit::HitType HitTypes[] = { XHandleHit, YHandleHit };
            if (m_helper.valid())
                m_helper.pickTextureGrid(inputState.pickRay(), HitTypes, hits);
        }

        bool UVGridTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            const Hits& hits = inputState.hits();
            const Hit& xHit = hits.findFirst(XHandleHit, true);
            const Hit& yHit = hits.findFirst(YHandleHit, true);
            
            if (!checkIfDragApplies(inputState, xHit, yHit))
                return false;
            
            m_handle = getScaleHandle(xHit, yHit);
            m_selector = Vec2b(xHit.isMatch(), yHit.isMatch());
            m_lastHitPoint = getHitPoint(inputState.pickRay());

            startDrag(m_lastHitPoint);
            
            controller()->beginUndoableGroup(getActionName());
            return true;
        }

        bool UVGridTool::doMouseDrag(const InputState& inputState) {
            const Vec2f curPoint = getHitPoint(inputState.pickRay());
            const Vec2f delta = performDrag(curPoint - m_lastHitPoint);
            
            m_lastHitPoint += delta;
            return true;
        }
        
        Vec2i UVGridTool::getScaleHandle(const Hit& xHit, const Hit& yHit) const {
            const int x = xHit.isMatch() ? xHit.target<int>() : 0;
            const int y = yHit.isMatch() ? yHit.target<int>() : 0;
            return Vec2i(x, y);
        }
        
        Vec2f UVGridTool::getHitPoint(const Ray3& pickRay) const {
            const Model::BrushFace* face = m_helper.face();
            const Plane3& boundary = face->boundary();
            const FloatType facePointDist = boundary.intersectWithRay(pickRay);
            const Vec3 facePoint = pickRay.pointAtDistance(facePointDist);
            
            const Mat4x4 toTex = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            return toTex * facePoint;
        }

        void UVGridTool::doEndMouseDrag(const InputState& inputState) {
            controller()->closeGroup();
        }
        
        void UVGridTool::doCancelMouseDrag(const InputState& inputState) {
            controller()->rollbackGroup();
        }
        
        Vec2i UVGridTool::getHandle() const {
            return m_handle;
        }

        Vec2f UVGridTool::getHandlePos() const {
            const Model::BrushFace* face = m_helper.face();
            const Mat4x4 toWorld = face->fromTexCoordSystemMatrix(face->offset(), face->scale(), true);
            const Mat4x4 toTex   = face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            
            return Vec2f(toTex * toWorld * Vec3(getScaledTranslatedHandlePos()));
        }
        
        Vec2f UVGridTool::getScaledTranslatedHandlePos() const {
            return Vec2f(m_handle * m_helper.stripeSize());
        }

        void UVGridTool::startDrag(const Vec2f& pos) {}
    }
}
