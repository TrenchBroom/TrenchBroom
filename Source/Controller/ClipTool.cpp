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

        bool ClipTool::handleUpdateState(InputState& inputState) {
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, m_filter));
            if (hit == NULL) {
                bool updateFeedback = m_hitIndex != -1;
                m_hitIndex = -1;
                return updateFeedback;
            }
            
            Utility::Grid& grid = document().grid();
            const Model::Face& face = hit->face();
            Vec3f point = grid.snap(hit->hitPoint(), face.boundary());
            Vec3f dir = point - inputState.pickRay().origin;
            dir.normalize();
            
            Ray testRay(inputState.pickRay().origin, dir);
            if (!face.side()->intersectWithRay(testRay)) {
                bool updateFeedback = m_hitIndex != -1;
                m_hitIndex = -1;
                return updateFeedback;
            }
            
            for (unsigned int i = 0; i < m_numPoints; i++) {
                if (point.equals(m_points[i])) {
                    bool updateFeedback = m_hitIndex != i;
                    m_hitIndex = i;
                    return updateFeedback;
                }
            }
            
            if (m_numPoints < 3) {
                bool updateFeedback = (m_hitIndex != m_numPoints || !m_points[m_numPoints].equals(point));
                m_hitIndex = m_numPoints;
                m_points[m_numPoints] = point;
                return updateFeedback;
            }
            
            return false;
        }
        
        void ClipTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (m_numPoints == 0 && m_hitIndex == -1)
                return;
            
            glDisable(GL_DEPTH_TEST);
            Renderer::ActivateShader shader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
            shader.currentShader().setUniformVariable("Color", Color(0.0f, 1.0f, 0.0f, 1.0f));
            Renderer::SphereFigure sphereFigure(3.0f, 3);
            for (unsigned int i = 0; i < m_numPoints; i++) {
                if (i != m_hitIndex) {
                    Renderer::ApplyMatrix translate(renderContext.transformation(), Mat4f().translate(m_points[i]));
                    sphereFigure.render(vbo, renderContext);
                }
            }
            
            if (m_hitIndex == m_numPoints && m_numPoints < 3) {
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                Renderer::ApplyMatrix translate(renderContext.transformation(), Mat4f().translate(m_points[m_hitIndex]));
                sphereFigure.render(vbo, renderContext);
            }
            glEnable(GL_DEPTH_TEST);
        }
        
        ClipTool::ClipTool(View::DocumentViewHolder& documentViewHolder) :
        Tool(documentViewHolder, true),
        m_filter(view().filter()),
        m_numPoints(0),
        m_hitIndex(-1) {}
    }
}