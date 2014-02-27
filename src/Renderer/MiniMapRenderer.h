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

#ifndef __TrenchBroom__MiniMapRenderer__
#define __TrenchBroom__MiniMapRenderer__

#include "TrenchBroom.h"
#include "VecMath.h"

#include "Model/ModelTypes.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace Model {
        class SelectionResult;
    }
    
    namespace Renderer {
        class Camera;
        class RenderContext;
        
        class MiniMapRenderer {
        private:
            struct BuildBrushEdges {
                VertexSpecs::P3::Vertex::List vertices;
                void operator()(Model::Brush* brush);
            };

            View::MapDocumentWPtr m_document;
            Vbo m_vbo;
            VertexArray m_unselectedEdgeArray;
            VertexArray m_selectedEdgeArray;
            bool m_unselectedValid;
            bool m_selectedValid;
        public:
            MiniMapRenderer(View::MapDocumentWPtr document);
            ~MiniMapRenderer();
            
            void render(RenderContext& context, const BBox3f& bounds, const Camera& camera3D);
        private:
            void setupGL(RenderContext& context);
            void renderEdges(RenderContext& context, const BBox3f& bounds);
            void renderCamera(RenderContext& context, const Camera& camera3D);
            
            void validateEdges(RenderContext& context);
            VertexArray buildVertexArray(const Model::BrushList& brushes) const;
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasCleared();
            void documentWasNewedOrLoaded();
            
            void objectWasAdded(Model::Object* object);
            void objectWillBeRemoved(Model::Object* object);
            void objectDidChange(Model::Object* object);
            void selectionDidChange(const Model::SelectionResult& result);
        };
    }
}

#endif /* defined(__TrenchBroom__MiniMapRenderer__) */
