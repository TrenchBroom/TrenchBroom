/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "ClipTool.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/Picker.h"
#include "View/InputState.h"

namespace TrenchBroom {
    namespace View {
        ClipTool::ClipTool(BaseTool* next, MapDocumentPtr document, ControllerFacade& controller, const Renderer::Camera& camera) :
        Tool(next, document, controller),
        m_clipper(camera) {}
        
        bool ClipTool::initiallyActive() const {
            return false;
        }
        
        bool ClipTool::doActivate(const InputState& inputState) {
            m_clipper.reset();
            return true;
        }
        
        bool ClipTool::doDeactivate(const InputState& inputState) {
            return true;
        }

        bool ClipTool::doMouseUp(const InputState& inputState) {
            Model::HitFilterChain hitFilter = Model::chainHitFilters(Model::TypedHitFilter(Model::Brush::BrushHit),
                                                                     Model::DefaultHitFilter(inputState.filter()));
            Model::PickResult::FirstHit first = inputState.pickResult().firstHit(hitFilter, true);
            if (first.matches) {
                const Vec3& point = first.hit.hitPoint();
                if (m_clipper.clipPointValid(point))
                    m_clipper.addClipPoint(point, *hitAsFace(first.hit));
            }
            
            return true;
        }
        
        void ClipTool::doMouseMove(const InputState& inputState) {
            Model::HitFilterChain hitFilter = Model::chainHitFilters(Model::TypedHitFilter(Model::Brush::BrushHit),
                                                                     Model::DefaultHitFilter(inputState.filter()));
            Model::PickResult::FirstHit first = inputState.pickResult().firstHit(hitFilter, true);
            if (first.matches) {
                const Vec3& point = first.hit.hitPoint();
                if (m_clipper.clipPointValid(point)) {
                    // set current highlight point
                }
            }
        }
        
        void ClipTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
        }
    }
}
