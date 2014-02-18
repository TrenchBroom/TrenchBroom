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
            
            const Vec3::List normals = findApplicablePlaneNormals(selectedFaces);
            const Model::BrushFaceList faces = selectApplicableFaces(selectedFaces, normals);
            performMove(delta, faces, normals);
        }
        
        Vec3::List TextureTool::findApplicablePlaneNormals(const Model::BrushFaceList& faces) const {
            assert(m_face != NULL);
            size_t counts[3] = { 0, 0, 0 };
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Model::BrushFace* face = *it;
                countPossibleAxes(face->boundary().normal, counts);
            }
            
            return selectApplicablePlaneNormals(counts, m_face);
        }

        Vec3::List TextureTool::selectApplicablePlaneNormals(const size_t (&counts)[3], const Model::BrushFace* face) const {
            Vec3::List result;
            
            const size_t count = countPossibleAxes(counts);
            if (count == 1) {
                result.push_back(face->boundary().normal.firstAxis());
            } else if (count == 2) {
                for (size_t i = 0; i < 3; ++i) {
                    if (counts[i] > 0) {
                        Vec3 pos;
                        pos[i] = 1.0;
                        result.push_back(pos);
                        result.push_back(-pos);
                    }
                }
            } else {
                const Vec3& faceNormal = face->boundary().normal;
                if (countPossibleAxes(faceNormal) == 1 &&
                    faceNormal.firstComponent() == Math::Axis::AZ) {
                    result.push_back(faceNormal.firstAxis());
                } else {
                    result.push_back(Vec3::PosX);
                    result.push_back(Vec3::NegX);
                    result.push_back(Vec3::PosY);
                    result.push_back(Vec3::NegY);
                }
            }
            
            assert(!result.empty());
            return result;
        }

        size_t TextureTool::countPossibleAxes(const Vec3& normal) const {
            size_t counts[3] = { 0, 0, 0};
            countPossibleAxes(normal, counts);
            return countPossibleAxes(counts);
        }

        void TextureTool::countPossibleAxes(const Vec3& normal, size_t (&counts)[3]) const {
            const size_t comp1 = normal.firstComponent();
            const size_t comp2 = normal.secondComponent();
            const size_t comp3 = normal.thirdComponent();
            const FloatType val1 = normal[comp1];
            const FloatType val2 = normal[comp2];
            const FloatType val3 = normal[comp3];
            
            ++counts[comp1];
            if (Math::eq(std::abs(val1), std::abs(val2))) {
                ++counts[comp2];
                if (Math::eq(std::abs(val2), std::abs(val3)))
                    ++counts[comp3];
            }
        }

        size_t TextureTool::countPossibleAxes(const size_t (&counts)[3]) const {
            size_t count = 0;
            for (size_t i = 0; i < 3; ++i)
                if (counts[i] > 0)
                    ++count;
            return count;
        }

        Model::BrushFaceList TextureTool::selectApplicableFaces(const Model::BrushFaceList& faces, const Vec3::List& normals) const {
            Model::BrushFaceList::const_iterator it, end;
            Model::BrushFaceList result;
            
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                const Vec3& faceNormal = face->boundary().normal;
                const Vec3 firstAxis = faceNormal.firstAxis();
                if (VectorUtils::contains(normals, firstAxis))
                    result.push_back(face);
            }
            
            return result;
        }

        void TextureTool::performMove(const Vec3& delta, const Model::BrushFaceList& faces, const Vec3::List& normals) {
            const Grid& grid = document()->grid();

            controller()->beginUndoableGroup("Move Texture");
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                const Vec3 actualDelta = rotateDelta(delta, face, normals);
                const Vec2 offset = grid.snap(face->convertToTexCoordSystem(actualDelta));
                
                const Model::BrushFaceList applyTo(1, face);
                if (offset.x() != 0.0)
                    controller()->setFaceXOffset(applyTo, -offset.x(), true);
                if (offset.y() != 0.0)
                    controller()->setFaceYOffset(applyTo, -offset.y(), true);
            }
            controller()->closeGroup();
        }

        Vec3 TextureTool::rotateDelta(const Vec3& delta, const Model::BrushFace* face, const Vec3::List& normals) const {
            assert(m_face != NULL);
            
            const Vec3& reference = m_face->boundary().normal.firstAxis();
            const Vec3 faceNormal = disambiguateNormal(face, normals);
            if (reference == faceNormal)
                return delta;

            const Quat3 rotation(reference, faceNormal);
            return rotation * delta;
        }

        Vec3 TextureTool::disambiguateNormal(const Model::BrushFace* face, const Vec3::List& normals) const {
            const Vec3& faceNormal = face->boundary().normal;

            const Vec3 firstAxis = faceNormal.firstAxis();
            if (VectorUtils::contains(normals, firstAxis))
                return firstAxis;
            
            const Vec3 secondAxis = faceNormal.secondAxis();
            if (VectorUtils::contains(normals, secondAxis))
                return secondAxis;
            
            const Vec3 thirdAxis = faceNormal.thirdAxis();
            assert(VectorUtils::contains(normals, thirdAxis));
            return thirdAxis;
        }
    }
}
