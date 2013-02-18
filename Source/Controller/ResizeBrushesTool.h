/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__ResizeBrushesTool__
#define __TrenchBroom__ResizeBrushesTool__

#include "Controller/Tool.h"
#include "Model/FaceTypes.h"
#include "Model/Picker.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
        
        namespace HitType {
            static const Type NearEdgeHit      = 1 << 9;
        }
        
        class NearEdgeHit : public Hit {
        private:
            Face& m_dragFace;
            Face& m_referenceFace;
        public:
            NearEdgeHit(const Vec3f& hitPoint, float distance, Face& dragFace, Face& referenceFace);

            bool pickable(Filter& filter) const;

            inline Face& dragFace() const {
                return m_dragFace;
            }
            
            inline Face& referenceFace() const {
                return m_referenceFace;
            }
        };
    }

    namespace Controller {
        class ResizeBrushesTool : public PlaneDragTool {
        protected:
            Model::SelectedFilter m_filter;
            Model::FaceList m_faces;
            Vec3f m_totalDelta;
            
            Model::FaceList dragFaces(Model::Face& dragFace);
            
            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            void handleRenderOverlay(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);

            bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint);
            bool handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint);
            void handleEndPlaneDrag(InputState& inputState);
        public:
            ResizeBrushesTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController);
        };
    }
}

#endif /* defined(__TrenchBroom__ResizeBrushesTool__) */
