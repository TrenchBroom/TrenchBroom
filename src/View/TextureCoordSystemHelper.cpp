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

#include "TextureCoordSystemHelper.h"
#include "Model/Brush.h"
#include "Model/HitFilters.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        TextureCoordSystemHelper::TextureCoordSystemHelper(View::MapDocumentWPtr document, View::ControllerWPtr controller) :
        m_document(document),
        m_controller(controller),
        m_face(NULL) {}
        
        bool TextureCoordSystemHelper::doStartDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_face == NULL);
            if (!applies(inputState))
                return false;
            return true;
        }
        
        bool TextureCoordSystemHelper::doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            return true;
        }
        
        void TextureCoordSystemHelper::doEndDrag(const InputState& inputState) {
            m_face = NULL;
        }
        
        void TextureCoordSystemHelper::doCancelDrag(const InputState& inputState) {
            m_face = NULL;
        }
        
        void TextureCoordSystemHelper::doSetRenderOptions(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext) const {
        }
        
        void TextureCoordSystemHelper::doRender(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext) {
        }
        
        bool TextureCoordSystemHelper::applies(const InputState& inputState) const {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            View::MapDocumentSPtr document = lock(m_document);
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document->filter(), true);
            if (!first.matches)
                return false;
            return false;
        }
    }
}
