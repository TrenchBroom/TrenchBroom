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

#include "ClipTool.h"
#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/ModelUtils.h"
#include "Renderer/RenderContext.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        const Hit::HitType ClipTool::HandleHit = Hit::freeHitType();

        ClipTool::ClipTool(MapDocumentWPtr document, ControllerWPtr controller, const Renderer::Camera& camera) :
        ToolImpl(document, controller),
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
            // need to make a copies here so that we are not affected by the deselection
            Model::EntityBrushesMap frontBrushes, backBrushes;
            using std::swap;
            swap(frontBrushes, m_frontBrushes);
            swap(backBrushes, m_backBrushes);
            
            const Model::ObjectList objects = document()->selectedObjects();
            
            const UndoableCommandGroup commandGroup(controller(), objects.size() == 1 ? "Clip Brush" : "Clip Brushes");
            controller()->deselectAll();
            controller()->removeObjects(objects);

            if (!frontBrushes.empty() && m_clipper.keepFrontBrushes()) {
                const Model::ObjectParentList brushes = Model::makeObjectParentList(frontBrushes);
                controller()->addObjects(brushes);
                controller()->selectObjects(makeObjectList(brushes));
                frontBrushes.clear();
            } else {
                clearAndDelete(frontBrushes);
            }
            if (!backBrushes.empty() && m_clipper.keepBackBrushes()) {
                const Model::ObjectParentList brushes = Model::makeObjectParentList(backBrushes);
                controller()->addObjects(brushes);
                controller()->selectObjects(makeObjectList(brushes));
                backBrushes.clear();
            } else {
                clearAndDelete(backBrushes);
            }
            
            m_clipper.reset();
            updateBrushes();
        }

        bool ClipTool::canDeleteLastClipPoint() const {
            return m_clipper.numPoints() > 0;
        }
        
        void ClipTool::deleteLastClipPoint() {
            m_clipper.deleteLastClipPoint();
            updateBrushes();
        }

        bool ClipTool::initiallyActive() const {
            return false;
        }
        
        bool ClipTool::doActivate(const InputState& inputState) {
            if (!document()->hasSelectedBrushes())
                return false;
            m_clipper.reset();
            updateBrushes();
            bindObservers();
            return true;
        }
        
        bool ClipTool::doDeactivate(const InputState& inputState) {
            clearAndDelete(m_frontBrushes);
            clearAndDelete(m_backBrushes);
            unbindObservers();
            return true;
        }

        void ClipTool::doPick(const InputState& inputState, Hits& hits) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const FloatType radius = 2.0 * prefs.get(Preferences::HandleRadius);
            const FloatType scaling = prefs.get(Preferences::HandleScalingFactor);
            const FloatType maxDist = prefs.get(Preferences::MaximumHandleDistance);
            const Ray3& ray = inputState.pickRay();
            
            const Vec3::List clipPoints = m_clipper.clipPointPositions();
            for (size_t i = 0; i < clipPoints.size(); ++i) {
                const FloatType dist = ray.intersectWithSphere(clipPoints[i],
                                                               radius, scaling, maxDist);
                if (!Math::isnan(dist)) {
                    const Vec3 hitPoint = ray.pointAtDistance(dist);
                    hits.addHit(Hit(HandleHit, dist, hitPoint, i));
                }
            }
        }

        bool ClipTool::doMouseUp(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;

            const Hit& hit = findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
            if (hit.isMatch()) {
                const Vec3 point = clipPoint(hit);
                if (m_clipper.canAddClipPoint(point)) {
                    m_clipper.addClipPoint(point, *Model::hitAsFace(hit));
                    updateBrushes();
                }
            }
            
            return true;
        }
        
        bool ClipTool::doStartMouseDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            const Hit& hit = inputState.hits().findFirst(HandleHit, true);
            if (!hit.isMatch())
                return false;
            
            m_dragPointIndex = hit.target<size_t>();
            return true;
        }
        
        bool ClipTool::doMouseDrag(const InputState& inputState) {
            const Hit& hit = findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
            if (hit.isMatch()) {
                const Vec3 point = clipPoint(hit);
                if (m_clipper.canUpdateClipPoint(m_dragPointIndex, point)) {
                    m_clipper.updateClipPoint(m_dragPointIndex, point, *Model::hitAsFace(hit));
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
            renderContext.setForceHideSelectionGuide();
        }

        void ClipTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            m_renderer.renderBrushes(renderContext);
            m_renderer.renderClipPoints(renderContext);
            
            if (dragging()) {
                m_renderer.renderHighlight(renderContext, m_dragPointIndex);
            } else {
                const Hit& brushHit = findFirstHit(inputState.hits(), Model::Brush::BrushHit, document()->filter(), true);
                const Hit& handleHit = inputState.hits().findFirst(HandleHit, true);

                if (handleHit.isMatch()) {
                    const size_t index = handleHit.target<size_t>();
                    m_renderer.renderHighlight(renderContext, index);
                } else if (brushHit.isMatch()) {
                    const Vec3 point = clipPoint(brushHit);
                    if (m_clipper.canAddClipPoint(point))
                        m_renderer.renderCurrentPoint(renderContext, point);
                }
            }
        }
        
        Vec3 ClipTool::clipPoint(const Hit& hit) const {
            const Model::BrushFace& face = *Model::hitAsFace(hit);
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
            } else {
                m_clipper.reset();
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

        void ClipTool::bindObservers() {
            document()->selectionDidChangeNotifier.addObserver(this, &ClipTool::selectionDidChange);
            document()->objectWillChangeNotifier.addObserver(this, &ClipTool::objectWillChange);
            document()->objectDidChangeNotifier.addObserver(this, &ClipTool::objectDidChange);
        }
        
        void ClipTool::unbindObservers() {
            if (!expired(document())) {
                document()->selectionDidChangeNotifier.removeObserver(this, &ClipTool::selectionDidChange);
                document()->objectWillChangeNotifier.removeObserver(this, &ClipTool::objectWillChange);
                document()->objectDidChangeNotifier.removeObserver(this, &ClipTool::objectDidChange);
            }
        }
        
        void ClipTool::selectionDidChange(const Model::SelectionResult& selection) {
            updateBrushes();
        }
        
        void ClipTool::objectWillChange(Model::Object* object) {
            updateBrushes();
        }
        
        void ClipTool::objectDidChange(Model::Object* object) {
            updateBrushes();
        }
    }
}
