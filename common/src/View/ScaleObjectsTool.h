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
#include "BBox.h"

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
            
            explicit BBoxSide(const Vec3& n) : normal(n) {}
        };
        
        /**
         * BBox corner, normalized to a +/- 1 unit box
         */
        class BBoxCorner {
        public:
            Vec3 corner;
            
            explicit BBoxCorner(const Vec3& c) : corner(c) {}
        };
        
        /**
         * BBox edge, normalized to a +/- 1 unit box
         */
        class BBoxEdge {
        public:
            Vec3 point0;
            Vec3 point1;
            
            explicit BBoxEdge(const Vec3 &p0, const Vec3& p1) : point0(p0), point1(p1) {}
        };
        
        // handle class hierarchy
        
        class BBoxHandle {
        };
        class CornerHandle : public BBoxHandle {
        public:
            BBoxCorner corner;
            
            explicit CornerHandle(const BBoxCorner& c) : corner(c) {}
        };
        class EdgeHandle : public BBoxHandle {
        public:
            BBoxEdge edge;
            
            explicit EdgeHandle(const BBoxEdge& e) : edge(e) {}
        };
        class FaceHandle : public BBoxHandle {
        public:
            BBoxSide side;
            
            explicit FaceHandle(const BBoxSide& s) : side(s) {}
        };
        
        
        
        class ScaleObjectsTool : public Tool {
        private:
            static const Model::Hit::HitType ScaleToolFaceHit;
            static const Model::Hit::HitType ScaleToolEdgeHit;
            static const Model::Hit::HitType ScaleToolCornerHit;
            
            MapDocumentWPtr m_document;
            ScaleObjectsToolPage* m_toolPage;

            /**
             * current "handle" location. Updated every time the currently
             * dragged face/edge/corner moves one grid step.
             */
            Vec3 m_dragOrigin;
            /**
             * Total amount of drag accumulated since beginResize()
             *
             * Only used to decide whether to commit the change at the end of the drag
             * in ScaleObjectsTool::commitResize
             */
            Vec3 m_totalDelta;

            Model::Hit m_dragStartHit; // contains the drag type (face/edge/corner)
            bool m_resizing; // unused
            
            /**
             * bounds in beginResize()
             */
            BBox3 m_bboxAtDragStart;
            bool m_isShearing;
            
        public: // debug only
            
            Vec3 dragOrigin() const { return m_dragOrigin; }
            
        public:
            ScaleObjectsTool(MapDocumentWPtr document);
            ~ScaleObjectsTool();
            
            bool applies() const;
            
            Model::Hit pick2D(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult);
            Model::Hit pick3D(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult);
        public:
            BBox3 bounds() const;
        private:
            class PickProximateFace;
            Model::Hit pickProximateFace(Model::Hit::HitType hitType, const Ray3& pickRay) const;
        public:
        
            std::vector<Polygon3> bboxFaces() const;
            
            // getting highlighted hanles
            bool hasDragPolygon() const;
            Polygon3 dragPolygon() const;
            
            bool hasDragEdge() const;
            Edge3 dragEdge() const;
            
            bool hasDragCorner() const;
            Vec3 dragCorner() const;
            
            // for rendering sheared bbox
            BBox3 bboxAtDragStart() const;
            Mat4x4 bboxShearMatrix() const;
            bool isShearing() const;
            
            // regular handles
            Vec3::List cornerHandles() const;
            
//            Vec3 dragPolygonNormal() const;
            
            void updateDragFaces(const Model::PickResult& pickResult);
            
        private:
//            Polygon3 getDragPolygon(const Model::Hit& hit) const;
            class MatchFaceBoundary;
            Model::BrushFaceList collectDragFaces(const Model::Hit& hit) const;
            Model::BrushFaceList collectDragFaces(Model::BrushFace* face) const;
        public:
            bool beginResize(const Model::PickResult& pickResult, bool split);
            bool resize(const Ray3& pickRay, const Renderer::Camera& camera, const bool proportional, const bool vertical, const bool shear);
            
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
