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

#ifndef __TrenchBroom__TextureTool__
#define __TrenchBroom__TextureTool__

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/MoveTool.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
    }
    
    namespace Renderer {
        class EdgeRenderer;
    }
    
    namespace View {
        class TextureTool : public Tool<ActivationPolicy, NoPickingPolicy, MousePolicy, PlaneDragPolicy, NoDropPolicy, RenderPolicy> {
        private:
            Model::BrushFace* m_face;
        public:
            TextureTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller);
        private:
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
            
            void doMouseMove(const InputState& inputState);

            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag(const InputState& inputState);
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
            Renderer::EdgeRenderer buildEdgeRenderer(const Model::BrushFace* face) const;

            bool applies(const InputState& inputState) const;
            
            void performMove(const Vec3& delta);
            
            Vec3::List findApplicablePlaneNormals(const Model::BrushFaceList& faces) const;
            size_t countPossibleAxes(const Vec3& normal) const;
            void countPossibleAxes(const Vec3& normal, size_t (&counts)[3]) const;
            size_t countPossibleAxes(const size_t (&counts)[3]) const;
            Vec3::List selectApplicablePlaneNormals(const size_t (&counts)[3], const Model::BrushFace* face) const;
            void getPlaneNormals(const size_t (&counts)[3], Vec3::List& normals) const;

            Model::BrushFaceList selectApplicableFaces(const Model::BrushFaceList& faces, const Vec3::List& normals) const;
            
            void performMove(const Vec3& delta, const Model::BrushFaceList& faces, const Vec3::List& normals);
            Vec3 rotateDelta(const Vec3& delta, const Model::BrushFace* face, const Vec3::List& normals) const;
            Vec3 disambiguateNormal(const Model::BrushFace* face, const Vec3::List& normals) const;
        };
    }
}

#endif /* defined(__TrenchBroom__TextureTool__) */
