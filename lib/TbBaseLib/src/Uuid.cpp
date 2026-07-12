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

#include "Uuid.h"

#include "stduuid/uuid.h"

#include <algorithm>
#include <array>
#include <functional>
#include <random>

namespace tb
{

std::string generateUuid()
{
  // Seed the Mersenne Twister once per thread and reuse it. Seeding pulls
  // std::mt19937::state_size (624) values from std::random_device, which is very expensive
  // (each draw may hit the OS entropy source). Doing this on every call made node creation
  // dominate map load times, since every node generates a UUID in its constructor. The
  // engine is thread_local so parallel node creation needs no synchronization; advancing
  // its state on each call still yields unique UUIDs.
  static thread_local auto generator = [] {
    auto rd = std::random_device{};
    auto seedData = std::array<int, std::mt19937::state_size>{};
    std::ranges::generate(seedData, std::ref(rd));
    auto seq = std::seed_seq(std::begin(seedData), std::end(seedData));
    return std::mt19937{seq};
  }();

  auto gen = uuids::uuid_random_generator{generator};
  return uuids::to_string(gen());
}

} // namespace tb
