/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "FaceTool.h"

namespace TrenchBroom {
    namespace View {
        FaceTool::FaceTool(MapDocumentWPtr document) :
        VertexToolBase(document) {}
        
        Model::BrushSet FaceTool::findIncidentBrushes(const Polygon3& handle) const {
            return findIncidentBrushes(m_faceHandles, handle);
        }
        
        void FaceTool::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            m_faceHandles.pick(pickRay, camera, pickResult);
        }
        
        FaceHandleManager& FaceTool::handleManager() {
            return m_faceHandles;
        }
        
        const FaceHandleManager& FaceTool::handleManager() const {
            return m_faceHandles;
        }
        
        FaceTool::MoveResult FaceTool::move(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            
            const auto handles = m_faceHandles.selectedHandles();
            const auto brushMap = buildBrushMap(m_faceHandles, std::begin(handles), std::end(handles));
            if (document->moveFaces(brushMap, delta)) {
                m_dragHandlePosition = translate(m_dragHandlePosition, delta);
                return MR_Continue;
            }
            return MR_Deny;
        }
        
        String FaceTool::actionName() const {
            return StringUtils::safePlural(m_faceHandles.selectedHandleCount(), "Move Face", "Move Faces");
        }
        
        void FaceTool::renderHandles(const Polygon3::List& handles, Renderer::RenderService& renderService, const Color& color) const {
            renderService.setForegroundColor(color);
            renderService.renderFaceHandles(VectorUtils::cast<Polygon3f>(handles));
        }
        
        void FaceTool::renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Polygon3& handle, const Color& color) const {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(color);
            renderService.renderFaceHandle(handle);
        }
        
        void FaceTool::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Polygon3& handle) const {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.renderFaceHandleHighlight(handle);
        }
        
        void FaceTool::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Polygon3& position) const {
            
        }
    }
}
