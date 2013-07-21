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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__SingleEntityRenderer__
#define __TrenchBroom__SingleEntityRenderer__

#include "Color.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Model {
        class Entity;
    }
    
    namespace Renderer {
        class RenderContext;
        class VboBlock;
        
        class SingleEntityRenderer {
        private:
            const Model::Entity* m_entity;
        public:
            SingleEntityRenderer(const Model::Entity* entity);

            void getBoundsVertices(VertexSpecs::P3C4::Vertex::List& vertices) const;
            void render(RenderContext& context);
        private:
            const Color& boundsColor() const;
        };
    }
}


#endif /* defined(__TrenchBroom__SingleEntityRenderer__) */
