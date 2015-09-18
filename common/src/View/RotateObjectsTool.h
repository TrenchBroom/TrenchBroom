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

#ifndef TrenchBroom_RotateObjectsTool
#define TrenchBroom_RotateObjectsTool

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Hit.h"
#include "View/Tool.h"
#include "View/RotateObjectsHandle.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class RotateObjectsHandle;
        class RotateObjectsToolPage;

        class RotateObjectsTool : public Tool {
        private:
            MapDocumentWPtr m_document;
            RotateObjectsToolPage* m_toolPage;
            RotateObjectsHandle m_handle;
            double m_angle;
            bool m_firstActivation;
        public:
            RotateObjectsTool(MapDocumentWPtr document);

            bool doActivate();

            void updateToolPageAxis(RotateObjectsHandle::HitArea area);
            
            double angle() const;
            void setAngle(double angle);
            
            Vec3 rotationCenter() const;
            void setRotationCenter(const Vec3& position);
            void resetRotationCenter();
            
            Vec3 snapRotationCenterMoveDelta(const Vec3& delta) const;
            
            void beginRotation();
            void commitRotation();
            void cancelRotation();

            FloatType snapRotationAngle(FloatType angle) const;
            void applyRotation(const Vec3& center, const Vec3& axis, FloatType angle);
            
            Model::Hit pick2D(const Ray3& pickRay, const Renderer::Camera& camera);
            Model::Hit pick3D(const Ray3& pickRay, const Renderer::Camera& camera);
            
            Vec3 rotationAxis(RotateObjectsHandle::HitArea area) const;
            Vec3 rotationAxisHandle(RotateObjectsHandle::HitArea area, const Vec3& cameraPos) const;

            void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area);
            void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area);
        private:
            wxWindow* doCreatePage(wxWindow* parent);
            String doGetIconName() const;
        };
    }
}

#endif /* defined(TrenchBroom_RotateObjectsTool) */
