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

#ifndef __TrenchBroom__ClipTool__
#define __TrenchBroom__ClipTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Hit.h"
#include "View/Clipper.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class BrushRenderer;
        class Camera;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class Selection;
        
        class ClipTool : public Tool {
        private:
            static const Model::Hit::HitType HandleHit;
        private:
            MapDocumentWPtr m_document;
            Clipper m_clipper;
            ClipResult m_clipResult;
            size_t m_dragPointIndex;
            
            Renderer::BrushRenderer* m_brushRenderer;
        public:
            ClipTool(MapDocumentWPtr document);
            ~ClipTool();
            
            bool canToggleClipSide() const;
            void toggleClipSide();
            bool canPerformClip() const;
            void performClip();
            bool canDeleteLastClipPoint() const;
            void deleteLastClipPoint();

            void pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;
            
            bool addClipPoint(const Vec3& clickPoint, const Model::BrushFace* face, const Renderer::Camera& camera);
            void setClipPoints(const Vec3& clickPoint, const Model::BrushFace* face, const Renderer::Camera& camera);
            bool resetClipper();
            
            bool beginDragClipPoint(const Model::PickResult& pickResult);
            void dragClipPoint(const Vec3& hitPoint, const Model::BrushFace* face);

            void renderBrushes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderClipPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderDragHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight(const Model::PickResult& pickResult, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            bool doActivate();
            bool doDeactivate();
            
            void bindObservers();
            void unbindObservers();
            void selectionDidChange(const Selection& selection);
            void nodesWillChange(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
            
            Vec3 computeClipPoint(const Vec3& point, const Plane3& boundary) const;

            void updateBrushes();
            
            class AddBrushesToRendererVisitor;
            void addBrushesToRenderer(const Model::ParentChildrenMap& map);
            
            void clearClipResult();
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */
