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

#include "ScaleObjectsTool.h"

#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/RotateObjectsHandle.h"
#include "View/ScaleObjectsToolPage.h"

namespace TrenchBroom {
    namespace View {
        ScaleObjectsTool::ScaleObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_toolPage(nullptr),
        m_scaleCenter(Vec3(0.0, 0.0, 0.0)),
        m_scaleFactors(Vec3(1.0, 1.0, 1.0)) {}

        bool ScaleObjectsTool::doActivate() {
            resetScaleCenter();
            return true;
        }

        const Grid& ScaleObjectsTool::grid() const {
            return lock(m_document)->grid();
        }
        
        Vec3 ScaleObjectsTool::scaleCenter() const {
            return m_scaleCenter;
        }
        
        void ScaleObjectsTool::setScaleCenter(const Vec3& scaleCenter) {
            m_scaleCenter = scaleCenter;
        }
        
        void ScaleObjectsTool::resetScaleCenter() {
            MapDocumentSPtr document = lock(m_document);
            const BBox3& bounds = document->selectionBounds();
            const Vec3 position = document->grid().snap(bounds.center());
            setScaleCenter(position);
        }
        
        Vec3 ScaleObjectsTool::scaleFactors() const {
            return m_scaleFactors;
        }
        
        void ScaleObjectsTool::setScaleFactors(const Vec3& scaleFactors) {
            m_scaleFactors = scaleFactors;
        }

        void ScaleObjectsTool::beginScale() {
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Scale Objects");
        }
        
        void ScaleObjectsTool::commitScale() {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
        }
        
        void ScaleObjectsTool::cancelScale() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
        }
        
//        FloatType ScaleObjectsTool::snapRotationAngle(const FloatType angle) const {
//            MapDocumentSPtr document = lock(m_document);
//            return document->grid().snapAngle(angle);
//        }
        
        void ScaleObjectsTool::applyScale(const Vec3& center, const Vec3& scaleFactors) {
            MapDocumentSPtr document = lock(m_document);
            document->rollbackTransaction();
            document->scaleObjects(center, scaleFactors);
        }
        
        Model::Hit ScaleObjectsTool::pick2D(const Ray3& pickRay, const Renderer::Camera& camera) {
            return Model::Hit::NoHit;
        }
        
        Model::Hit ScaleObjectsTool::pick3D(const Ray3& pickRay, const Renderer::Camera& camera) {
            return Model::Hit::NoHit;
        }
        
        Vec3 ScaleObjectsTool::rotationAxis(const RotateObjectsHandle::HitArea area) const {
            return Vec3::Null;
        }
        
        Vec3 ScaleObjectsTool::rotationAxisHandle(const RotateObjectsHandle::HitArea area, const Vec3& cameraPos) const {
            return Vec3::Null;
        }
        
        void ScaleObjectsTool::renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {

        }
        
        void ScaleObjectsTool::renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {

        }

        void ScaleObjectsTool::renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea area) {
            
        }
        void ScaleObjectsTool::renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const RotateObjectsHandle::HitArea area) {
            
        }

        wxWindow* ScaleObjectsTool::doCreatePage(wxWindow* parent) {
            assert(m_toolPage == nullptr);
            m_toolPage = new ScaleObjectsToolPage(parent, m_document);
            return m_toolPage;
        }
    }
}
