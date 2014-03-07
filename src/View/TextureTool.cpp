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

#include "TextureTool.h"

#include "Renderer/RenderContext.h"
#include "View/MoveTextureHelper.h"
#include "View/TextureCoordSystemHelper.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TextureToolHelper::~TextureToolHelper() {}

        bool TextureToolHelper::startDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            return doStartDrag(inputState, plane, initialPoint);
        }
        
        bool TextureToolHelper::drag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            return doDrag(inputState, lastPoint, curPoint, refPoint);
        }
        
        void TextureToolHelper::endDrag(const InputState& inputState) {
            return doEndDrag(inputState);
        }
        
        void TextureToolHelper::cancelDrag(const InputState& inputState) {
            return doCancelDrag(inputState);
        }
        
        void TextureToolHelper::setRenderOptions(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext) const {
            doSetRenderOptions(inputState, dragging, renderContext);
        }
        
        void TextureToolHelper::render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext) {
            doRender(inputState, dragging, renderContext);
        }

        TextureTool::TextureTool(MapDocumentWPtr document, ControllerWPtr controller) :
        Tool(document, controller),
        m_textureCoordSystemHelper(new TextureCoordSystemHelper(document, controller)),
        m_moveTextureHelper(new MoveTextureHelper(document, controller)),
        m_currentHelper(NULL) {}

        TextureTool::~TextureTool() {
            delete m_textureCoordSystemHelper;
            m_textureCoordSystemHelper = NULL;
            delete m_moveTextureHelper;
            m_moveTextureHelper = NULL;
            m_currentHelper = NULL;
        }

        bool TextureTool::initiallyActive() const {
            return false;
        }
        
        bool TextureTool::doActivate(const InputState& inputState) {
            return true;
        }
        
        bool TextureTool::doDeactivate(const InputState& inputState) {
            return true;
        }

        bool TextureTool::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_currentHelper == NULL);
            if (m_textureCoordSystemHelper->startDrag(inputState, plane, initialPoint))
                m_currentHelper = m_textureCoordSystemHelper;
            else if (m_moveTextureHelper->startDrag(inputState, plane, initialPoint))
                m_currentHelper = m_moveTextureHelper;

            return m_currentHelper != NULL;
        }
        
        bool TextureTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_currentHelper != NULL);
            return m_currentHelper->drag(inputState, lastPoint, curPoint, refPoint);
        }
        
        void TextureTool::doEndPlaneDrag(const InputState& inputState) {
            assert(m_currentHelper != NULL);
            m_currentHelper->endDrag(inputState);
            m_currentHelper = NULL;
        }
        
        void TextureTool::doCancelPlaneDrag(const InputState& inputState) {
            assert(m_currentHelper != NULL);
            m_currentHelper->cancelDrag(inputState);
            m_currentHelper = NULL;
        }

        void TextureTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            if (m_currentHelper != NULL) {
                m_currentHelper->setRenderOptions(inputState, dragging(), renderContext);
            } else {
                renderContext.clearTintSelection();
                renderContext.setForceHideSelectionGuide();
                m_textureCoordSystemHelper->setRenderOptions(inputState, dragging(), renderContext);
                m_moveTextureHelper->setRenderOptions(inputState, dragging(), renderContext);
            }
        }
        
        void TextureTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (m_currentHelper != NULL) {
                m_currentHelper->render(inputState, dragging(), renderContext);
            } else {
                m_textureCoordSystemHelper->render(inputState, dragging(), renderContext);
                m_moveTextureHelper->render(inputState, dragging(), renderContext);
            }
        }
    }
}
