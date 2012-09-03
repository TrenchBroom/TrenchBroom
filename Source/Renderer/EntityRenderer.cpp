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

#include "EntityRenderer.h"

#include "Model/Entity.h"
#include "Utility/GLee.h"

namespace TrenchBroom {
    namespace Renderer {
        void EntityRenderer::render(const Model::Entity& entity) {
            render(entity.origin(), static_cast<float>(entity.angle()));
        }

        void EntityRenderer::render(const Vec3f& position, float angle) {
            glPushMatrix();
            
            glTranslatef(position.x, position.y, position.z);
            if (angle != 0.0f) {
                if (angle == -1.0f)
                    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
                else if (angle == -2.0f)
                    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                else
                    glRotatef(angle, 0.0f, 0.0f, 1.0f);
            }
            
            render();
            glPopMatrix();
        }
    }
}