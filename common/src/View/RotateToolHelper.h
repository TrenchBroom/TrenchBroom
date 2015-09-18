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

#ifndef TrenchBroom_RotateToolHelper
#define TrenchBroom_RotateToolHelper

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/ToolAdapter.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        
        struct RotateInfo {
            Vec3 center;
            Vec3 axis;
            Vec3 origin;
            Plane3 plane;
        };
        
        class RotateToolDelegate {
        public:
            virtual ~RotateToolDelegate();
            
            bool handleRotate(const InputState& inputState) const;
            RotateInfo getRotateInfo(const InputState& inputState) const;
            bool startRotate(const InputState& inputState);
            FloatType getAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const;
            bool rotate(const Vec3& center, const Vec3& axis, const FloatType angle);
            void endRotate(const InputState& inputState);
            void cancelRotate();
        private:
            virtual bool doHandleRotate(const InputState& inputState) const = 0;
            virtual RotateInfo doGetRotateInfo(const InputState& inputState) const = 0;
            virtual bool doStartRotate(const InputState& inputState) = 0;
            virtual FloatType doGetAngle(const InputState& inputState, const Vec3& handlePoint, const Vec3& curPoint, const Vec3& axis) const = 0;
            virtual bool doRotate(const Vec3& center, const Vec3& axis, FloatType angle) = 0;
            virtual void doEndRotate(const InputState& inputState) = 0;
            virtual void doCancelRotate() = 0;
        };
        
        class RotateToolHelper : public PlaneDragHelper {
        private:
            static const size_t SnapAngleKey;
            static const size_t AngleKey;
            
            RotateToolDelegate& m_delegate;
            Vec3 m_center;
            Vec3 m_axis;
            FloatType m_lastAngle;
            Vec3 m_firstPoint;
            
            class AngleIndicatorRenderer;
        public:
            RotateToolHelper(PlaneDragPolicy* policy, RotateToolDelegate& delegate);
            
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            
        private:
            void renderAngleIndicator(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderText(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            String angleString(const FloatType angle) const;
        };
    }
}

#endif /* defined(TrenchBroom_RotateToolHelper) */
