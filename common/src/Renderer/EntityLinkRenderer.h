/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_EntityLinkRenderer
#define TrenchBroom_EntityLinkRenderer

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/Renderable.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
        
        class EntityLinkRenderer : public DirectRenderable {
        private:
            // Due to current api limitations we can't have multiple vertex formats per Renderable
            // so we'll waste some attribute space on the lines

            //using Vertex = VertexSpecs::P3C4::Vertex;

            // Hack...
            using T03 = AttributeSpec<AttributeType_TexCoord0, GL_FLOAT, 3>;
            using T13 = AttributeSpec<AttributeType_TexCoord1, GL_FLOAT, 3>;

            using Vertex = VertexSpec4<
                    AttributeSpecs::P3,          // vertex of the arrow
                    AttributeSpecs::C4,          // arrow color
                    T03,                          // arrow location
                    T13>::Vertex;                 // direction the arrow is pointing

            View::MapDocumentWPtr m_document;
            
            Color m_defaultColor;
            Color m_selectedColor;
            
            VertexArray m_entityLinks;
            VertexArray m_entityArrows;

            bool m_valid;
        public:
            EntityLinkRenderer(View::MapDocumentWPtr document);
            
            void setDefaultColor(const Color& color);
            void setSelectedColor(const Color& color);
            
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
            void invalidate();
        private:
            void doPrepareVertices(Vbo& vertexVbo) override;
            void doRender(RenderContext& renderContext) override;
        private:
            void validate();
            
            class MatchEntities;
            class CollectEntitiesVisitor;
            
            class CollectLinksVisitor;
            class CollectAllLinksVisitor;
            class CollectTransitiveSelectedLinksVisitor;
            class CollectDirectSelectedLinksVisitor;

            void getLinks(Vertex::List& links) const;
            void getAllLinks(Vertex::List& links) const;
            void getTransitiveSelectedLinks(Vertex::List& links) const;
            void getDirectSelectedLinks(Vertex::List& links) const;
            void collectSelectedLinks(CollectLinksVisitor& collectLinks) const;
            
            EntityLinkRenderer(const EntityLinkRenderer& other);
            EntityLinkRenderer& operator=(const EntityLinkRenderer& other);
        };
    }
}

#endif /* defined(TrenchBroom_EntityLinkRenderer) */
