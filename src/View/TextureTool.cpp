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

#include "TextureTool.h"

#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"
#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TextureTool::TextureTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller) :
        Tool(next, document, controller),
        m_face(NULL) {}

        bool TextureTool::initiallyActive() const {
            return false;
        }
        
        bool TextureTool::doActivate(const InputState& inputState) {
            return true;
        }
        
        bool TextureTool::doDeactivate(const InputState& inputState) {
            return true;
        }
        
        void TextureTool::doMouseMove(const InputState& inputState) {
        }

        bool TextureTool::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_face == NULL);
            if (!applies(inputState))
                return false;

            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            assert(first.matches);
            
            m_face = Model::hitAsFace(first.hit);
            plane = Plane3(first.hit.hitPoint(), m_face->boundary().normal);
            initialPoint = first.hit.hitPoint();
            return true;
        }
        
        bool TextureTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_face != NULL);
            assert(m_face->selected() || m_face->parent()->selected());
            
            const Vec2 last = m_face->convertToTexCoordSystem(refPoint);
            const Vec2 cur = m_face->convertToTexCoordSystem(curPoint);

            const Grid& grid = document()->grid();
            const Vec2 offset = grid.snap(cur - last);
            
            if (offset.null())
                return true;

            const Vec3 delta = curPoint - refPoint;
            performMove(delta);
            
            const Vec3 newRef = m_face->convertToWorldCoordSystem(last + offset);
            refPoint = newRef;
            return true;
        }
        
        void TextureTool::doEndPlaneDrag(const InputState& inputState) {
            m_face = NULL;
        }
        
        void TextureTool::doCancelPlaneDrag(const InputState& inputState) {
            m_face = NULL;
        }

        void TextureTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.clearTintSelection();
        }
        
        void TextureTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            const Model::BrushFace* face = NULL;
            if (dragging()) {
                assert(m_face != NULL);
                face = m_face;
            } else {
                const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
                if (first.matches) {
                    face = Model::hitAsFace(first.hit);
                    if (!face->selected() && !face->parent()->selected())
                        face = NULL;
                }
            }
            
            if (face == NULL)
                return;

            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::EdgeRenderer edgeRenderer = buildEdgeRenderer(face);
            
            glDisable(GL_DEPTH_TEST);
            edgeRenderer.setUseColor(true);
            edgeRenderer.setColor(prefs.get(Preferences::ResizeHandleColor));
            edgeRenderer.render(renderContext);
            glEnable(GL_DEPTH_TEST);
        }
        
        Renderer::EdgeRenderer TextureTool::buildEdgeRenderer(const Model::BrushFace* face) const {
            assert(face != NULL);
            
            const Model::BrushEdgeList& edges = face->edges();

            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices(2 * edges.size());

            for (size_t i = 0; i < edges.size(); ++i) {
                const Model::BrushEdge* edge = edges[i];
                vertices[2 * i + 0] = Vertex(edge->start->position);
                vertices[2 * i + 1] = Vertex(edge->end->position);
            }
            
            return Renderer::EdgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
        }

        bool TextureTool::applies(const InputState& inputState) const {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            if (!first.matches)
                return false;
            const Model::BrushFace* face = Model::hitAsFace(first.hit);
            const Model::Brush* brush = face->parent();
            return face->selected() || brush->selected();
        }

        void TextureTool::performMove(const Vec3& delta) {
            const Model::BrushFaceList& selectedFaces = document()->allSelectedFaces();
            
            const TDragPlane dragPlane = findDragPlane(selectedFaces);
            const Model::BrushFaceList faces = selectApplicableFaces(selectedFaces, dragPlane);
            performMove(delta, faces, dragPlane);
        }
        
        TextureTool::TDragPlane TextureTool::findDragPlane(const Model::BrushFaceList& faces) const {
            bool axes[3] = { true, true, true };
            
            Model::BrushFaceList::const_iterator it = faces.begin();
            const Model::BrushFaceList::const_iterator end = faces.end();
            assert(it != end);
            restrictDragPlanes((*it)->boundary().normal, axes);
            while (++it != end) {
                const Model::BrushFace* face = *it;
                restrictDragPlanes(face->boundary().normal, axes);
            }
            
            const size_t planeNormals = countPossibleDragPlanes(axes);
            if (planeNormals == 1)
                return selectUniqueDragPlane(axes);
            
            return selectUniqueDragPlane(m_face->boundary().normal);
        }

        void TextureTool::restrictDragPlanes(const Vec3& normal, bool (&axes)[3]) const {
            const size_t count = countPossibleDragPlanes(normal);
            assert(count > 0);
            
            const size_t first = normal.firstComponent();
            axes[first] = false;

            if (count == 2) {
                const size_t second = normal.secondComponent();
                axes[second] = false;
            }
        }

        size_t TextureTool::countPossibleDragPlanes(const Vec3& normal) const {
            const size_t comp1 = normal.firstComponent();
            const size_t comp2 = normal.secondComponent();
            const size_t comp3 = normal.thirdComponent();
            const FloatType val1 = normal[comp1];
            const FloatType val2 = normal[comp2];
            const FloatType val3 = normal[comp3];
            
            if (Math::gt(std::abs(val1), std::abs(val2)))
                return 1;
            if (Math::gt(std::abs(val2), std::abs(val3)))
                return 2;
            return 3;
        }

        size_t TextureTool::countPossibleDragPlanes(const bool (&axes)[3]) const {
            size_t count = 0;
            for (size_t i = 0; i < 3; ++i)
                if (axes[i])
                    ++count;
            return count;
        }

        TextureTool::TDragPlane TextureTool::selectUniqueDragPlane(const Vec3& normal) const {
            const size_t count = countPossibleDragPlanes(normal);
            if (count == 1) {
                return getDragPlane(normal);
            } else if (count == 2 &&
                       getDragPlane(normal.firstAxis()) != DPVertical &&
                       getDragPlane(normal.secondAxis()) != DPVertical) {
                return DPHorizontal;
            }
            return DPVertical;
        }

        TextureTool::TDragPlane TextureTool::getDragPlane(const Vec3& vec) const {
            const Math::Axis::Type first = vec.firstComponent();
            return first == Math::Axis::AZ ? DPHorizontal : DPVertical;
        }

        TextureTool::TDragPlane TextureTool::selectUniqueDragPlane(const bool (&axes)[3]) const {
            assert(axes[0] ^ axes[1] ^ axes[2]);
            return axes[2] ? DPVertical : DPHorizontal;
        }

        Model::BrushFaceList TextureTool::selectApplicableFaces(const Model::BrushFaceList& faces, const TDragPlane dragPlane) const {
            Model::BrushFaceList::const_iterator it, end;
            Model::BrushFaceList result;
            
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                const Vec3& faceNormal = face->boundary().normal;
                const TDragPlane faceDragPlane = selectUniqueDragPlane(faceNormal);
                if (faceDragPlane == dragPlane)
                    result.push_back(face);
            }
            
            return result;
        }

        void TextureTool::performMove(const Vec3& delta, const Model::BrushFaceList& faces, const TDragPlane dragPlane) {
            const Grid& grid = document()->grid();

            controller()->beginUndoableGroup("Move Texture");
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                const Vec3 actualDelta = rotateDelta(delta, face, dragPlane);
                const Vec2 offset = grid.snap(face->convertToTexCoordSystem(actualDelta));
                
                const Model::BrushFaceList applyTo(1, face);
                if (offset.x() != 0.0)
                    controller()->setFaceXOffset(applyTo, -offset.x(), true);
                if (offset.y() != 0.0)
                    controller()->setFaceYOffset(applyTo, -offset.y(), true);
            }
            controller()->closeGroup();
        }

        Vec3 TextureTool::rotateDelta(const Vec3& delta, const Model::BrushFace* face, const TDragPlane dragPlane) const {
            assert(m_face != NULL);
            
            const Vec3& reference = m_face->boundary().normal.firstAxis();
            const Vec3 faceNormal = disambiguateNormal(face, dragPlane);
            if (reference == faceNormal)
                return delta;

            const Quat3 rotation(reference, faceNormal);
            return rotation * delta;
        }

        Vec3 TextureTool::disambiguateNormal(const Model::BrushFace* face, const TDragPlane dragPlane) const {
            const Vec3& faceNormal = face->boundary().normal;
            if (dragPlane == DPHorizontal) {
                assert(!Math::zero(faceNormal.z()));
                return Math::pos(faceNormal.z()) ? Vec3::PosZ : Vec3::NegZ;
            }
            
            const Vec3 firstAxis = faceNormal.firstAxis();
            assert(firstAxis.z() == 0.0);
            return firstAxis;
        }
    }
}
