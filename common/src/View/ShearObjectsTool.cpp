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

#include "ShearObjectsTool.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "ScaleObjectsTool.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/FindMatchingBrushFaceVisitor.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <algorithm>
#include <iterator>
#include <array>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ShearObjectsTool::ShearToolSideHit = Model::Hit::freeHitType();

        ShearObjectsTool::ShearObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_resizing(false),
        m_constrainVertical(false),
        m_dragStartHit(Model::Hit::NoHit) {}
        
        ShearObjectsTool::~ShearObjectsTool() = default;
        
        bool ShearObjectsTool::applies() const {
            MapDocumentSPtr document = lock(m_document);
            return !document->selectedNodes().empty();
        }

        void ShearObjectsTool::pickBackSides(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            // select back sides. Used for both 2D and 3D.
            if (pickResult.empty()) {
                const auto result = pickBackSideOfBox(pickRay, camera, bounds());

                // The hit point is the closest point on the pick ray to one of the edges of the face.
                // For face dragging, we'll project the pick ray onto the line through this point and having the face normal.
                assert(result.pickedSideNormal != vec3::zero);
                pickResult.addHit(Model::Hit(ShearToolSideHit, result.distAlongRay, pickRay.pointAtDistance(result.distAlongRay), BBoxSide{result.pickedSideNormal}));
            }
        }

        void ShearObjectsTool::pick2D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin)) {
                return;
            }

            Model::PickResult localPickResult;

            pickBackSides(pickRay, camera, localPickResult);

            auto hit = localPickResult.query().first();

            if (hit.isMatch()) {
                pickResult.addHit(hit);
            }
        }

        void ShearObjectsTool::pick3D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin)) {
                return;
            }

            Model::PickResult localPickResult;

            // these handles only work in 3D.
            assert(camera.perspectiveProjection());

            // sides
            for (const BBoxSide& side : allSides()) {
                const auto poly = polygonForBBoxSide(myBounds, side);

                const FloatType dist = intersectPolygonWithRay(pickRay, poly.begin(), poly.end());
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ShearToolSideHit, dist, pickRay.pointAtDistance(dist), side));
                }
            }

            pickBackSides(pickRay, camera, localPickResult);

            auto hit = localPickResult.query().first();

            if (hit.isMatch()) {
                pickResult.addHit(hit);
            }
        }


        BBox3 ShearObjectsTool::bounds() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds();
        }
        
        // used for rendering
        
        bool ShearObjectsTool::hasDragPolygon() const {
            return dragPolygon().vertexCount() > 0;
        }

        Polygon3f ShearObjectsTool::dragPolygon() const {
            if (m_dragStartHit.type() == ShearToolSideHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                return Polygon3f(polygonForBBoxSide(bounds(), side));
            }
                                                            
            return Polygon3f();
        }

        // for rendering sheared bbox
        BBox3 ShearObjectsTool::bboxAtDragStart() const {
            if (m_resizing) {
                return m_bboxAtDragStart;
            } else {
                return bounds();
            }
        }

        void ShearObjectsTool::startShearWithHit(const Model::Hit& hit) {
            ensure(hit.isMatch(), "must start with matching hit");
            ensure(hit.type() == ShearToolSideHit, "wrong hit type");
            ensure(!m_resizing, "must not be resizing already");

            m_bboxAtDragStart = bounds();
            m_dragStartHit = hit;
            m_dragCumulativeDelta = vec3::zero;

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Shear Objects");
            m_resizing = true;
        }

        void ShearObjectsTool::commitShear() {
            ensure(m_resizing, "must be resizing already");

            MapDocumentSPtr document = lock(m_document);
            if (isZero(m_dragCumulativeDelta)) {
                document->cancelTransaction();
            } else {
                document->commitTransaction();
            }
            m_resizing = false;
        }

        void ShearObjectsTool::cancelShear() {
            ensure(m_resizing, "must be resizing already");

            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();

            m_resizing = false;
        }

        void ShearObjectsTool::shearByDelta(const vec3 &delta) {
            ensure(m_resizing, "must be resizing already");

            m_dragCumulativeDelta += delta;

            MapDocumentSPtr document = lock(m_document);

            if (!isZero(delta)) {
                const BBoxSide side = m_dragStartHit.target<BBoxSide>();

                if (document->shearObjects(bounds(), side.normal, delta)) {
                    // FIXME: What are we supposed to do if this returns false?
                }
            }
        }

        const Model::Hit& ShearObjectsTool::dragStartHit() const {
            return m_dragStartHit;
        }

        Mat4x4 ShearObjectsTool::bboxShearMatrix() const {
            if (!m_resizing) {
                return Mat4x4::identity;
            }

            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ShearToolSideHit) {
                return Mat4x4::identity;
            }
            
            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            
            return shearBBoxMatrix(m_bboxAtDragStart,
                                   side.normal,
                                   m_dragCumulativeDelta);
        }

        Polygon3f ShearObjectsTool::shearHandle() const {
            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ShearToolSideHit) {
                return Polygon3f();
            }
            
            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            // use the bboxAtDragStart() function so we get bounds() if we're not currently inside a drag.
            const Polygon3 polyAtDragStart = polygonForBBoxSide(bboxAtDragStart(), side);
            
            const Polygon3 handle = polyAtDragStart.transformed(bboxShearMatrix());
            return Polygon3f(handle);
        }

        void ShearObjectsTool::updatePickedSide(const Model::PickResult &pickResult) {
            const Model::Hit& hit = pickResult.query().type(ShearToolSideHit).occluded().first();

            // extract the highlighted handle from the hit here, and only refresh views if it changed
            if (hit.type() == ShearToolSideHit && m_dragStartHit.type() == ShearToolSideHit) {
                if (hit.target<BBoxSide>() == m_dragStartHit.target<BBoxSide>()) {
                    return;
                }
            }

            // hack for highlighting on mouseover
            m_dragStartHit = hit;

            refreshViews();
        }

        bool ShearObjectsTool::constrainVertical() const {
            return m_constrainVertical;
        }

        void ShearObjectsTool::setConstrainVertical(const bool constrainVertical) {
            m_constrainVertical = constrainVertical;
        }
    }
}
