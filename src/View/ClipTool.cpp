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
#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/ModelUtils.h"
#include "Renderer/RenderContext.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ClipTool::HandleHit = Model::Hit::freeHitType();

        ClipTool::ClipTool(BaseTool* next, MapDocumentPtr document, ControllerPtr controller, const Renderer::Camera& camera) :
        Tool(next, document, controller),
        m_clipper(camera),
        m_renderer(m_clipper) {}
        
        bool ClipTool::canToggleClipSide() const {
            return m_clipper.numPoints() > 0;
        }

        void ClipTool::toggleClipSide() {
            m_clipper.toggleClipSide();
            updateBrushes();
        }
        
        bool ClipTool::canPerformClip() const {
            return m_clipper.numPoints() > 0;
        }

        void ClipTool::performClip() {
            // need to make a copy here so that it is not affected by the deselection
            const Model::ObjectList objects = document()->selectedObjects();
            
            controller()->beginUndoableGroup(objects.size() == 1 ? "Clip Brush" : "Clip Brushes");
            controller()->deselectAll();
            controller()->removeObjects(objects);
            if (!m_frontBrushes.empty() && m_clipper.keepFrontBrushes()) {
                const Model::ObjectParentList frontBrushes = Model::makeObjectParentList(m_frontBrushes);
                controller()->addObjects(frontBrushes);
                controller()->selectObjects(makeObjectList(frontBrushes));
                m_frontBrushes.clear();
            } else {
                clearAndDelete(m_frontBrushes);
            }
            if (!m_backBrushes.empty() && m_clipper.keepBackBrushes()) {
                const Model::ObjectParentList backBrushes = Model::makeObjectParentList(m_backBrushes);
                controller()->addObjects(backBrushes);
                controller()->selectObjects(makeObjectList(backBrushes));
                m_backBrushes.clear();
            } else {
                clearAndDelete(m_backBrushes);
            }
            controller()->closeGroup();
            
            m_clipper.reset();
            updateBrushes();
        }

        bool ClipTool::initiallyActive() const {
            return false;
        }
        
        bool ClipTool::doActivate(const InputState& inputState) {
            m_clipper.reset();
            updateBrushes();
            return true;
        }
        
        bool ClipTool::doDeactivate(const InputState& inputState) {
            clearAndDelete(m_frontBrushes);
            clearAndDelete(m_backBrushes);
            return true;
        }

        void ClipTool::doPick(const InputState& inputState, Model::PickResult& pickResult) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const double radius = 2.0 * prefs.getDouble(Preferences::HandleRadius);
            const double scaling = prefs.getDouble(Preferences::HandleScalingFactor);
            const double maxDist = prefs.getDouble(Preferences::MaximumHandleDistance);
            const Ray3& ray = inputState.pickRay();
            
            const Vec3::List clipPoints = m_clipper.clipPointPositions();
            for (size_t i = 0; i < clipPoints.size(); ++i) {
                const FloatType dist = ray.intersectWithSphere(clipPoints[i],
                                                               radius, scaling, maxDist);
                if (!Math::isnan(dist)) {
                    const Vec3 hitPoint = ray.pointAtDistance(dist);
                    pickResult.addHit(Model::Hit(HandleHit, dist, hitPoint, i));
                }
            }
        }

        bool ClipTool::doMouseUp(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;

            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            if (first.matches) {
                const Vec3 point = clipPoint(first.hit);
                if (m_clipper.canAddClipPoint(point)) {
                    m_clipper.addClipPoint(point, *hitAsFace(first.hit));
                    updateBrushes();
                }
            }
            
            return true;
        }
        
        bool ClipTool::doStartMouseDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            const Model::PickResult::FirstHit firstHandleHit = Model::firstHit(inputState.pickResult(), HandleHit, true);
            if (!firstHandleHit.matches)
                return false;
            
            m_dragPointIndex = firstHandleHit.hit.target<size_t>();
            return true;
        }
        
        bool ClipTool::doMouseDrag(const InputState& inputState) {
            Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            if (first.matches) {
                const Vec3 point = clipPoint(first.hit);
                if (m_clipper.canUpdateClipPoint(m_dragPointIndex, point)) {
                    m_clipper.updateClipPoint(m_dragPointIndex, point, *hitAsFace(first.hit));
                    updateBrushes();
                }
            }
            return true;
        }
        
        void ClipTool::doEndMouseDrag(const InputState& inputState) {
        }
        
        void ClipTool::doCancelMouseDrag(const InputState& inputState) {
        }

        void ClipTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setHideSelection();
        }

        void ClipTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            m_renderer.renderBrushes(renderContext);
            m_renderer.renderClipPoints(renderContext);
            
            if (dragging()) {
                m_renderer.renderHighlight(renderContext, m_dragPointIndex);
            } else {
                const Model::PickResult::FirstHit firstBrushHit = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
                const Model::PickResult::FirstHit firstHandleHit = Model::firstHit(inputState.pickResult(), HandleHit, true);
                
                if (firstHandleHit.matches) {
                    const size_t index = firstHandleHit.hit.target<size_t>();
                    m_renderer.renderHighlight(renderContext, index);
                } else if (firstBrushHit.matches) {
                    const Vec3 point = clipPoint(firstBrushHit.hit);
                    if (m_clipper.canAddClipPoint(point))
                        m_renderer.renderCurrentPoint(renderContext, point);
                }
            }
        }
        
        Vec3 ClipTool::clipPoint(const Model::Hit& hit) const {
            const Model::BrushFace& face = *hitAsFace(hit);
            const Vec3& point = hit.hitPoint();
            return document()->grid().snap(point, face.boundary());
        }

        void ClipTool::updateBrushes() {
            clearAndDelete(m_frontBrushes);
            clearAndDelete(m_backBrushes);
            
            const Model::BrushList& brushes = document()->selectedBrushes();
            if (!brushes.empty()) {
                const ClipResult result = m_clipper.clip(brushes, document());
                m_frontBrushes = result.frontBrushes;
                m_backBrushes = result.backBrushes;
            }
            
            m_renderer.setBrushes(Model::mergeEntityBrushesMap(m_frontBrushes),
                                  Model::mergeEntityBrushesMap(m_backBrushes));
        }

        void ClipTool::clearAndDelete(Model::EntityBrushesMap& brushes) {
            Model::EntityBrushesMap::iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                VectorUtils::clearAndDelete(it->second);
            brushes.clear();
        }
    }
}
