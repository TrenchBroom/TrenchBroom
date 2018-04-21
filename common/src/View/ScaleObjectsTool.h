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

            bool operator<(const BBoxSide& other) const {
                return normal < other.normal;
            }
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


        enum class AnchorPos {
            Opposite,
            Center
        };

        BBox3 moveBBoxFace(const BBox3& in,
                           BBoxSide side,
                           Vec3 delta,
                           bool proportional,
                           AnchorPos anchor);

        BBox3 moveBBoxCorner(const BBox3& in,
                             BBoxCorner corner,
                             Vec3 delta,
                             AnchorPos anchor);

        BBox3 moveBBoxEdge(const BBox3& in,
                           BBoxEdge edge,
                           Vec3 delta,
                           bool proportional,
                           AnchorPos anchor);

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
             * Point on the initial pick ray that's closest to the handle being dragged.
             * Note, when dragging "back faces" the mouse can start far from the bbox.
             * In this case the m_dragOrigin can be far from the bbox being resized, and close to the camera instead.
             *
             */
            Vec3 m_dragOrigin;
            /**
             * Total amount of drag accumulated since beginResize()
             *
             * Only used to decide whether to commit the change at the end of the drag
             * in ScaleObjectsTool::commitResize
             */
            Vec3 m_totalDelta;

            /**
             * debug temporary
             */
            Vec3 m_handlePos;

            Model::Hit m_dragStartHit; // contains the drag type (face/edge/corner)
            bool m_resizing;

            /**
             * bounds in beginResize()
             * Only valid during a drag (when m_resizing is true).
             */
            BBox3 m_bboxAtDragStart;
            bool m_isShearing;

            /**
             * Modifier key state
             */
            AnchorPos m_anchorPos;
            /**
             * Modifier key state
             */
            bool m_scaleAllAxes;
        public: // debug only
            
            Vec3 dragOrigin() const { return m_dragOrigin; }
            
        public:
            ScaleObjectsTool(MapDocumentWPtr document);
            ~ScaleObjectsTool() override;
            
            bool applies() const;
            
            Model::Hit pick(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult);
        public:
            BBox3 bounds() const;
        private:
            class PickProximateFace;
            Model::Hit pickProximateFace(Model::Hit::HitType hitType, const Ray3& pickRay) const;
        public:
        
            std::vector<Polygon3> bboxFaces() const;
            
            // getting highlighted hanles
            std::vector<Polygon3f> polygonsHighlightedByProportionalDrag() const;
            std::vector<Polygon3f> polygonsHighlightedByDrag() const;

            Vec3 handlePos() const {
                return m_handlePos;
            }
            
            
            bool hasDragPolygon() const;
            Polygon3f dragPolygon() const;
            
            bool hasDragEdge() const;
            Edge3f dragEdge() const;
            
            bool hasDragCorner() const;
            Vec3f dragCorner() const;

            bool hasDragAnchor() const;
            Vec3f dragAnchor() const;

            /**
             * If inside a drag, returns the bbox at the start of the drag.
             * Otherwise, returns the current bounds(). for rendering sheared bbox.
             */
            BBox3 bboxAtDragStart() const;


            Mat4x4 bboxShearMatrix() const;
            Polygon3f shearHandle() const;
            /**
             * This can only be called when a drag is not in progress
             * @param shearing
             */
            void setShearing(bool shearing);
            bool isShearing() const;
            
            // regular handles
            Vec3::List cornerHandles() const;
            
//            Vec3 dragPolygonNormal() const;
            
            void updateDragFaces(const Model::PickResult& pickResult);

            // persist the state of modifier keys
            void setAnchorPos(AnchorPos pos);
            AnchorPos anchorPos() const;

            void setScaleAllAxes(bool allAxes);
            bool scaleAllAxes() const;

        private:
//            Polygon3 getDragPolygon(const Model::Hit& hit) const;
            class MatchFaceBoundary;
            Model::BrushFaceList collectDragFaces(const Model::Hit& hit) const;
            Model::BrushFaceList collectDragFaces(Model::BrushFace* face) const;
        public:
            bool beginResize(const Model::PickResult& pickResult);
            bool resize(const Ray3& pickRay, const Renderer::Camera& camera, const bool vertical);
            
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
