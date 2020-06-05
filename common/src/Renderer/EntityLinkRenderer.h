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
#include "Renderer/GLVertex.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

#include <vecmath/forward.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocument; // FIXME: Renderer should not depend on View
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;

        class EntityLinkRenderer : public DirectRenderable {
        private:
            using Vertex = GLVertexTypes::P3C4::Vertex;
            
            using ArrowVertex = GLVertexType<
                    GLVertexAttributeTypes::P3,  // vertex of the arrow (exposed in shader as gl_Vertex)
                    GLVertexAttributeTypes::C4,  // arrow color (exposed in shader as gl_Color)
                    GLVertexAttributeUser<AttributeNames::arrowPosition, GL_FLOAT, 3, Normalize::False>,          // arrow position
                    GLVertexAttributeUser<AttributeNames::lineDir,       GL_FLOAT, 3, Normalize::False>>::Vertex; // direction the arrow is pointing

            std::weak_ptr<View::MapDocument> m_document;

            Color m_defaultColor;
            Color m_selectedColor;

            VertexArray m_entityLinks;
            VertexArray m_entityLinkArrows;

            bool m_valid;
        public:
            EntityLinkRenderer(std::weak_ptr<View::MapDocument> document);

            void setDefaultColor(const Color& color);
            void setSelectedColor(const Color& color);

            void render(RenderContext& renderContext, RenderBatch& renderBatch);
            void invalidate();
        private:
            void doPrepareVertices(VboManager& vboManager) override;
            void doRender(RenderContext& renderContext) override;
            void renderLines(RenderContext& renderContext);
            void renderArrows(RenderContext& renderContext);
        private:
            void validate();

            static void getArrows(std::vector<ArrowVertex>& arrows, const std::vector<Vertex>& links);
            static void addArrow(std::vector<ArrowVertex>& arrows, const vm::vec4f& color, const vm::vec3f& arrowPosition, const vm::vec3f& lineDir);

            class MatchEntities;
            class CollectEntitiesVisitor;

            class CollectLinksVisitor;
            class CollectAllLinksVisitor;
            class CollectTransitiveSelectedLinksVisitor;
            class CollectDirectSelectedLinksVisitor;

            void getLinks(std::vector<Vertex>& links) const;
            void getAllLinks(std::vector<Vertex>& links) const;
            void getTransitiveSelectedLinks(std::vector<Vertex>& links) const;
            void getDirectSelectedLinks(std::vector<Vertex>& links) const;
            void collectSelectedLinks(CollectLinksVisitor& collectLinks) const;

            EntityLinkRenderer(const EntityLinkRenderer& other);
            EntityLinkRenderer& operator=(const EntityLinkRenderer& other);
        };
    }
}

#endif /* defined(TrenchBroom_EntityLinkRenderer) */
