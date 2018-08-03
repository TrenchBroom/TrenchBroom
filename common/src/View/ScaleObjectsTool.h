/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen
 
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
#include "Model/Hit.h"
#include "View/Tool.h"
#include "View/ScaleObjectsToolPage.h"
#include "BBox.h"

#include <bitset>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        /**
         * Identifies the side of a bbox using a normal. The normal will be one of +/- 1.0 along X, Y, or Z.
         */
        class BBoxSide {
        public:
            Vec3 normal;

            static bool validSideNormal(const Vec3& n);
            explicit BBoxSide(const Vec3& n);
            bool operator<(const BBoxSide& other) const;
            bool operator==(const BBoxSide& other) const;
        };
        
        /**
         * Identifies a bbox corner, using a point on a bbox whose corners are at +/- 1.0
         * (i.e. a 2x2x2 box centered at 0, 0, 0).
         */
        class BBoxCorner {
        public:
            Vec3 corner;

            static bool validCorner(const Vec3& c);
            explicit BBoxCorner(const Vec3& c);

            bool operator==(const BBoxCorner& other) const;
        };
        
        /**
         * Identifies a directed edge of a bbox, using points on a bbox whose corners are at +/- 1.0
         * (i.e. a 2x2x2 box centered at 0, 0, 0).
         */
        class BBoxEdge {
        public:
            Vec3 point0;
            Vec3 point1;
            
            explicit BBoxEdge(const Vec3 &p0, const Vec3& p1);

            bool operator==(const BBoxEdge& other) const;
        };

        enum class AnchorPos {
            Opposite,
            Center
        };

        /**
         * A set containing a subset of (X, Y, Z). Identifies which axes will be scaled.
         * Used for recording which axes the Shift key affects in 2D views.
         */
        class ProportionalAxes {
        private:
            std::bitset<3> m_bits;
        public:
            ProportionalAxes(bool xProportional, bool yProportional, bool zProportional);

            static ProportionalAxes All();
            static ProportionalAxes None();

            void setAxisProportional(size_t axis, bool proportional);
            bool isAxisProportional(size_t axis) const;
            bool allAxesProportional() const;

            bool operator==(const ProportionalAxes& other) const;
            bool operator!=(const ProportionalAxes& other) const;
        };

        std::vector<BBoxSide> allSides();
        std::vector<BBoxEdge> allEdges();
        std::vector<BBoxCorner> allCorners();
        Vec3 pointForBBoxCorner(const BBox3& box, const BBoxCorner& corner);
        BBoxSide oppositeSide(const BBoxSide& side);
        BBoxCorner oppositeCorner(const BBoxCorner& corner);
        BBoxEdge oppositeEdge(const BBoxEdge& edge);
        Edge3 pointsForBBoxEdge(const BBox3& box, const BBoxEdge& edge);
        Polygon3 polygonForBBoxSide(const BBox3& box, const BBoxSide& side);
        Vec3 centerForBBoxSide(const BBox3& box, const BBoxSide& side);

        /**
         * Computes a new bbox after moving the given side by the given delta.
         *
         * Only the component of `delta` matching the axis of `side` is used.
         *
         * `proportional` controls which other axes are scaled.
         *
         * Returns BBox3(Vec3::Null, Vec3::Null) if the move could not be completed
         * because the specified delta either collapses the bbox, or inverts it.
         */
        BBox3 moveBBoxSide(const BBox3 &in,
                           const BBoxSide &side,
                           const Vec3 &delta,
                           ProportionalAxes proportional,
                           AnchorPos anchor);

        /**
         * Computes a new bbox after moving the given corner by the given delta.
         *
         * All components of the `delta` are used.
         *
         * Returns BBox3(Vec3::Null, Vec3::Null) if the move could not be completed
         * because the specified delta either collapses the bbox, or inverts it.
         */
        BBox3 moveBBoxCorner(const BBox3& in,
                             const BBoxCorner& corner,
                             const Vec3& delta,
                             AnchorPos anchor);

        /**
         * Computes a new bbox after moving the specified edge by the specified delta.
         *
         * If `edge` points along an axis i, the ith component of `delta` is ignored.
         * `proportional` only controls whether the bbox grows along axis `i`.
         *
         * Returns BBox3(Vec3::Null, Vec3::Null) if the move could not be completed
         * because the specified delta either collapses the bbox, or inverts it.
         */
        BBox3 moveBBoxEdge(const BBox3& in,
                           const BBoxEdge& edge,
                           const Vec3& delta,
                           ProportionalAxes proportional,
                           AnchorPos anchor);

        /**
         * Returns the line through the bbox that an invisible handle should be dragged, assuming proportional
         * dragging on all 3 axes.
         *
         * Only looks at the hit type (corner/edge/side), and which particular corner/edge/side.
         */
        Line3 handleLineForHit(const BBox3& bboxAtDragStart, const Model::Hit& hit);

        /**
         * Wrapper around moveBBoxSide/moveBBoxEdge/moveBBoxCorner.
         *
         * Looks in the `dragStartHit` and calls the appropriate move function based on whether a side, edge, or corner
         * handle was grabbed.
         */
        BBox3 moveBBoxForHit(const BBox3& bboxAtDragStart,
                             const Model::Hit& dragStartHit,
                             const Vec3& delta,
                             ProportionalAxes proportional,
                             AnchorPos anchor);

        class ScaleObjectsTool : public Tool {
        public:
            static const Model::Hit::HitType ScaleToolSideHit;
            static const Model::Hit::HitType ScaleToolEdgeHit;
            static const Model::Hit::HitType ScaleToolCornerHit;

        private:
            MapDocumentWPtr m_document;
            ScaleObjectsToolPage* m_toolPage;
            bool m_resizing;
            AnchorPos m_anchorPos;
            BBox3 m_bboxAtDragStart;
            Model::Hit m_dragStartHit; // contains the drag type (side/edge/corner)
            Vec3 m_dragCumulativeDelta;
            ProportionalAxes m_proportionalAxes;

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
            std::vector<Polygon3f> polygonsHighlightedByDrag() const;

            bool hasDragPolygon() const;
            Polygon3f dragPolygon() const;
            
            bool hasDragEdge() const;
            Edge3f dragEdge() const;
            
            bool hasDragCorner() const;
            Vec3f dragCorner() const;

            bool hasDragAnchor() const;
            Vec3f dragAnchor() const;

            /**
             * Returns the bbox at the start of the drag. Only allowed to call while m_resizing is true.
             */
            BBox3 bboxAtDragStart() const;

            Vec3::List cornerHandles() const;

            void updatePickedHandle(const Model::PickResult &pickResult);

            void setAnchorPos(AnchorPos pos);
            AnchorPos anchorPos() const;

            void setProportionalAxes(ProportionalAxes proportionalAxes);
            ProportionalAxes proportionalAxes() const;

        public:
            void startScaleWithHit(const Model::Hit& hit);
            void scaleByDelta(const Vec3& delta);
            void commitScale();
            void cancelScale();

        private:
            wxWindow* doCreatePage(wxWindow* parent) override;
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsTool) */
