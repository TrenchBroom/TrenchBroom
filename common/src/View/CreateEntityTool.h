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

#ifndef __TrenchBroom__CreateEntityTool__
#define __TrenchBroom__CreateEntityTool__

#include "StringUtils.h"
#include "Renderer/EntityRenderer.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Entity;
    }
    
    namespace Renderer {
        class Camera;
        class FontManager;
    }
    
    namespace View {
        class CreateEntityTool : public ToolImpl<NoActivationPolicy, NoPickingPolicy, NoMousePolicy, NoMouseDragPolicy, DropPolicy, NoRenderPolicy> {
        private:
            const Renderer::Camera& m_camera;
            Model::Entity* m_entity;
        public:
            CreateEntityTool(MapDocumentWPtr document, ControllerWPtr controller, const Renderer::Camera& camera, Renderer::FontManager& fontManager);
        private:
            bool doDragEnter(const InputState& inputState, const String& payload);
            bool doDragMove(const InputState& inputState);
            void doDragLeave(const InputState& inputState);
            bool doDragDrop(const InputState& inputState);
            void updateEntityPosition(const InputState& inputState);
        };
    }
}

#endif /* defined(__TrenchBroom__CreateEntityTool__) */
