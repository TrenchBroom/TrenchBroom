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

#include "UuidGenerator.h"

#include <array>

namespace TrenchBroom {
    namespace Model {
        static auto makeRandomGenerator() {
            auto randomDevice = std::random_device{};
            auto seed_data = std::array<unsigned int, std::mt19937::state_size>{};
            std::generate(std::begin(seed_data), std::end(seed_data), std::ref(randomDevice));
            auto seq = std::seed_seq(std::begin(seed_data), std::end(seed_data));
            return std::mt19937{seq};                    
        }

        UuidGenerator::UuidGenerator() :
        m_randomGenerator{makeRandomGenerator()},
        m_uuidGenerator{uuids::uuid_random_generator{m_randomGenerator}} {}

        std::string UuidGenerator::generateId() {
            return uuids::to_string(m_uuidGenerator());
        }
    }
}
