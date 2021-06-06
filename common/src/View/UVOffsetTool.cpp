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

#include "UVOffsetTool.h"

#include "Ensure.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Polyhedron.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/UVView.h"

#include <kdl/memory_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/intersection.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        UVOffsetTool::UVOffsetTool(std::weak_ptr<MapDocument> document, const UVViewHelper& helper) :
        ToolControllerBase{},
        Tool{true},
        m_document{std::move(document)},
        m_helper{helper} {}

        Tool* UVOffsetTool::doGetTool() {
            return this;
        }

        const Tool* UVOffsetTool::doGetTool() const {
            return this;
        }

        static vm::vec2f computeHitPoint(const UVViewHelper& helper, const vm::ray3& ray) {
            const auto& boundary = helper.face()->boundary();
            const auto distance = vm::intersect_ray_plane(ray, boundary);
            const auto hitPoint = vm::point_at_distance(ray, distance);

            const auto transform = helper.face()->toTexCoordSystemMatrix(vm::vec2f::zero(), helper.face()->attributes().scale(), true);
            return vm::vec2f{transform * hitPoint};
        }

        static vm::vec2f snapDelta(const UVViewHelper& helper, const vm::vec2f& delta) {
            assert(helper.valid());

            const auto* texture = helper.texture();
            if (texture == nullptr) {
                return vm::round(delta);
            }

            const auto transform = helper.face()->toTexCoordSystemMatrix(helper.face()->attributes().offset() - delta, helper.face()->attributes().scale(), true);

            auto distance = vm::vec2f::max();
            for (const Model::BrushVertex* vertex : helper.face()->vertices()) {
                const auto temp = helper.computeDistanceFromTextureGrid(transform * vertex->position());
                distance = vm::abs_min(distance, temp);
            }

            return helper.snapDelta(delta, -distance);
        }

        bool UVOffsetTool::doStartMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                !inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return false;
            }

            m_lastPoint = computeHitPoint(m_helper, inputState.pickRay());

            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Move Texture");
            return true;
        }

        bool UVOffsetTool::doMouseDrag(const InputState& inputState) {
            assert(m_helper.valid());

            const auto curPoint = computeHitPoint(m_helper, inputState.pickRay());
            const auto delta    = curPoint - m_lastPoint;
            const auto snapped  = snapDelta(m_helper, delta);

            const auto corrected = vm::correct(m_helper.face()->attributes().offset() - snapped, 4, 0.0f);

            if (corrected == m_helper.face()->attributes().offset()) {
                return true;
            }

            auto request = Model::ChangeBrushFaceAttributesRequest{};
            request.setOffset(corrected);

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);

            m_lastPoint = m_lastPoint + snapped;
            return true;
        }

        void UVOffsetTool::doEndMouseDrag(const InputState&) {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();
        }

        void UVOffsetTool::doCancelMouseDrag() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
        }

        bool UVOffsetTool::doCancel() {
            return false;
        }
    }
}
