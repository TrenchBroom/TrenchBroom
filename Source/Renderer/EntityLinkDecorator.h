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

#include "Utility/Color.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
    }
    
    namespace Renderer {
        class VertexArray;
        
        class EntityLinkDecorator : public EntityDecorator {
        private:
            Color m_color;
            VertexArray* m_vertexArray;
            bool m_valid;
            bool m_doRebuild;
        public:
            EntityLinkDecorator(const Model::Map& map, const Color& color);
            
            inline void invalidate() {
                m_valid = false;
                m_doRebuild = true;
            }

            void addArrowVerts(Vec4f::List& vList, const Vec3f& pointA, const Vec3f& pointB);
            void gatherLinks(Vec4f::List& vListLocal, Vec4f::List& vListContext, RenderContext& context, Model::Entity& curEnt, Model::EntitySet &visitedEntities);
            void gatherLinksLocal(Vec4f::List& vList, RenderContext& context, const Model::Entity& curEnt);
            void gatherLinksUnrelated(Vec4f::List& vList, RenderContext& context, const Model::Entity& curEnt);
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityLinkDecorator__) */
