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
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class Grid;
        class Selection;

        class ClipTool : public Tool {
        public:
            class ClipPlaneStrategy {
            public:
                virtual ~ClipPlaneStrategy();
                
                Vec3 snapClipPoint(const Grid& grid, const Vec3& point) const;
                bool computeClipPlane(const Vec3& point1, const Vec3& point2, Plane3& clipPlane) const;
                bool computeClipPlane(const Vec3& point1, const Vec3& point2, const Vec3& point3, const Plane3& clipPlane2, Plane3& clipPlane3) const;
            private:
                virtual Vec3 doSnapClipPoint(const Grid& grid, const Vec3& point) const = 0;
                virtual bool doComputeClipPlane(const Vec3& point1, const Vec3& point2, Plane3& clipPlane) const = 0;
                virtual bool doComputeClipPlane(const Vec3& point1, const Vec3& point2, const Vec3& point3, Plane3& clipPlane) const = 0;
            };
        private:
            MapDocumentWPtr m_document;
            Vec3 m_clipPoints[3];
            Plane3 m_clipPlanes[2];
            size_t m_numClipPoints;
        public:
            ClipTool(MapDocumentWPtr document);
            
            void toggleClipSide();
            void performClip();

            void pick(const Ray3& pickRay, Model::PickResult& pickResult);
            
            Vec3 defaultClipPointPos() const;
            
            bool addClipPoint(const Vec3& point, const ClipPlaneStrategy& strategy);
            bool updateClipPoint(size_t index, const Vec3& newPosition, const ClipPlaneStrategy& strategy);

            bool hasClipPoints() const;
            void deleteLastClipPoint();
            
            bool reset();
            
            void renderBrushes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderClipPoints(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight(bool dragging, const Model::PickResult& pickResult, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            bool doActivate();
            bool doDeactivate();
            
            void bindObservers();
            void unbindObservers();
            void selectionDidChange(const Selection& selection);
            void nodesWillChange(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
            
            void update();
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */
