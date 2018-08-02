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

            static bool validSideNormal(const Vec3& n);
            explicit BBoxSide(const Vec3& n);
            bool operator<(const BBoxSide& other) const;
        };
        
        /**
         * BBox corner, normalized to a +/- 1 unit box
         */
        class BBoxCorner {
        public:
            Vec3 corner;

            static bool validCorner(const Vec3& c);
            explicit BBoxCorner(const Vec3& c);
        };
        
        /**
         * BBox edge, normalized to a +/- 1 unit box
         */
        class BBoxEdge {
        public:
            Vec3 point0;
            Vec3 point1;
            
            explicit BBoxEdge(const Vec3 &p0, const Vec3& p1);
        };

        enum class AnchorPos {
            Opposite,
            Center
        };

        std::vector<BBoxSide> AllSides();
        Vec3 normalForBBoxSide(BBoxSide side);
        std::vector<BBoxEdge> AllEdges();
        std::vector<BBoxCorner> AllCorners();
        Vec3 pointForBBoxCorner(const BBox3& box, BBoxCorner corner);
        BBoxSide oppositeSide(BBoxSide side);
        BBoxCorner oppositeCorner(BBoxCorner corner);
        BBoxEdge oppositeEdge(BBoxEdge edge);
        Edge3 pointsForBBoxEdge(const BBox3& box, BBoxEdge edge);
        Polygon3 polygonForBBoxSide(const BBox3& box, BBoxSide side);
        Vec3 centerForBBoxSide(const BBox3& box, BBoxSide side);

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

        /**
         * Only looks at the hit type (corner/edge/face), and which particular corner/edge/face.
         *
         * Returns the line through the bbox that an invisible handle should be dragged, assuming proportional
         * dragging on all 3 axes.
         */
        Line3 handleLineForHit(const BBox3& bboxAtDragStart, const Model::Hit& hit);
        BBox3 moveBBoxForHit(const BBox3& bboxAtDragStart,
                             const Model::Hit& dragStartHit,
                             Vec3 delta,
                             bool proportional,
                             AnchorPos anchor);

        class ScaleObjectsTool : public Tool {
        public:
            static const Model::Hit::HitType ScaleToolFaceHit;
            static const Model::Hit::HitType ScaleToolEdgeHit;
            static const Model::Hit::HitType ScaleToolCornerHit;

        private:
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

            bool m_resizing;

            AnchorPos m_anchorPos;

            // moved from controller:

            BBox3 m_bboxAtDragStart;
            Model::Hit m_dragStartHit; // contains the drag type (face/edge/corner)

            Vec3 m_dragCumulativeDelta;

            bool m_centerAnchor;
            bool m_scaleAllAxes;


        public: // debug only
            
            Vec3 dragOrigin() const { return m_dragOrigin; }
            
        public:
            explicit ScaleObjectsTool(MapDocumentWPtr document);
            ~ScaleObjectsTool() override;

            const Model::Hit& dragStartHit() const;
            bool applies() const;

            void pickBackSides(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            void pick2D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            void pick3D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
        public:
            BBox3 bounds() const;
        public:



            // getting highlighted handles
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

            // regular handles
            Vec3::List cornerHandles() const;

            void updateDragFaces(const Model::PickResult& pickResult);

            // persist the state of modifier keys
            void setAnchorPos(AnchorPos pos);
            AnchorPos anchorPos() const;

            void setScaleAllAxes(bool allAxes);
            bool scaleAllAxes() const;

        public:
            void startScaleWithHit(const Model::Hit& hit);
            void dragScale(const Vec3& delta, bool scaleAllAxes);
            void commitScale();
            void cancelScale();
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
