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

#ifndef FaceTool_h
#define FaceTool_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/VertexHandleManager.h"
#include "View/VertexToolBase.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class FaceTool : public VertexToolBase<Polygon3> {
        private:
            FaceHandleManager m_faceHandles;
        public:
            FaceTool(MapDocumentWPtr document);
        public:
            Model::BrushSet findIncidentBrushes(const Polygon3& handle) const;
        private:
            using VertexToolBase::findIncidentBrushes;
        public:
            void pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override;
        private:
            FaceHandleManager& handleManager() override;
            const FaceHandleManager& handleManager() const override;
        public:
            MoveResult move(const Vec3& delta) override;
            
            String actionName() const override;
        public: // Rendering
            using VertexToolBase::renderHandle;
            using VertexToolBase::renderHandles;
            
            void renderHandles(const Polygon3::List& handles, Renderer::RenderService& renderService, const Color& color) const override;
            void renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Polygon3& handle, const Color& color) const override;
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Polygon3& handle) const override;
            void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Polygon3& position) const override;
        };
    }
}

#endif /* FaceTool_h */
