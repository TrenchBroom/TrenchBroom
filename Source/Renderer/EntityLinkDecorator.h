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

#ifndef __TrenchBroom__EntityLinkDecorator__
#define __TrenchBroom__EntityLinkDecorator__

#include "Renderer/EntityDecorator.h"

#include "Model/Entity.h"
#include "Utility/Color.h"
#include "View/ViewOptions.h"

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace Renderer {
        class VertexArray;
        
        class EntityLinkDecorator : public EntityDecorator {
        private:
            Color m_color;
            VertexArray* m_selectedLinkArray;
            VertexArray* m_unselectedLinkArray;
            bool m_valid;
            
            void makeLink(Model::Entity& source, Model::Entity& target, Vec3f::List& vertices) const;
            void buildLinks(RenderContext& context, Model::Entity& entity, size_t depth, Model::EntitySet& visitedEntities, Vec3f::List& selectedLinks, Vec3f::List& unselectedLinks) const;
        public:
            EntityLinkDecorator(const Model::MapDocument& document, const Color& color);
            ~EntityLinkDecorator();
            
            inline void invalidate() {
                m_valid = false;
            }

            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityLinkDecorator__) */
