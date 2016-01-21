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

#include "RotateObjectsTool.h"

#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/RotateObjectsHandle.h"
#include "View/RotateObjectsToolPage.h"

namespace TrenchBroom {
    namespace View {
        RotateObjectsTool::RotateObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_toolPage(NULL),
        m_handle(),
        m_angle(Math::radians(15.0)),
        m_firstActivation(true) {}

        bool RotateObjectsTool::doActivate() {
            if (m_firstActivation) {
                resetRotationCenter();
                m_firstActivation = false;
            }
            return true;
        }

        void RotateObjectsTool::updateToolPageAxis(const RotateObjectsHandle::HitArea area) {
            if (area == RotateObjectsHandle::HitArea_XAxis)
                m_toolPage->setAxis(Math::Axis::AX);
            else if (area == RotateObjectsHandle::HitArea_YAxis)
                m_toolPage->setAxis(Math::Axis::AY);
            else if (area == RotateObjectsHandle::HitArea_ZAxis)
                m_toolPage->setAxis(Math::Axis::AZ);
        }
        
        double RotateObjectsTool::angle() const {
            return m_angle;
        }
        
        void RotateObjectsTool::setAngle(const double angle) {
            m_angle = angle;
        }
        
        Vec3 RotateObjectsTool::rotationCenter() const {
            return m_handle.position();
        }
        
        void RotateObjectsTool::setRotationCenter(const Vec3& position) {
            m_handle.setPosition(position);
            m_toolPage->setCenter(position);
            refreshViews();
        }
        
        void RotateObjectsTool::resetRotationCenter() {
            MapDocumentSPtr document = lock(m_document);
            const BBox3& bounds = document->selectionBounds();
            const Vec3 position = document->grid().snap(bounds.center());
            setRotationCenter(position);
        }
        
        Vec3 RotateObjectsTool::snapRotationCenterMoveDelta(const Vec3& delta) const {
            MapDocumentSPtr document = lock(m_document);
            return document->grid().snap(delta);
        }
        
        void RotateObjectsTool::beginRotation() {
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Rotate Objects");
        }
        
        void RotateObjectsTool::commitRotation() {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
        }
        
        void RotateObjectsTool::cancelRotation() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
        }
        
        FloatType RotateObjectsTool::snapRotationAngle(const FloatType angle) const {
            MapDocumentSPtr document = lock(m_document);
            return document->grid().snapAngle(angle);
        }
        
        void RotateObjectsTool::applyRotation(const Vec3& center, const Vec3& axis, const FloatType angle) {
            MapDocumentSPtr document = lock(m_document);
            document->rollbackTransaction();
            document->rotateObjects(center, axis, angle);
        }
        
        Model::Hit RotateObjectsTool::pick2D(const Ray3& pickRay, const Renderer::Camera& camera) {
            return m_handle.pick2D(pickRay, camera);
        }
        
        Model::Hit RotateObjectsTool::pick3D(const Ray3& pickRay, const Renderer::Camera& camera) {
            return m_handle.pick3D(pickRay, camera);
        }
        
        Vec3 RotateObjectsTool::rotationAxis(const RotateObjectsHandle::HitArea area) const {
            return m_handle.rotationAxis(area);
        }
        
        Vec3 RotateObjectsTool::rotationAxisHandle(const RotateObjectsHandle::HitArea area, const Vec3& cameraPos) const {
            return m_handle.pointHandlePosition(area, cameraPos);
        }
        
        void RotateObjectsTool::renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea area) {
            m_handle.renderHandle2D(renderContext, renderBatch, area);
        }
        
        void RotateObjectsTool::renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea area) {
            m_handle.renderHandle3D(renderContext, renderBatch, area);
        }

        wxWindow* RotateObjectsTool::doCreatePage(wxWindow* parent) {
            assert(m_toolPage == NULL);
            m_toolPage = new RotateObjectsToolPage(parent, m_document, this);
            return m_toolPage;
        }

        String RotateObjectsTool::doGetIconName() const {
            return "RotateTool.png";
        }
    }
}
