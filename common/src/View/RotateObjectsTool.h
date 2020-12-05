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

#pragma once

#include "FloatType.h"
#include "View/Tool.h"
#include "View/RotateObjectsHandle.h"

#include <vecmath/forward.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class Grid;
        class MapDocument;
        class RotateObjectsToolPage;

        class RotateObjectsTool : public Tool {
        private:
            std::weak_ptr<MapDocument> m_document;
            RotateObjectsToolPage* m_toolPage;
            RotateObjectsHandle m_handle;
            double m_angle;
            std::vector<vm::vec3> m_recentlyUsedCenters;
        public:
            explicit RotateObjectsTool(std::weak_ptr<MapDocument> document);

            bool doActivate() override;

            const Grid& grid() const;

            void updateToolPageAxis(RotateObjectsHandle::HitArea area);

            double angle() const;
            void setAngle(double angle);

            vm::vec3 rotationCenter() const;
            void setRotationCenter(const vm::vec3& position);
            void resetRotationCenter();

            FloatType majorHandleRadius(const Renderer::Camera& camera) const;
            FloatType minorHandleRadius(const Renderer::Camera& camera) const;

            void beginRotation();
            void commitRotation();
            void cancelRotation();

            FloatType snapRotationAngle(FloatType angle) const;
            void applyRotation(const vm::vec3& center, const vm::vec3& axis, FloatType angle);

            Model::Hit pick2D(const vm::ray3& pickRay, const Renderer::Camera& camera);
            Model::Hit pick3D(const vm::ray3& pickRay, const Renderer::Camera& camera);

            vm::vec3 rotationAxis(RotateObjectsHandle::HitArea area) const;

            void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area);
            void renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area);
        private:
            void updateRecentlyUsedCenters(const vm::vec3& center);
        private:
            QWidget* doCreatePage(QWidget* parent) override;
        };
    }
}

