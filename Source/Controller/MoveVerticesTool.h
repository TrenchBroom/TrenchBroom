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

#ifndef __TrenchBroom__MoveVerticesTool__
#define __TrenchBroom__MoveVerticesTool__

#include "Controller/MoveTool.h"
#include "Controller/VertexHandleManager.h"
#include "Renderer/Text/TextRenderer.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
    }
    
    namespace Controller {
        class MoveVerticesTool : public MoveTool {
        protected:
            typedef enum {
                VMMove,
                VMSplit,
                VMSnap
            } VertexToolMode;
            
            VertexHandleManager m_handleManager;
            VertexToolMode m_mode;
            bool m_ignoreObjectChanges;
            size_t m_changeCount;
            Renderer::Text::TextRenderer<Vec3f, Vec3f::LexicographicOrder>* m_textRenderer;
            Renderer::Text::TextRenderer<Vec3f, Vec3f::LexicographicOrder>::SimpleTextRendererFilter m_textFilter;
            Vec3f m_dragHandlePosition;
            
            bool isApplicable(InputState& inputState, Vec3f& hitPoint);
            wxString actionName();
            void startDrag(InputState& inputState);
            void snapDragDelta(InputState& inputState, Vec3f& delta);
            MoveResult performMove(const Vec3f& delta);

            bool handleActivate(InputState& inputState);
            bool handleDeactivate(InputState& inputState);
            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            void handleFreeRenderResources();
            
            bool handleMouseDown(InputState& inputState);
            bool handleMouseUp(InputState& inputState);
            bool handleMouseDClick(InputState& inputState);
            void handleMouseMove(InputState& inputState);

            void handleObjectsChange(InputState& inputState);
            void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet);
        public:
            MoveVerticesTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController, float axisLength, float planeRadius, float vertexSize);
        
            inline void setChangeCount(size_t changeCount) {
                assert(active());
                m_changeCount = changeCount;
            }
            
            inline void incChangeCount() {
                assert(active());
                m_changeCount++;
            }
            
            inline void decChangeCount() {
                assert(active());
                assert(m_changeCount > 0);
                m_changeCount--;
            }
            
            bool hasSelection();
            MoveResult moveVertices(const Vec3f& delta);
            
            void resetInstancedRenderers();
        };
    }
}

#endif /* defined(__TrenchBroom__MoveVerticesTool__) */
