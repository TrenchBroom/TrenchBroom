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

#ifndef __TrenchBroom__CreateEntityFromMenuHelper__
#define __TrenchBroom__CreateEntityFromMenuHelper__

#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class MapDocument;
        class PointEntityDefinition;
    }

    namespace Renderer {
        class EntityFigure;
        class RenderContext;
        class Vbo;
    }

    namespace Controller {
        class CreateEntityFromMenuHelper {
        private:
            Model::MapDocument& m_document;
            Model::Entity* m_entity;
            Renderer::EntityFigure* m_figure;
        public:
            CreateEntityFromMenuHelper(Model::MapDocument& document);
            ~CreateEntityFromMenuHelper();

            void show(Model::PointEntityDefinition& definition, const Vec3f& origin);
            void hide();

            void render(Renderer::Vbo& vbo, Renderer::RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__CreateEntityFromMenuHelper__) */
