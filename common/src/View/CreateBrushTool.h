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

#ifndef __TrenchBroom__CreateBrushTool__
#define __TrenchBroom__CreateBrushTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/BoundsGuideRenderer.h"
#include "Renderer/BrushRenderer.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
    }
    
    namespace Renderer {
        class Camera;
        class BrushRenderer;
        class RenderContext;
        class TextureFont;
    }
    
    namespace View {
        class CreateBrushTool : public ToolImpl<NoActivationPolicy, NoPickingPolicy, NoMousePolicy, PlaneDragPolicy, NoDropPolicy, RenderPolicy> {
        private:
            const Renderer::Camera& m_camera;
            Vec3 m_initialPoint;
            Renderer::BrushRenderer m_brushRenderer;
            Renderer::BoundsGuideRenderer m_guideRenderer;
            Model::Brush* m_brush;
        public:
            CreateBrushTool(MapDocumentWPtr document, ControllerWPtr controller, const Renderer::Camera& camera, Renderer::TextureFont& font);
        private:
            void doModifierKeyChange(const InputState& inputState);
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag(const InputState& inputState);

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);

            BBox3 computeBounds(const Vec3& point1, const Vec3& point2) const;
            bool checkBounds(const BBox3& bounds) const;
            Model::Brush* createBrush(const BBox3& bounds) const;
            void setTexture(Model::Brush* brush) const;
            void updateBrushRenderer();
            void updateGuideRenderer(const BBox3& bounds);
            void addBrushToMap(Model::Brush* brush);
        };
    }
}

#endif /* defined(__TrenchBroom__CreateBrushTool__) */
