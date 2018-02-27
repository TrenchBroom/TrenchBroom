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

#ifndef TrenchBroom_ScaleObjectsTool
#define TrenchBroom_ScaleObjectsTool

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/RotateObjectsHandle.h"
#include "View/ScaleObjectsToolPage.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class Selection;
        
        class BBoxSide {
        public:
            Vec3 normal;
            
            BBoxSide(const Vec3& n) : normal(n) {}
        };
        
        /**
         * BBox corner, normalized to a +/- 1 unit box
         */
        class BBoxCorner {
        public:
            Vec3 corner;
            
            BBoxCorner(const Vec3& c) : corner(c) {}
        };
        
        /**
         * BBox edge, normalized to a +/- 1 unit box
         */
        class BBoxEdge {
        public:
            Vec3 point0;
            Vec3 point1;
            
            BBoxEdge(const Vec3 &p0, const Vec3& p1) : point0(p0), point1(p1) {}
        };        
    
        class BBoxHandle {
        };
        class CornerHandle : public BBoxHandle {
            BBoxCorner m_corner;
        };
        class EdgeHandle : public BBoxHandle {
            BBoxEdge m_edge;
        };
        class FaceHandle : public BBoxHandle {
            BBoxSide m_side;
        };
        
        class ScaleObjectsTool : public Tool {
        private:
            static const Model::Hit::HitType ScaleHit3D;
            static const Model::Hit::HitType ScaleHit2D;
            
            MapDocumentWPtr m_document;
            ScaleObjectsToolPage* m_toolPage;
            
            Vec3 m_dragAxis;
            
            Vec3 m_dragOrigin;
            Vec3 m_totalDelta;

            bool m_resizing;
            BBoxSide m_dragSide;
            BBox3 m_bboxAtDragStart;
        public:
            ScaleObjectsTool(MapDocumentWPtr document);
            ~ScaleObjectsTool();
            
            bool applies() const;
            
            Model::Hit pick2D(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult);
            Model::Hit pick3D(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult);
        private:
            BBox3 bounds() const;
        private:
            class PickProximateFace;
            Model::Hit pickProximateFace(Model::Hit::HitType hitType, const Ray3& pickRay) const;
        public:
        
            std::vector<Polygon3> bboxFaces() const;
            
            bool hasDragPolygon() const;
            Polygon3 dragPolygon() const;
//            Vec3 dragPolygonNormal() const;
            
            void updateDragFaces(const Model::PickResult& pickResult);
            
        private:
//            Polygon3 getDragPolygon(const Model::Hit& hit) const;
            class MatchFaceBoundary;
            Model::BrushFaceList collectDragFaces(const Model::Hit& hit) const;
            Model::BrushFaceList collectDragFaces(Model::BrushFace* face) const;
        public:
            bool beginResize(const Model::PickResult& pickResult, bool split);
            bool resize(const Ray3& pickRay, const Renderer::Camera& camera);
            
            void commitResize();
            void cancelResize();
        private:
            void bindObservers();
            void unbindObservers();
            void nodesDidChange(const Model::NodeList& nodes);
            void selectionDidChange(const Selection& selection);
            
        private:
            wxWindow* doCreatePage(wxWindow* parent) override;
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsTool) */
