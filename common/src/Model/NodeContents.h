/*
 Copyright (C) 2020 Kristian Duske

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

#pragma once

#include "Model/Brush.h"
#include "Model/Entity.h"

#include <variant>

namespace TrenchBroom {
    namespace Model {
        class NodeContents {
        private:
            std::variant<Entity, Brush> m_contents;
        public:
            /** Unsets cached and derived information of the given objects, i.e.
             *  - for entities, unsets the entity definition and the model
             *  - for brushes, unsets the textures
             */
            explicit NodeContents(std::variant<Entity, Brush> contents);

            std::variant<Entity, Brush>& get();
        };
    }
}