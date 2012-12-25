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
        NearEdgeHit::NearEdgeHit(const Vec3f& hitPoint, float distance, Face& dragFace, Face& referenceFace) :
        Hit(HitType::NearEdgeHit, hitPoint, distance),
        m_dragFace(dragFace),
        m_referenceFace(referenceFace) {}
        
        bool NearEdgeHit::pickable(Filter& filter) const {
            return true;
        }
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
            return inputState.modifierKeys() == ModifierKeys::MKShift;
        }

        void ResizeBrushesTool::handlePick(InputState& inputState) {
            if (inputState.modifierKeys() != ModifierKeys::MKShift)
                return;
            
            float closestEdgeDist = std::numeric_limits<float>::max();
            const Model::Edge* closestEdge = NULL;
            Model::Face* dragFace = NULL;
            Model::Face* otherFace = NULL;
            Vec3f hitPoint;
            float hitDistance = 0.0f;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::BrushList& selectedBrushes = editStateManager.selectedBrushes();
            
            Model::FaceHit* faceHit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, m_filter));
            if (faceHit != NULL) {
                const Model::Face& face = faceHit->face();
                const Model::EdgeList& edges = face.edges();

                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::Edge* edge = *edgeIt;
                    
                    Vec3f pointOnSegment;
                    float distanceToClosestPointOnRay;
                    float distanceBetweenRayAndEdge = inputState.pickRay().distanceToSegment(edge->start->position,
                                                                                             edge->end->position,
                                                                                             pointOnSegment,
                                                                                             distanceToClosestPointOnRay);
                    if (!Math::isnan(distanceBetweenRayAndEdge) && distanceBetweenRayAndEdge < closestEdgeDist) {
                        closestEdge = edge;
                        closestEdgeDist = distanceBetweenRayAndEdge;
                        hitDistance = distanceToClosestPointOnRay;
                        hitPoint = inputState.pickRay().pointAtDistance(hitDistance);
                        if (edge->left->face == &face) {
                            dragFace = edge->left->face;
                            otherFace = edge->right->face;
                        } else {
                            dragFace = edge->right->face;
                            otherFace = edge->left->face;
                        }
                    }
                }
            } else {
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = selectedBrushes.begin(), brushEnd = selectedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    const Model::EdgeList& edges = brush.edges();
                    Model::EdgeList::const_iterator edgeIt, edgeEnd;
                    for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                        const Model::Edge* edge = *edgeIt;
                        
                        float leftDot = edge->left->face->boundary().normal.dot(inputState.pickRay().direction);
                        float rightDot = edge->right->face->boundary().normal.dot(inputState.pickRay().direction);
                        if (leftDot > 0.0f != rightDot > 0.0f) {
                            Vec3f pointOnSegment;
                            float distanceToClosestPointOnRay;
                            float distanceBetweenRayAndEdge = inputState.pickRay().distanceToSegment(edge->start->position,
                                                                                                     edge->end->position,
                                                                                                     pointOnSegment,
                                                                                                     distanceToClosestPointOnRay);
                            if (!Math::isnan(distanceBetweenRayAndEdge) && distanceBetweenRayAndEdge < closestEdgeDist) {
                                closestEdge = edge;
                                closestEdgeDist = distanceBetweenRayAndEdge;
                                hitDistance = distanceToClosestPointOnRay;
                                hitPoint = inputState.pickRay().pointAtDistance(hitDistance);
                                if (leftDot > 0.0f) {
                                    dragFace = edge->left->face;
                                    otherFace = edge->right->face;
                                } else {
                                    dragFace = edge->right->face;
                                    otherFace = edge->left->face;
                                }
                            }
                        }
                    }
                }
            }
            
            if (closestEdge != NULL) {
                assert(dragFace != NULL);
                assert(otherFace != NULL);
                inputState.pickResult().add(new Model::NearEdgeHit(hitPoint, hitDistance, *dragFace, *otherFace));
            }
        }
        
        void ResizeBrushesTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Model::FaceList faces;
            if (dragType() != DTDrag) {
                if (inputState.modifierKeys() != ModifierKeys::MKShift)
                    return;
                
                Model::NearEdgeHit* hit = static_cast<Model::NearEdgeHit*>(inputState.pickResult().first(Model::HitType::NearEdgeHit, true, m_filter));
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
            shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedResizeBrushFaceColor));
            edgeArray.render();
            glEnable(GL_DEPTH_TEST);
            
            shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::ResizeBrushFaceColor));
            edgeArray.render();
            
            Renderer::glResetEdgeOffset();
        }
        
        bool ResizeBrushesTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            if (inputState.modifierKeys() != ModifierKeys::MKShift)
                return false;
            
            Model::NearEdgeHit* hit = static_cast<Model::NearEdgeHit*>(inputState.pickResult().first(Model::HitType::NearEdgeHit, true, m_filter));
            if (hit == NULL)
                return false;
            
            Model::Face& dragFace = hit->dragFace();

            const Vec3f& dragNormal = dragFace.boundary().normal;
            Vec3f planeNormal = dragNormal.crossed(inputState.pickRay().direction);
            if (planeNormal.null())
                return false;

            planeNormal = dragNormal.crossed(planeNormal);
            planeNormal.normalize();
            plane = Plane(planeNormal, hit->hitPoint());

            m_faces = dragFaces(dragFace);
            initialPoint = hit->hitPoint();
            m_totalDelta = Vec3f::Null;
            
            beginCommandGroup(wxT("Resize Brush"));
            return true;
        }
        
        bool ResizeBrushesTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            assert(!m_faces.empty());
            
            const Vec3f planeDelta = curPoint - refPoint;
            if (planeDelta.null())
                return true;
            
            Utility::Grid& grid = document().grid();
            Model::Face& dragFace = *m_faces.front();
            const Vec3f& faceAxis = dragFace.boundary().normal.firstAxis();
            const float faceDist = planeDelta.dot(faceAxis);
            const Vec3f faceDelta = grid.snap(faceDist * faceAxis);
            
            if (faceDelta.null())
                return true;
            
            ResizeBrushesCommand* command = ResizeBrushesCommand::resizeBrushes(document(), m_faces, faceDelta, document().textureLock());
            if (submitCommand(command)) {
                const Vec3f planeDir = planeDelta.normalized();
                const float planeDist = faceDelta.dot(planeDir);
                refPoint += planeDist * planeDir;
                m_totalDelta += faceDelta;
            }
            return true;
        }
        
        void ResizeBrushesTool::handleEndPlaneDrag(InputState& inputState) {
            if (m_totalDelta.null())
                rollbackCommandGroup();
            else
                endCommandGroup();
            m_faces.clear();
        }
        
        ResizeBrushesTool::ResizeBrushesTool(View::DocumentViewHolder& documentViewHolder) :
        PlaneDragTool(documentViewHolder, false),
        m_filter(ResizeBrushesFilter(view().filter())) {}
    }
}
