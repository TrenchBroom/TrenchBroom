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

#ifndef __TrenchBroom__ResizeBrushesTool__
#define __TrenchBroom__ResizeBrushesTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Hit.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Hit;
    class Hits;

    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class ResizeBrushesTool : public Tool {
        private:
            static const Hit::HitType ResizeHit3D;
            static const Hit::HitType ResizeHit2D;

            MapDocumentWPtr m_document;
            Model::BrushFaceList m_dragFaces;
            Vec3 m_dragOrigin;
            Vec3 m_totalDelta;
            bool m_splitBrushes;
        public:
            ResizeBrushesTool(MapDocumentWPtr document);
            
            bool applies() const;
            
            Hit pick2D(const Ray3& pickRay, const Hits& hits);
            Hit pick3D(const Ray3& pickRay, const Hits& hits);
        private:
            class PickProximateFace;
            Hit pickProximateFace(Hit::HitType hitType, const Ray3& pickRay) const;
        public:
            bool hasDragFaces() const;
            const Model::BrushFaceList& dragFaces() const;
            void updateDragFaces(const Hits& hits);
        private:
            class MatchFaceBoundary;
            Model::BrushFaceList collectDragFaces(const Hit& hit) const;
            Model::BrushFaceList collectDragFaces(Model::BrushFace* face) const;
        public:
            bool beginResize(const Hits& hits, bool split);
            bool resize(const Ray3& pickRay, const Renderer::Camera& camera);
            void commitResize();
            void cancelResize();
        private:
            bool splitBrushes(const Vec3& delta);
            Model::BrushFace* findMatchingFace(Model::Brush* brush, const Model::BrushFace* reference) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ResizeBrushesTool__) */
