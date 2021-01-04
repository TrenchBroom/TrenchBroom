/*
 Copyright (C) 2020 MaxED

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

#include "EL/Expression.h"
#include "IO/Path.h"

#include <iosfwd>

namespace TrenchBroom {
    namespace Model {
        class EntityAttributes;
    }

    namespace Assets {
        class SpriteDefinition {
        private:
            EL::Expression m_expression;
        public:
            SpriteDefinition();
            SpriteDefinition(size_t line, size_t column);
            explicit SpriteDefinition(const EL::Expression& expression);

            friend bool operator==(const SpriteDefinition& lhs, const SpriteDefinition& rhs);
            friend bool operator!=(const SpriteDefinition& lhs, const SpriteDefinition& rhs);
            friend std::ostream& operator<<(std::ostream& str, const SpriteDefinition& def);

            void append(const SpriteDefinition& other);

            std::string spritePath(const EL::VariableStore& variableStore) const;
            std::string defaultSpritePath() const;

        private:
            static std::string convertToSprite(const EL::Value& value);
            static std::string toString(const EL::Value& value);
        };
    }
}

