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
#include "Model/ModelTypes.h"
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
        class Grid;
        class Selection;

        class ClipTool : public Tool {
        public:
            static const Model::Hit::HitType ClipPointHit;

            class ClipPointSnapper {
            public:
                virtual ~ClipPointSnapper();
                bool snapClipPoint(const Grid& grid, const Vec3& point, Vec3& snapped) const;
            private:
                virtual bool doSnapClipPoint(const Grid& grid, const Vec3& point, Vec3& snapped) const = 0;
            };
            
            class ClipPointStrategy {
            public:
                virtual ~ClipPointStrategy();
                bool computeThirdClipPoint(const Vec3& point1, const Vec3& point2, Vec3& point3) const;
            private:
                virtual bool doComputeThirdClipPoint(const Vec3& point1, const Vec3& point2, Vec3& point3) const = 0;
            };
            
            class ClipPointStrategyFactory {
            public:
                virtual ~ClipPointStrategyFactory();
                const ClipPointStrategy* createStrategy() const;
            private:
                virtual ClipPointStrategy* doCreateStrategy() const = 0;
            };
            
            class NullClipPointStrategyFactory : public ClipPointStrategyFactory {
            private:
                ClipPointStrategy* doCreateStrategy() const;
            };
        private:
            enum ClipSide {
                ClipSide_Front,
                ClipSide_Both,
                ClipSide_Back
            };
        private:
            MapDocumentWPtr m_document;
            Vec3 m_clipPoints[3];
            size_t m_numClipPoints;
            size_t m_dragIndex;
            ClipSide m_clipSide;
            
            const ClipPointStrategy* m_clipPointStrategy;
            
            Model::ParentChildrenMap m_frontBrushes;
            Model::ParentChildrenMap m_backBrushes;

            Renderer::BrushRenderer* m_remainingBrushRenderer;
            Renderer::BrushRenderer* m_clippedBrushRenderer;
            
            bool m_ignoreNotifications;
        public:
            ClipTool(MapDocumentWPtr document);
            ~ClipTool();
            
            void toggleClipSide();
            void performClip();

            void pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            
            Vec3 defaultClipPointPos() const;
            
            bool addClipPoint(const Vec3& point, const ClipPointSnapper& snapper, const ClipPointStrategyFactory& factory = NullClipPointStrategyFactory());
            bool setClipPlane(const Plane3& plane);
        public:
            bool beginDragClipPoint(const Model::PickResult& pickResult);
            Vec3 draggedPointPosition() const;
            bool dragClipPoint(const Vec3& newPosition, const ClipPointSnapper& snapper);

            bool hasClipPoints() const;
            void deleteLastClipPoint();
            
            void reset();
        private:
            void resetClipPoints();
            void resetClipSide();
            void resetClipPointStrategy();
            
            bool canClip() const;
            Vec3 clipPoint(size_t index) const;
            bool virtualClipPoint(Vec3& point) const;
        public:
            void renderBrushes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderClipPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight(bool dragging, const Model::PickResult& pickResult, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            void renderHighlight(size_t index, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            bool doActivate();
            bool doDeactivate();
            
            void bindObservers();
            void unbindObservers();
            void selectionDidChange(const Selection& selection);
            void nodesWillChange(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
            
            void update();
            
            void updateBrushes();
            void clipBrushes();
            void deleteBrushes();
            void setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace* frontFace, Model::BrushFace* backFace) const;
            
            void clearRenderers();
            void updateRenderers();
            bool keepFrontBrushes() const;
            bool keepBackBrushes() const;

            class AddBrushesToRendererVisitor;
            void addBrushesToRenderer(const Model::ParentChildrenMap& map, Renderer::BrushRenderer* renderer);
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */
