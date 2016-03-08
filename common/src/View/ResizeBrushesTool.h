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

#ifndef TrenchBroom_ResizeBrushesTool
#define TrenchBroom_ResizeBrushesTool

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
        class Camera;
    }
    
    namespace View {
        class ResizeBrushesTool : public Tool {
        private:
            static const Model::Hit::HitType ResizeHit3D;
            static const Model::Hit::HitType ResizeHit2D;

            MapDocumentWPtr m_document;
            Model::BrushFaceList m_dragFaces;
            Vec3 m_dragOrigin;
            Vec3 m_totalDelta;
            bool m_splitBrushes;
        public:
            ResizeBrushesTool(MapDocumentWPtr document);
            
            bool applies() const;
            
            Model::Hit pick2D(const Ray3& pickRay, const Model::PickResult& pickResult);
            Model::Hit pick3D(const Ray3& pickRay, const Model::PickResult& pickResult);
        private:
            class PickProximateFace;
            Model::Hit pickProximateFace(Model::Hit::HitType hitType, const Ray3& pickRay) const;
        public:
            bool hasDragFaces() const;
            const Model::BrushFaceList& dragFaces() const;
            void updateDragFaces(const Model::PickResult& pickResult);
        private:
            Model::BrushFaceList getDragFaces(const Model::Hit& hit) const;
            class MatchFaceBoundary;
            Model::BrushFaceList collectDragFaces(const Model::Hit& hit) const;
            Model::BrushFaceList collectDragFaces(Model::BrushFace* face) const;
        public:
            bool beginResize(const Model::PickResult& pickResult, bool split);
            bool resize(const Ray3& pickRay, const Renderer::Camera& camera);
            Vec3 selectDelta(const Vec3& relativeDelta, const Vec3& absoluteDelta, FloatType mouseDistance) const;

            void commitResize();
            void cancelResize();
        private:
            bool splitBrushes(const Vec3& delta);
            Model::BrushFace* findMatchingFace(Model::Brush* brush, const Model::BrushFace* reference) const;
        };
    }
}

#endif /* defined(TrenchBroom_ResizeBrushesTool) */
