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

#ifndef TrenchBroom_EntityClassnameAnchor_h
#define TrenchBroom_EntityClassnameAnchor_h

#include "Model/Entity.h"
#include "Renderer/Text/TextRenderer.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Entity;
    }
    
    namespace Renderer {
        class EntityClassnameAnchor : public Text::TextAnchor {
        private:
            Model::Entity& m_entity;
            Vec3f m_position;
        public:
            EntityClassnameAnchor(Model::Entity& entity) : m_entity(entity) {}

            inline const Vec3f& position() {
                m_position = m_entity.center();
                m_position.z += m_entity.bounds().size().z / 2.0f + 3.0f;
                return m_position;
            }
        };
    }
}

#endif
