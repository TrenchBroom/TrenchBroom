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

#ifndef __TrenchBroom__EntityLinkRenderer__
#define __TrenchBroom__EntityLinkRenderer__

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/Vbo.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class ModelFilter;
    }
    
    namespace Renderer {
        class RenderContext;
        
        class EntityLinkRenderer {
        private:
            typedef VertexSpecs::P3C4::Vertex Vertex;
            
            View::MapDocumentWPtr m_document;
            
            Color m_defaultColor;
            Color m_selectedColor;
            
            Vbo m_vbo;
            VertexArray m_entityLinks;
            bool m_valid;
        public:
            EntityLinkRenderer(View::MapDocumentWPtr document);
            
            void setDefaultColor(const Color& color);
            void setSelectedColor(const Color& color);
            
            void invalidate();
            void render(RenderContext& renderContext);
        private:
            void validate();
            
            void addTransitiveSelectedLinks(View::MapDocumentSPtr document, Vertex::List& vertices) const;
            void buildLinks(const Model::ModelFilter& filter, Model::EntitySet& visitedEntities, Model::Entity* source, bool isConnectedToSelected, Vertex::List& vertices) const;

            void addAllLinks(View::MapDocumentSPtr document, Vertex::List& vertices) const;
            void addDirectSelectedLinks(View::MapDocumentSPtr document, Vertex::List& vertices) const;
            
            void addSourceLinks(const Model::ModelFilter& filter, const Model::EntityList& entities, Vertex::List& vertices) const;
            void addTargetLinks(const Model::ModelFilter& filter, const Model::EntityList& entities, Vertex::List& vertices) const;
            void addLinks(const Model::ModelFilter& filter, const Model::Entity* source, const Model::EntityList& targets, Vertex::List& vertices) const;
            void addLink(const Model::Entity* source, const Model::Entity* target, Vertex::List& vertices) const;
            
            EntityLinkRenderer(const EntityLinkRenderer& other);
            EntityLinkRenderer& operator=(const EntityLinkRenderer& other);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityLinkRenderer__) */
