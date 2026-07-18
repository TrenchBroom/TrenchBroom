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

#include "base/Uuid.h"

#include <uuid.h>

#include <algorithm>
#include <array>
#include <functional>
#include <random>

namespace tb
{
namespace
{

auto makeRandomSeed()
{
  auto rd = std::random_device{};
  auto data = std::array<int, std::mt19937::state_size>{};
  std::ranges::generate(data, std::ref(rd));
  return data;
}

auto makeRandomGenerator()
{
  const auto randomSeed = makeRandomSeed();
  auto seq = std::seed_seq{std::begin(randomSeed), std::end(randomSeed)};
  return std::mt19937{seq};
}

} // namespace

std::string generateUuid()
{
  // Seed the Mersenne Twister once per thread and reuse it; seeding is an expensive
  // operation.
  static thread_local auto randomGenerator = makeRandomGenerator();

  auto uuidGenerator = uuids::uuid_random_generator{randomGenerator};
  return uuids::to_string(uuidGenerator());
}

} // namespace tb
