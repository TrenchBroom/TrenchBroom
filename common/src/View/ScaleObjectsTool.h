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
#include "View/Tool.h"
#include "View/RotateObjectsHandle.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class Grid;
        class RotateObjectsHandle;
        class ScaleObjectsToolPage;

        class ScaleObjectsTool : public Tool {
        private:
            MapDocumentWPtr m_document;
            ScaleObjectsToolPage* m_toolPage;
            Vec3 m_scaleCenter;
            Vec3 m_scaleFactors;
        public:
            ScaleObjectsTool(MapDocumentWPtr document);

            bool doActivate() override;

            const Grid& grid() const;
            
            Vec3 scaleCenter() const;
            void setScaleCenter(const Vec3& scaleCenter);
            void resetScaleCenter();
                
            Vec3 scaleFactors() const;
            void setScaleFactors(const Vec3& scaleFactors);
            
            void beginScale();
            void commitScale();
            void cancelScale();

            FloatType snapRotationAngle(FloatType angle) const;
            void applyRotation(const Vec3& center, const Vec3& axis, FloatType angle);
            
            Model::Hit pick2D(const Ray3& pickRay, const Renderer::Camera& camera);
            Model::Hit pick3D(const Ray3& pickRay, const Renderer::Camera& camera);
            
            Vec3 rotationAxis(RotateObjectsHandle::HitArea area) const;
            Vec3 rotationAxisHandle(RotateObjectsHandle::HitArea area, const Vec3& cameraPos) const;

            void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area);
            void renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area);
        private:
            wxWindow* doCreatePage(wxWindow* parent) override;
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsTool) */
