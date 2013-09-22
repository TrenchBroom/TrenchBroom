/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ResizeBrushesTool.h"

#include "Controller/ResizeBrushesCommand.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/EditStateManager.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Model {
        DragFaceHit::DragFaceHit(const Vec3f& hitPoint, float distance, Face& dragFace) :
        Hit(HitType::DragFaceHit, hitPoint, distance),
        m_dragFace(dragFace) {}
    }

    namespace Controller {
        Model::FaceList ResizeBrushesTool::dragFaces(Model::Face& dragFace) {
            Model::FaceList result;
            result.push_back(&dragFace);

            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::BrushList& selectedBrushes = editStateManager.selectedBrushes();
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = selectedBrushes.begin(), brushEnd = selectedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                const Model::FaceList& faces = brush.faces();
                Model::FaceList::const_iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                    Model::Face& face = **faceIt;
                    if (&face != &dragFace && face.boundary().equals(dragFace.boundary()))
                        result.push_back(&face);
                }
            }

            return result;
        }

        bool ResizeBrushesTool::handleIsModal(InputState& inputState) {
            return (inputState.modifierKeys() == ModifierKeys::MKShift ||
                    inputState.modifierKeys() == (ModifierKeys::MKShift | ModifierKeys::MKAlt));
        }

        void ResizeBrushesTool::handlePick(InputState& inputState) {
            if (inputState.modifierKeys() != ModifierKeys::MKShift)
                return;

            Model::FaceHit* faceHit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, m_filter));
            if (faceHit != NULL) {
                inputState.pickResult().add(new Model::DragFaceHit(faceHit->hitPoint(), faceHit->distance(), faceHit->face()));
            } else {
                float closestEdgeDist = std::numeric_limits<float>::max();
                const Model::Edge* closestEdge = NULL;
                Model::Face* dragFace = NULL;
                Vec3f hitPoint;
                float hitDistance = 0.0f;
                
                Model::EditStateManager& editStateManager = document().editStateManager();
                const Model::BrushList& selectedBrushes = editStateManager.selectedBrushes();
                

                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = selectedBrushes.begin(), brushEnd = selectedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    const Model::EdgeList& edges = brush.edges();
                    Model::EdgeList::const_iterator edgeIt, edgeEnd;
                    for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                        const Model::Edge* edge = *edgeIt;

                        float leftDot = edge->left->face->boundary().normal.dot(inputState.pickRay().direction);
                        float rightDot = edge->right->face->boundary().normal.dot(inputState.pickRay().direction);
                        if ((leftDot > 0.0f) != (rightDot > 0.0f)) {
                            Vec3f pointOnSegment;
                            float distanceToClosestPointOnRay;
                            float distanceBetweenRayAndEdge = inputState.pickRay().distanceToSegment(edge->start->position,
                                                                                                     edge->end->position,
                                                                                                     pointOnSegment,
                                                                                                     distanceToClosestPointOnRay);
                            if (!Math<float>::isnan(distanceBetweenRayAndEdge) && distanceBetweenRayAndEdge < closestEdgeDist) {
                                closestEdge = edge;
                                closestEdgeDist = distanceBetweenRayAndEdge;
                                hitDistance = distanceToClosestPointOnRay;
                                hitPoint = inputState.pickRay().pointAtDistance(hitDistance);
                                if (leftDot > rightDot) {
                                    dragFace = edge->left->face;
                                } else {
                                    dragFace = edge->right->face;
                                }
                            }
                        }
                    }
                }
                
                if (closestEdge != NULL)
                    inputState.pickResult().add(new Model::DragFaceHit(hitPoint, hitDistance, *dragFace));
            }
        }

        void ResizeBrushesTool::handleRenderOverlay(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::FaceList faces;
            if (dragType() != DTDrag) {
                if (inputState.modifierKeys() != ModifierKeys::MKShift)
                    return;

                Model::DragFaceHit* hit = static_cast<Model::DragFaceHit*>(inputState.pickResult().first(Model::HitType::DragFaceHit, true, m_filter));
                if (DTDrag && hit == NULL)
                    return;

                faces = dragFaces(hit->dragFace());
            } else {
                faces = m_faces;
            }

            Model::FaceList::const_iterator faceIt, faceEnd;

            unsigned int vertexCount = 0;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face& face = **faceIt;
                vertexCount += static_cast<unsigned int>(2 * face.edges().size());
            }

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::VertexArray edgeArray(vbo, GL_LINES, vertexCount, Renderer::Attribute::position3f());
            Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);

            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face& face = **faceIt;
                const Model::EdgeList& edges = face.edges();
                Model::EdgeList::const_iterator eIt, eEnd;
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    const Model::Edge& edge = **eIt;
                    edgeArray.addAttribute(edge.start->position);
                    edgeArray.addAttribute(edge.end->position);
                }
            }

            Renderer::glSetEdgeOffset(0.3f);

            Renderer::SetVboState activateVbo(vbo, Renderer::Vbo::VboActive);
            Renderer::ActivateShader shader(renderContext.shaderManager(), Renderer::Shaders::EdgeShader);

            glDisable(GL_DEPTH_TEST);
            shader.setUniformVariable("Color", prefs.getColor(Preferences::ResizeBrushFaceColor));
            edgeArray.render();
            glEnable(GL_DEPTH_TEST);

            Renderer::glResetEdgeOffset();
        }
        
        bool ResizeBrushesTool::handleStartDrag(InputState& inputState) {
            if (inputState.modifierKeys() != ModifierKeys::MKShift)
                return false;
            
            Model::DragFaceHit* hit = static_cast<Model::DragFaceHit*>(inputState.pickResult().first(Model::HitType::DragFaceHit, true, m_filter));
            if (hit == NULL)
                return false;
            
            m_dragOrigin = hit->hitPoint();
            m_totalDelta = Vec3f::Null;
            m_faces = dragFaces(hit->dragFace());
            
            beginCommandGroup(wxT("Resize Brush"));
            return true;
        }
        
        bool ResizeBrushesTool::handleDrag(InputState& inputState) {
            assert(!m_faces.empty());

            const Planef dragPlane = Planef::orthogonalDragPlane(m_dragOrigin, inputState.camera().direction());
            
            Model::Face& dragFace = *m_faces.front();
            const Vec3f& faceNormal3D = dragFace.boundary().normal;
            const Vec3f faceNormal2D = dragPlane.project(faceNormal3D);
            const float rayPointDistance = dragPlane.intersectWithRay(inputState.pickRay());
            const Vec3f rayPoint = inputState.pickRay().pointAtDistance(rayPointDistance);
            const Vec3f dragVector2D = rayPoint - m_dragOrigin;
            
            const float dragDistance = dragVector2D.dot(faceNormal2D);
            
            Utility::Grid& grid = document().grid();
            const Vec3f relativeFaceDelta = grid.snap(dragDistance) * faceNormal3D;
            const Vec3f absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal3D * dragDistance);
            
            // select the delta that is closest to the actual delta indicated by the mouse cursor
            const Vec3f faceDelta = (std::abs(relativeFaceDelta.length() - dragDistance) <
                                     std::abs(absoluteFaceDelta.length() - dragDistance) ?
                                     relativeFaceDelta :
                                     absoluteFaceDelta);

            if (faceDelta.null())
                return true;

            ResizeBrushesCommand* command = ResizeBrushesCommand::resizeBrushes(document(), m_faces, faceDelta, document().textureLock());
            if (submitCommand(command)) {
                m_totalDelta += faceDelta;
                m_dragOrigin += faceDelta;
            }
            return true;
        }
        
        void ResizeBrushesTool::handleEndDrag(InputState& inputState) {
            if (m_totalDelta.null())
                rollbackCommandGroup();
            endCommandGroup();
            m_faces.clear();
        }
        
        void ResizeBrushesTool::handleCancelDrag(InputState& inputState) {
            rollbackCommandGroup();
            endCommandGroup();
            m_faces.clear();
        }

        ResizeBrushesTool::ResizeBrushesTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        Tool(documentViewHolder, inputController, true),
        m_filter(Model::SelectedFilter(view().filter())) {}
    }
}
