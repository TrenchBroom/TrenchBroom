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

#include "RotateObjectsTool.h"

#include "Model/Hit.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/RotateObjectsHandle.h"
#include "View/RotateObjectsToolPage.h"

#include <kdl/memory_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/scalar.h>

namespace TrenchBroom {
    namespace View {
        RotateObjectsTool::RotateObjectsTool(std::weak_ptr<MapDocument> document) :
        Tool(false),
        m_document(document),
        m_toolPage(nullptr),
        m_handle(),
        m_angle(vm::to_radians(15.0)) {}

        bool RotateObjectsTool::doActivate() {
            resetRotationCenter();
            return true;
        }

        const Grid& RotateObjectsTool::grid() const {
            return kdl::mem_lock(m_document)->grid();
        }

        void RotateObjectsTool::updateToolPageAxis(const RotateObjectsHandle::HitArea area) {
            if (area == RotateObjectsHandle::HitArea::XAxis) {
                m_toolPage->setAxis(vm::axis::x);
            } else if (area == RotateObjectsHandle::HitArea::YAxis) {
                m_toolPage->setAxis(vm::axis::y);
            } else if (area == RotateObjectsHandle::HitArea::ZAxis) {
                m_toolPage->setAxis(vm::axis::z);
            }
        }

        double RotateObjectsTool::angle() const {
            return m_angle;
        }

        void RotateObjectsTool::setAngle(const double angle) {
            m_angle = angle;
        }

        vm::vec3 RotateObjectsTool::rotationCenter() const {
            return m_handle.position();
        }

        void RotateObjectsTool::setRotationCenter(const vm::vec3& position) {
            m_handle.setPosition(position);
            m_toolPage->setCurrentCenter(position);
            refreshViews();
        }

        void RotateObjectsTool::resetRotationCenter() {
            auto document = kdl::mem_lock(m_document);
            const auto& bounds = document->selectionBounds();
            const auto position = document->grid().snap(bounds.center());
            setRotationCenter(position);
        }

        FloatType RotateObjectsTool::majorHandleRadius(const Renderer::Camera& camera) const {
            return m_handle.majorHandleRadius(camera);
        }

        FloatType RotateObjectsTool::minorHandleRadius(const Renderer::Camera& camera) const {
            return m_handle.minorHandleRadius(camera);
        }

        void RotateObjectsTool::beginRotation() {
            auto document = kdl::mem_lock(m_document);
            document->startTransaction("Rotate Objects");
        }

        void RotateObjectsTool::commitRotation() {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();
            updateRecentlyUsedCenters(rotationCenter());
        }

        void RotateObjectsTool::cancelRotation() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
        }

        FloatType RotateObjectsTool::snapRotationAngle(const FloatType angle) const {
            auto document = kdl::mem_lock(m_document);
            return document->grid().snapAngle(angle);
        }

        void RotateObjectsTool::applyRotation(const vm::vec3& center, const vm::vec3& axis, const FloatType angle) {
            auto document = kdl::mem_lock(m_document);
            document->rollbackTransaction();
            document->rotateObjects(center, axis, angle);
        }

        Model::Hit RotateObjectsTool::pick2D(const vm::ray3& pickRay, const Renderer::Camera& camera) {
            return m_handle.pick2D(pickRay, camera);
        }

        Model::Hit RotateObjectsTool::pick3D(const vm::ray3& pickRay, const Renderer::Camera& camera) {
            return m_handle.pick3D(pickRay, camera);
        }

        vm::vec3 RotateObjectsTool::rotationAxis(const RotateObjectsHandle::HitArea area) const {
            return m_handle.rotationAxis(area);
        }

        void RotateObjectsTool::renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_handle.renderHandle2D(renderContext, renderBatch);
        }

        void RotateObjectsTool::renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_handle.renderHandle3D(renderContext, renderBatch);
        }

        void RotateObjectsTool::renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea area) {
            m_handle.renderHighlight2D(renderContext, renderBatch, area);
        }
        void RotateObjectsTool::renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea area) {
            m_handle.renderHighlight3D(renderContext, renderBatch, area);
        }

        void RotateObjectsTool::updateRecentlyUsedCenters(const vm::vec3& center) {
            m_recentlyUsedCenters = kdl::vec_erase(std::move(m_recentlyUsedCenters), center);
            m_recentlyUsedCenters.push_back(center);
            m_toolPage->setRecentlyUsedCenters(m_recentlyUsedCenters);
        }

        QWidget* RotateObjectsTool::doCreatePage(QWidget* parent) {
            assert(m_toolPage == nullptr);

            m_toolPage = new RotateObjectsToolPage(m_document, *this, parent);
            return m_toolPage;
        }
    }
}
