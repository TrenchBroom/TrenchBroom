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
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        bool ResizeBrushesTool::handleIsModal(InputState& inputState) {
            return inputState.modifierKeys() == ModifierKeys::MKCtrlCmd;
        }

        void ResizeBrushesTool::handlePick(InputState& inputState) {
            if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::BrushList& selectedBrushes = editStateManager.selectedBrushes();
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = selectedBrushes.begin(), brushEnd = selectedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                const Model::EdgeList& edges = brush.edges();
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::Edge& edge = **edgeIt;

                    float leftDot = edge.left->face->boundary().normal.dot(inputState.pickRay().direction);
                    float rightDot = edge.right->face->boundary().normal.dot(inputState.pickRay().direction);
                    if (leftDot > 0.0f != rightDot > 0.0f) {
                        float distanceToClosestPoint;
                        float distanceToSegment = inputState.pickRay().distanceToSegment(edge.start->position, edge.end->position, distanceToClosestPoint);
                        if (distanceToSegment <= prefs.getFloat(Preferences::MaximumNearFaceDistance)) {
                            Vec3f hitPoint = inputState.pickRay().pointAtDistance(distanceToClosestPoint);
                            Model::Face& face = leftDot > 0.0f ? *edge.left->face : *edge.right->face;
                            inputState.pickResult().add(Model::FaceHit::nearFaceHit(face, hitPoint, distanceToClosestPoint));
                        }
                    }
                }
            }
        }
        
        void ResizeBrushesTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                return;
            
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit | Model::HitType::NearFaceHit, true, m_filter));
            if (hit == NULL)
                return;

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);
            
            Model::Face& face = hit->face();
            const Model::VertexList& vertices = face.vertices();
            unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
            Renderer::VertexArray edgeArray(vbo, GL_LINE_LOOP, vertexCount, Renderer::Attribute::position3f());
            
            Model::VertexList::const_iterator vIt, vEnd;
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Model::Vertex& vertex = **vIt;
                edgeArray.addAttribute(vertex.position);
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
            if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                return false;
            
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit | Model::HitType::NearFaceHit, true, m_filter));
            if (hit == NULL)
                return false;
            
            assert(m_faces.empty());
            Model::Face& referenceFace = hit->face();
            m_faces.push_back(&referenceFace);
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::BrushList& selectedBrushes = editStateManager.selectedBrushes();
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = selectedBrushes.begin(), brushEnd = selectedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                const Model::FaceList& faces = brush.faces();
                Model::FaceList::const_iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                    Model::Face& face = **faceIt;
                    if (&face != &referenceFace && face.boundary().equals(referenceFace.boundary()))
                        m_faces.push_back(&face);
                }
            }
            
            Vec3f planeNormal = referenceFace.boundary().normal.crossed(inputState.pickRay().direction);
            planeNormal = referenceFace.boundary().normal.crossed(planeNormal);
            planeNormal.normalize();
            
            plane = Plane(planeNormal, hit->hitPoint());
            initialPoint = hit->hitPoint();
            
            beginCommandGroup(wxT("Resize Brush"));
            
            return true;
        }
        
        bool ResizeBrushesTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
            assert(!m_faces.empty());
            
            Model::Face& referenceFace = *m_faces.front();
            Vec3f planeDelta = curPoint - lastPoint;
            Vec3f faceDelta = referenceFace.boundary().normal * planeDelta.dot(referenceFace.boundary().normal);
            
            Utility::Grid& grid = document().grid();
            float distance = grid.snap(faceDelta.length());

            if (Math::zero(distance))
                return true;
            
            ResizeBrushesCommand* command = ResizeBrushesCommand::resizeBrushes(document(), m_faces, distance, document().textureLock());
            if (submitCommand(command))
                refPoint += (distance * referenceFace.boundary().normal).dot(planeDelta.normalized());
            return true;
        }
        
        void ResizeBrushesTool::handleEndPlaneDrag(InputState& inputState) {
            endCommandGroup();
            m_faces.clear();
        }

        ResizeBrushesTool::ResizeBrushesTool(View::DocumentViewHolder& documentViewHolder) :
        PlaneDragTool(documentViewHolder, false),
        m_filter(ResizeBrushesFilter(view().filter())) {}
    }
}
