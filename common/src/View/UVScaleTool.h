/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_UVScaleTool
#define TrenchBroom_UVScaleTool

#include "Model/Hit.h"
#include "Renderer/VertexSpec.h"
#include "View/Tool.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class UVViewHelper;
        
        class UVScaleTool : public ToolControllerBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy>, public Tool {
        private:
            static const Model::Hit::HitType XHandleHit;
            static const Model::Hit::HitType YHandleHit;
        private:
            typedef Renderer::VertexSpecs::P3::Vertex EdgeVertex;

            MapDocumentWPtr m_document;
            UVViewHelper& m_helper;
            
            Vec2i m_handle;
            Vec2b m_selector;
            Vec2f m_lastHitPoint; // in non-scaled, non-translated texture coordinates
        public:
            UVScaleTool(MapDocumentWPtr document, UVViewHelper& helper);
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
            
            Vec2i getScaleHandle(const Model::Hit& xHit, const Model::Hit& yHit) const;
            Vec2f getHitPoint(const Ray3& pickRay) const;
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            Vec2f getScaledTranslatedHandlePos() const;
            Vec2f getHandlePos() const;
            Vec2f snap(const Vec2f& position) const;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            EdgeVertex::List getHandleVertices(const Model::PickResult& pickResult) const;
            
            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_UVScaleTool) */
