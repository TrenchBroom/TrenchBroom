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

#include "Ensure.h"
#include "FloatType.h"
#include "Preferences.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/ScaleObjectsTool.h"

#include <kdl/memory_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/bbox.h>
#include <vecmath/polygon.h>
#include <vecmath/intersection.h>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type ShearObjectsTool::ShearToolSideHitType = Model::HitType::freeType();

        ShearObjectsTool::ShearObjectsTool(std::weak_ptr<MapDocument> document) :
        Tool(false),
        m_document(std::move(document)),
        m_resizing(false),
        m_constrainVertical(false),
        m_dragStartHit(Model::Hit::NoHit) {}

        ShearObjectsTool::~ShearObjectsTool() = default;

        bool ShearObjectsTool::applies() const {
            auto document = kdl::mem_lock(m_document);
            return !document->selectedNodes().empty();
        }

        void ShearObjectsTool::pickBackSides(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            // select back sides. Used for both 2D and 3D.
            if (pickResult.empty()) {
                const auto result = pickBackSideOfBox(pickRay, camera, bounds());

                // The hit point is the closest point on the pick ray to one of the edges of the face.
                // For face dragging, we'll project the pick ray onto the line through this point and having the face normal.
                assert(result.pickedSideNormal != vm::vec3::zero());
                pickResult.addHit(Model::Hit(ShearToolSideHitType, result.distAlongRay, vm::point_at_distance(pickRay, result.distAlongRay), BBoxSide{result.pickedSideNormal}));
            }
        }

        void ShearObjectsTool::pick2D(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            using namespace Model::HitFilters;

            const vm::bbox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin)) {
                return;
            }

            Model::PickResult localPickResult;

            pickBackSides(pickRay, camera, localPickResult);

            if (!localPickResult.empty()) {
                pickResult.addHit(localPickResult.all().front());
            }
        }

        void ShearObjectsTool::pick3D(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            using namespace Model::HitFilters;

            const auto& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin)) {
                return;
            }

            Model::PickResult localPickResult;

            // these handles only work in 3D.
            assert(camera.perspectiveProjection());

            // sides
            for (const auto& side : allSides()) {
                const auto poly = polygonForBBoxSide(myBounds, side);

                const auto dist = vm::intersect_ray_polygon(pickRay, std::begin(poly), std::end(poly));
                if (!vm::is_nan(dist)) {
                    localPickResult.addHit(Model::Hit(ShearToolSideHitType, dist, vm::point_at_distance(pickRay, dist), side));
                }
            }

            pickBackSides(pickRay, camera, localPickResult);

            if (!localPickResult.empty()) {
                pickResult.addHit(localPickResult.all().front());
            }
        }


        vm::bbox3 ShearObjectsTool::bounds() const {
            auto document = kdl::mem_lock(m_document);
            return document->selectionBounds();
        }

        // used for rendering

        bool ShearObjectsTool::hasDragPolygon() const {
            return dragPolygon().vertexCount() > 0;
        }

        vm::polygon3f ShearObjectsTool::dragPolygon() const {
            if (m_dragStartHit.type() == ShearToolSideHitType) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                return vm::polygon3f(polygonForBBoxSide(bounds(), side));
            }

            return vm::polygon3f();
        }

        // for rendering sheared bbox
        vm::bbox3 ShearObjectsTool::bboxAtDragStart() const {
            if (m_resizing) {
                return m_bboxAtDragStart;
            } else {
                return bounds();
            }
        }

        void ShearObjectsTool::startShearWithHit(const Model::Hit& hit) {
            ensure(hit.isMatch(), "must start with matching hit");
            ensure(hit.type() == ShearToolSideHitType, "wrong hit type");
            ensure(!m_resizing, "must not be resizing already");

            m_bboxAtDragStart = bounds();
            m_dragStartHit = hit;
            m_dragCumulativeDelta = vm::vec3::zero();

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Shear Objects");
            m_resizing = true;
        }

        void ShearObjectsTool::commitShear() {
            ensure(m_resizing, "must be resizing already");

            auto document = kdl::mem_lock(m_document);
            if (vm::is_zero(m_dragCumulativeDelta, vm::C::almost_zero())) {
                document->cancelTransaction();
            } else {
                document->commitTransaction();
            }
            m_resizing = false;
        }

        void ShearObjectsTool::cancelShear() {
            ensure(m_resizing, "must be resizing already");

            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();

            m_resizing = false;
        }

        void ShearObjectsTool::shearByDelta(const vm::vec3 &delta) {
            ensure(m_resizing, "must be resizing already");

            m_dragCumulativeDelta = m_dragCumulativeDelta + delta;

            auto document = kdl::mem_lock(m_document);

            if (!vm::is_zero(delta, vm::C::almost_zero())) {
                const BBoxSide side = m_dragStartHit.target<BBoxSide>();
                document->shearObjects(bounds(), side.normal, delta);
            }
        }

        const Model::Hit& ShearObjectsTool::dragStartHit() const {
            return m_dragStartHit;
        }

        vm::mat4x4 ShearObjectsTool::bboxShearMatrix() const {
            if (!m_resizing) {
                return vm::mat4x4::identity();
            }

            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ShearToolSideHitType) {
                return vm::mat4x4::identity();
            }

            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            return vm::shear_bbox_matrix(m_bboxAtDragStart, side.normal, m_dragCumulativeDelta);
        }

        vm::polygon3f ShearObjectsTool::shearHandle() const {
            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ShearToolSideHitType) {
                return vm::polygon3f();
            }

            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            // use the bboxAtDragStart() function so we get bounds() if we're not currently inside a drag.
            const vm::polygon3 polyAtDragStart = polygonForBBoxSide(bboxAtDragStart(), side);

            const vm::polygon3 handle = polyAtDragStart.transform(bboxShearMatrix());
            return vm::polygon3f(handle);
        }

        void ShearObjectsTool::updatePickedSide(const Model::PickResult &pickResult) {
            using namespace Model::HitFilters;
            const Model::Hit& hit = pickResult.first(type(ShearToolSideHitType));

            // extract the highlighted handle from the hit here, and only refresh views if it changed
            if (hit.type() == ShearToolSideHitType && m_dragStartHit.type() == ShearToolSideHitType) {
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
