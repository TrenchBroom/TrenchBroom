/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Value.h"
#include "el/VariableStore.h"
#include "mdl/Entity.h"
#include "mdl/EntityPropertiesVariableStore.h"

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("EntityPropertiesVariableStore")
{
  const auto entity = Entity{{{"classname", "info_player_start"}}};

  SECTION("clone")
  {
    const auto store = EntityPropertiesVariableStore{entity};
    const auto clone = std::unique_ptr<el::VariableStore>{store.clone()};

    CHECK(clone->size() == 1u);
    CHECK(clone->value("classname") == el::Value{"info_player_start"});
  }

  SECTION("size")
  {
    CHECK(EntityPropertiesVariableStore{entity}.size() == 1u);
    CHECK(EntityPropertiesVariableStore{Entity{}}.size() == 0u);
  }

  SECTION("value")
  {
    const auto store = EntityPropertiesVariableStore{entity};
    CHECK(store.value("classname") == el::Value{"info_player_start"});

    // unlike BoundValueStore's generic Undefined fallback, an unknown property is an
    // empty string here -- a pre-existing quirk this store preserves exactly
    CHECK(store.value("missing") == el::Value{""});
  }

  SECTION("names")
  {
    CHECK(
      EntityPropertiesVariableStore{entity}.names()
      == std::vector<std::string>{"classname"});
  }

  SECTION("set")
  {
    auto store = EntityPropertiesVariableStore{entity};
    store.set("classname", el::Value{"other"});

    CHECK(store.value("classname") == el::Value{"info_player_start"});
  }
}

} // namespace tb::mdl
