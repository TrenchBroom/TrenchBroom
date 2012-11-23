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

#include "ClipTool.h"

#include "Model/Picker.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SphereFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "View/EditorView.h"
#include "Utility/Grid.h"

namespace TrenchBroom {
    namespace Model {
        ClipHandleHit::ClipHandleHit(const Vec3f& hitPoint, float distance, unsigned int index) :
        Hit(HitType::ClipHandleHit, hitPoint, distance),
        m_index(index) {
            assert(m_index < 3);
        }
        
        bool ClipHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        bool ClipTool::handleIsModal(InputState& inputState) {
            return true;
        }

        void ClipTool::handlePick(InputState& inputState) {
            for (unsigned int i = 0; i < m_numPoints; i++) {
                float distance = inputState.pickRay().intersectWithSphere(m_points[i], m_handleRadius);
                if (!Math::isnan(distance)) {
                    Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                    inputState.pickResult().add(new Model::ClipHandleHit(hitPoint, distance, i));
                }
            }
        }

        bool ClipTool::handleUpdateState(InputState& inputState) {
            bool updateFeedback = false;
            
            Model::ClipHandleHit* handleHit = static_cast<Model::ClipHandleHit*>(inputState.pickResult().first(Model::HitType::ClipHandleHit, true, m_filter));
            if (handleHit != NULL) {
                updateFeedback |= m_directHit == false || m_hitIndex != handleHit->index();
                m_hitIndex = handleHit->index();
                m_directHit = true;
                return updateFeedback;
            } else {
                updateFeedback |= m_directHit == true;
                m_directHit = false;
            }

            Model::FaceHit* faceHit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, m_filter));
            if (faceHit == NULL) {
                updateFeedback |= m_hitIndex != -1;
                m_hitIndex = -1;
                return updateFeedback;
            }
            
            Utility::Grid& grid = document().grid();
            const Model::Face& face = faceHit->face();
            Vec3f point = grid.snap(faceHit->hitPoint(), face.boundary());
            Vec3f dir = point - inputState.pickRay().origin;
            dir.normalize();
            
            Ray testRay(inputState.pickRay().origin, dir);
            if (!face.side()->intersectWithRay(testRay)) {
                updateFeedback |= m_hitIndex != -1;
                m_hitIndex = -1;
                return updateFeedback;
            }
            
            for (unsigned int i = 0; i < m_numPoints; i++) {
                if (point.equals(m_points[i])) {
                    updateFeedback |= m_hitIndex != i;
                    m_hitIndex = i;
                    return updateFeedback;
                }
            }
            
            if (m_numPoints < 3) {
                updateFeedback |= (m_hitIndex != m_numPoints || !m_points[m_numPoints].equals(point));
                m_hitIndex = m_numPoints;
                m_points[m_numPoints] = point;
                return updateFeedback;
            }

            updateFeedback |= m_hitIndex != -1;
            m_hitIndex = -1;
            return updateFeedback;
        }
        
        void ClipTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (m_numPoints == 0 && m_hitIndex == -1)
                return;
            
            Model::ClipHandleHit* handleHit = static_cast<Model::ClipHandleHit*>(inputState.pickResult().first(Model::HitType::ClipHandleHit, true, m_filter));
            
            glDisable(GL_DEPTH_TEST);
            Renderer::ActivateShader shader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
            Renderer::SphereFigure sphereFigure(m_handleRadius, 3);
            for (unsigned int i = 0; i < m_numPoints; i++) {
                if (handleHit != NULL && handleHit->index() == i)
                    shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                else
                    shader.currentShader().setUniformVariable("Color", Color(0.0f, 1.0f, 0.0f, 1.0f));
                Renderer::ApplyMatrix translate(renderContext.transformation(), Mat4f().translate(m_points[i]));
                sphereFigure.render(vbo, renderContext);
            }
            
            if (m_hitIndex == m_numPoints && m_numPoints < 3) {
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::ApplyMatrix translate(renderContext.transformation(), Mat4f().translate(m_points[m_hitIndex]));
                sphereFigure.render(vbo, renderContext);
            }
            glEnable(GL_DEPTH_TEST);
        }
        
        bool ClipTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft)
                return false;
            if (m_hitIndex == -1 || m_numPoints == 3)
                return false;

            m_numPoints++;
            m_hitIndex = -1;
            return true;
        }

        ClipTool::ClipTool(View::DocumentViewHolder& documentViewHolder) :
        Tool(documentViewHolder, true),
        m_handleRadius(2.5f),
        m_filter(view().filter()),
        m_numPoints(0),
        m_hitIndex(-1),
        m_directHit(false) {}
    }
}