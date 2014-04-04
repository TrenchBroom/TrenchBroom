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

#include "TexturingViewRotateTool.h"

#include "Model/BrushFace.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"
#include "View/InputState.h"
#include "View/TexturingViewHelper.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingViewRotateTool::HandleHit = Hit::freeHitType();
        const FloatType TexturingViewRotateTool::MaxPickDistance = 5.0;

        TexturingViewRotateTool::TexturingViewRotateTool(MapDocumentWPtr document, ControllerWPtr controller, TexturingViewHelper& helper, Renderer::OrthographicCamera& camera) :
        ToolImpl(document, controller),
        m_helper(helper),
        m_camera(camera) {}
        
        void TexturingViewRotateTool::doPick(const InputState& inputState, Hits& hits) {
            if (!m_helper.valid()) {
                const Model::BrushFace* face = m_helper.face();
                const Model::BrushEdgeList& edges = face->edges();
                const Ray3& pickRay = inputState.pickRay();
                
                Model::BrushEdgeList::const_iterator it, end;
                for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                    const Model::BrushEdge* edge = *it;
                    const Ray3::LineDistance pickDistance = pickRay.distanceToSegment(edge->start->position, edge->end->position);
                    assert(!pickDistance.parallel);
                    
                    if (Math::abs(pickDistance.distance) <= MaxPickDistance) {
                        const FloatType hitDistance = pickDistance.rayDistance;
                        const Vec3 hitPoint = pickRay.pointAtDistance(hitDistance);
                        const FloatType error = pickDistance.distance;
                        hits.addHit(Hit(HandleHit, hitDistance, hitPoint, edge->vector(), error));
                    }
                }
            }
        }
        
        bool TexturingViewRotateTool::doMouseUp(const InputState& inputState) {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Hits& hits = inputState.hits();
            const Hit& handleHit = hits.findFirst(HandleHit, true);
            if (!handleHit.isMatch())
                return false;
            
            const Vec3 direction = handleHit.target<Vec3>();
            return false;
        }
        
        bool TexturingViewRotateTool::doMouseDoubleClick(const InputState& inputState) {
            return false;
        }
        
        void TexturingViewRotateTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
        }
    }
}
