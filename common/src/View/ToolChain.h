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

#ifndef TrenchBroom_ToolChain
#define TrenchBroom_ToolChain

#include "StringUtils.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        class ToolAdapter;
        
        class ToolChain {
        private:
            ToolAdapter* m_tool;
            ToolChain* m_suffix;
        public:
            ToolChain();
            ~ToolChain();
            
            void append(ToolAdapter* adapter);
            
            void pick(const InputState& inputState, Model::PickResult& pickResult);
            
            void modifierKeyChange(const InputState& inputState);
            
            void mouseDown(const InputState& inputState);
            void mouseUp(const InputState& inputState);
            bool mouseClick(const InputState& inputState);
            bool mouseDoubleClick(const InputState& inputState);
            void mouseScroll(const InputState& inputState);
            void mouseMove(const InputState& inputState);
            
            ToolAdapter* startMouseDrag(const InputState& inputState);
            ToolAdapter* dragEnter(const InputState& inputState, const String& payload);
            
            void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool cancel();
        private:
            bool checkInvariant() const;
            bool chainEndsHere() const;
        };
    }
}

#endif /* defined(TrenchBroom_ToolChain) */
