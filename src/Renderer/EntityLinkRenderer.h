/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class EntityLinkRenderer {
        public:
            class Filter {
            public:
                bool showLink(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const;
                const Color& linkColor(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const;
                const Color& killColor(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const;
            private:
                virtual bool doGetShowLink(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const = 0;
                virtual const Color& doGetLinkColor(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const = 0;
                virtual const Color& doGetKillColor(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const = 0;
            };
        private:
            typedef VertexSpecs::P3C4::Vertex Vertex;
            
            Vbo m_vbo;
            VertexArray m_entityLinks;
            bool m_valid;
        public:
            EntityLinkRenderer();
            
            void invalidate();
            void validate(const Filter& filter, const Model::EntityList& unselectedEntities, const Model::EntityList& selectedEntities);
            void render(RenderContext& renderContext);
        private:
            void buildLinks(const Filter& filter, Model::EntitySet& visitedEntities, Model::Entity* source, bool isConnectedToSelected, Vertex::List& vertices) const;
            void addLink(const Model::Entity* source, const Model::Entity* target, const Color& color, Vertex::List& vertices) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityLinkRenderer__) */
