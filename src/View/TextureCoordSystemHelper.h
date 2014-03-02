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

#ifndef __TrenchBroom__TextureCoordSystemHelper__
#define __TrenchBroom__TextureCoordSystemHelper__

#include "View/Tool.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"
#include "View/TextureTool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class EdgeRenderer;
    }
    
    namespace View {
        class TextureCoordSystemHelper : public TextureToolHelper {
        private:
            View::MapDocumentWPtr m_document;
            View::ControllerWPtr m_controller;
            Model::BrushFace* m_face;
        public:
            TextureCoordSystemHelper(View::MapDocumentWPtr document, View::ControllerWPtr controller);
        private:
            bool doStartDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndDrag(const InputState& inputState);
            void doCancelDrag(const InputState& inputState);
            
            void doSetRenderOptions(const InputState& inputState, bool dragging, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, bool dragging, Renderer::RenderContext& renderContext);
            
            bool applies(const InputState& inputState) const;
        };
    }
}

#endif /* defined(__TrenchBroom__TextureCoordSystemHelper__) */
