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

#include "MoveVertexTool.h"

#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "Renderer/ManyCubesFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Model {
        VertexHandleHit::VertexHandleHit(const Vec3f& hitPoint, float distance, Brush& brush, const Vec3f& vertex) :
        Hit(HitType::VertexHandleHit, hitPoint, distance),
        m_brush(brush),
        m_vertex(vertex) {}
        
        bool VertexHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        bool MoveVertexTool::handleActivate(InputState& inputState) {
            m_selection.clear();
            m_vertexFigure = new Renderer::ManyCubesFigure(m_vertexHandleSize);
            m_selectedVertexFigure = new Renderer::ManyCubesFigure(m_vertexHandleSize);
            m_vertexFigureValid = false;
            return true;
        }
        
        bool MoveVertexTool::handleDeactivate(InputState& inputState) {
            m_selection.clear();
            delete m_vertexFigure;
            m_vertexFigure = NULL;
            delete m_selectedVertexFigure;
            m_selectedVertexFigure = NULL;
            return true;
        }
        
        bool MoveVertexTool::handleIsModal(InputState& inputState) {
            return true;
        }

        void MoveVertexTool::handlePick(InputState& inputState) {
            Model::MoveHandleHit* moveHandleHit = m_moveHandle.pick(inputState.pickRay());
            if (moveHandleHit != NULL)
                inputState.pickResult().add(moveHandleHit);
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                const Model::VertexList& vertices = brush.vertices();
                
                Model::VertexList::const_iterator vertexIt, vertexEnd;
                for (vertexIt = vertices.begin(), vertexEnd = vertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                    const Model::Vertex& vertex = **vertexIt;
                    float distance = inputState.pickRay().intersectWithCube(vertex.position, m_vertexHandleSize);
                    if (!Math::isnan(distance)) {
                        Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                        inputState.pickResult().add(new Model::VertexHandleHit(hitPoint, distance, brush, vertex.position));
                    }
                }
            }
        }
        
        bool MoveVertexTool::handleUpdateState(InputState& inputState) {
            return true;
        }
        
        void MoveVertexTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            assert(m_vertexFigure != NULL);
            assert(m_selectedVertexFigure != NULL);
            
            if (!m_vertexFigureValid) {
                m_vertexFigure->clear();
                m_selectedVertexFigure->clear();
                
                Model::EditStateManager& editStateManager = document().editStateManager();
                const Model::BrushList& brushes = editStateManager.selectedBrushes();
                Model::BrushList::const_iterator brushIt, brushEnd;
                
                for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    const Model::VertexList& vertices = brush.vertices();
                    bool partiallySelected = m_selection.containsBrush(&brush);
                    
                    Model::VertexList::const_iterator vertexIt, vertexEnd;
                    for (vertexIt = vertices.begin(), vertexEnd = vertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                        const Model::Vertex& vertex = **vertexIt;
                        if (partiallySelected && m_selection.containsVertex(&brush, vertex.position))
                            m_selectedVertexFigure->addCube(vertex.position);
                        else
                            m_vertexFigure->addCube(vertex.position);
                    }
                }
                
                m_vertexFigureValid = true;
            }
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::ActivateShader shader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
            
            shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::VertexHandleColor));
            m_vertexFigure->render(vbo, renderContext);
            shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::SelectedVertexHandleColor));
            m_selectedVertexFigure->render(vbo, renderContext);
            
            glDisable(GL_DEPTH_TEST);
            shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedVertexHandleColor));
            m_vertexFigure->render(vbo, renderContext);
            shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedSelectedVertexHandleColor));
            m_selectedVertexFigure->render(vbo, renderContext);
            glEnable(GL_DEPTH_TEST);
        }

        bool MoveVertexTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                (inputState.modifierKeys() != ModifierKeys::MKNone && inputState.modifierKeys() != ModifierKeys::MKCtrlCmd))
                return false;
            
            Model::VertexHandleHit* handleHit = static_cast<Model::VertexHandleHit*>(inputState.pickResult().first(Model::HitType::VertexHandleHit, true, view().filter()));
            if (handleHit == NULL)
                return false;
            
            bool selected = m_selection.containsVertex(&handleHit->brush(), handleHit->vertex());
            if (selected) {
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd)
                    m_selection.removeVertex(&handleHit->brush(), handleHit->vertex());
                m_vertexFigureValid = false;
                setNeedsUpdate();
                return true;
            }
            
            if (inputState.modifierKeys() == ModifierKeys::MKNone)
                m_selection.clear();
            m_selection.addVertex(&handleHit->brush(), handleHit->vertex());
            m_vertexFigureValid = false;
            setNeedsUpdate();
            
            return true;
        }

        bool MoveVertexTool::handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) {
            return false;
        }
        
        void MoveVertexTool::handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) {
        }
        
        void MoveVertexTool::handleEndPlaneDrag(InputState& inputState) {
        }

        void MoveVertexTool::handleObjectsChange(InputState& inputState) {
            m_selection.clear();
            m_vertexFigureValid = false;
            setNeedsUpdate();
        }

        MoveVertexTool::MoveVertexTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius, float vertexSize) :
        PlaneDragTool(documentViewHolder, true),
        m_moveHandle(axisLength, planeRadius),
        m_vertexHandleSize(vertexSize),
        m_vertexFigure(NULL),
        m_selectedVertexFigure(NULL),
        m_vertexFigureValid(false) {}
    }
}
