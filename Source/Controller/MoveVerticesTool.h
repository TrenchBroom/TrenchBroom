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
#include "Model/Picker.h"
#include "Renderer/Text/TextRenderer.h"
#include "Utility/VecMath.h"

#include <vector>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
    }
    
    namespace Controller {
        class Command;
        
        class MoveVerticesTool : public MoveTool {
        private:
            static const float MaxVertexDistance;
            
            typedef enum {
                VMMove,
                VMSplit,
                VMSnap
            } VertexToolMode;
            
            typedef std::vector<Model::VertexHandleHit*> HandleHitList;
            
            VertexHandleManager m_handleManager;
            VertexToolMode m_mode;
            size_t m_changeCount;
            Renderer::Text::TextRenderer<Vec3f, Vec3f::LexicographicOrder>* m_textRenderer;
            Renderer::Text::TextRenderer<Vec3f, Vec3f::LexicographicOrder>::SimpleTextRendererFilter m_textFilter;
            Vec3f m_dragHandlePosition;
            
            HandleHitList firstHits(Model::PickResult& pickResult) const;
        protected:
            bool isApplicable(InputState& inputState, Vec3f& hitPoint);
            wxString actionName(InputState& inputState);
            void startDrag(InputState& inputState);
            void snapDragDelta(InputState& inputState, Vec3f& delta);
            MoveResult performMove(const Vec3f& delta);

            void rebuildBrushGeometry();
            
            bool handleActivate(InputState& inputState);
            bool handleDeactivate(InputState& inputState);
            bool handleIsModal(InputState& inputState);

            void handlePick(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
        private:
            void initTextRenderer();
            void renderGuide(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, const Vec3f& position);
            void renderHighlight(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, const Vec3f& position);
            void addVertexPositionText(const Vec3f& position);
            void renderHighlightEdges(Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, const Model::HitType::Type firstHitType, HandleHitList& hits);
            void gatherEdgeVertices(Renderer::LinesRenderer& linesRenderer, HandleHitList& hits);
            void gatherFaceEdgeVertices(Renderer::LinesRenderer& linesRenderer, HandleHitList& hits);
            void renderText(Renderer::RenderContext& renderContext);
        protected:
            void handleFreeRenderResources();
            
            bool handleMouseDown(InputState& inputState);
            bool handleMouseUp(InputState& inputState);
            bool handleMouseDClick(InputState& inputState);

            bool handleNavigateUp(InputState& inputState);
            void handleUpdate(const Command& command, InputState& inputState);
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
            
            inline bool hasSelection() const {
                return selectedVertexCount() > 0 || selectedEdgeCount() > 0 || selectedFaceCount() > 0;
            }
            
            inline size_t selectedVertexCount() const {
                return m_handleManager.selectedVertexCount();
            }
            
            inline size_t totalVertexCount() const {
                return m_handleManager.totalVertexCount();
            }
            
            inline size_t selectedEdgeCount() const {
                return m_handleManager.selectedEdgeCount();
            }
            
            inline size_t totalEdgeCount() const {
                return m_handleManager.totalEdgeCount();
            }
            
            inline size_t selectedFaceCount() const {
                return m_handleManager.selectedFaceCount();
            }
            
            inline size_t totalFaceCount() const {
                return m_handleManager.totalFaceCount();
            }

            MoveResult moveVertices(const Vec3f& delta);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveVerticesTool__) */
