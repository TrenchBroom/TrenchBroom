/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/EntityNodeIndex.h"

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
namespace
{

std::vector<EntityNodeBase*> findExactExact(
  const EntityNodeIndex& index, const std::string& name, const std::string& value)
{
  return index.findEntityNodes(EntityNodeIndexQuery::exact(name), value);
}

std::vector<EntityNodeBase*> findNumberedExact(
  const EntityNodeIndex& index, const std::string& name, const std::string& value)
{
  return index.findEntityNodes(EntityNodeIndexQuery::numbered(name), value);
}

} // namespace

TEST_CASE("EntityNodeIndex")
{
  auto index = EntityNodeIndex{};

  SECTION("addEntityNode")
  {
    auto entity1 = EntityNode{Entity{{{"test", "somevalue"}}}};
    auto entity2 = EntityNode{Entity{{
      {"test", "somevalue"},
      {"other", "someothervalue"},
    }}};

    index.addEntityNode(&entity1);
    index.addEntityNode(&entity2);

    CHECK(findExactExact(index, "test", "notfound").empty());

    CHECK_THAT(
      findExactExact(index, "test", "somevalue"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity1, &entity2}));

    CHECK_THAT(
      findExactExact(index, "other", "someothervalue"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity2}));
  }

  SECTION("removeEntityNode")
  {
    auto entity1 = EntityNode{Entity{{{"test", "somevalue"}}}};

    auto entity2 = EntityNode{Entity{{
      {"test", "somevalue"},
      {"other", "someothervalue"},
    }}};

    index.addEntityNode(&entity1);
    index.addEntityNode(&entity2);

    index.removeEntityNode(&entity2);

    CHECK_THAT(
      findExactExact(index, "test", "somevalue"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity1}));
  }

  SECTION("addProperty")
  {
    auto entity1 = EntityNode{Entity{{{"test", "somevalue"}}}};
    auto entity2 = EntityNode{Entity{{{"test", "somevalue"}}}};

    index.addEntityNode(&entity1);
    index.addEntityNode(&entity2);

    entity2.setEntity(Entity{{
      {"test", "somevalue"},
      {"other", "someothervalue"},
    }});
    index.addProperty(&entity2, "other", "someothervalue");

    CHECK(findExactExact(index, "test", "notfound") == std::vector<EntityNodeBase*>{});

    CHECK_THAT(
      findExactExact(index, "test", "somevalue"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity1, &entity2}));

    CHECK_THAT(
      findExactExact(index, "other", "someothervalue"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity2}));
  }

  SECTION("removeProperty")
  {
    auto entity1 = EntityNode{Entity{{{"test", "somevalue"}}}};

    auto entity2 = EntityNode{Entity{{
      {"test", "somevalue"},
      {"other", "someothervalue"},
    }}};

    index.addEntityNode(&entity1);
    index.addEntityNode(&entity2);

    index.removeProperty(&entity2, "other", "someothervalue");

    CHECK_THAT(
      findExactExact(index, "test", "somevalue"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity1, &entity2}));

    CHECK(
      findExactExact(index, "other", "someothervalue") == std::vector<EntityNodeBase*>{});
  }

  SECTION("addNumberedEntityProperty")
  {
    auto entity1 = EntityNode{Entity{{
      {"test1", "somevalue"},
      {"test2", "somevalue"},
    }}};

    index.addEntityNode(&entity1);

    CHECK(findNumberedExact(index, "test", "notfound") == std::vector<EntityNodeBase*>{});

    CHECK_THAT(
      findNumberedExact(index, "test", "somevalue"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity1}));
  }

  SECTION("addRemoveFloatProperty")
  {
    auto entity1 = EntityNode{Entity{{{"delay", "3.5"}}}};

    index.addEntityNode(&entity1);
    CHECK_THAT(
      findExactExact(index, "delay", "3.5"),
      Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{&entity1}));

    index.removeProperty(&entity1, "delay", "3.5");
    CHECK(findNumberedExact(index, "delay", "3.5") == std::vector<EntityNodeBase*>{});
  }

  SECTION("allKeys")
  {
    auto entity1 = EntityNode{Entity{{{"test", "somevalue"}}}};

    auto entity2 = EntityNode{Entity{{
      {"test", "somevalue"},
      {"other", "someothervalue"},
    }}};

    index.addEntityNode(&entity1);
    index.addEntityNode(&entity2);

    CHECK_THAT(
      index.allKeys(),
      Catch::Matchers::UnorderedEquals(std::vector<std::string>{"test", "other"}));
  }

  SECTION("allValuesForKeys")
  {
    auto entity1 = EntityNode{Entity{{{"test", "somevalue"}}}};

    auto entity2 = EntityNode{Entity{{
      {"test", "somevalue2"},
      {"other", "someothervalue"},
    }}};

    index.addEntityNode(&entity1);
    index.addEntityNode(&entity2);

    CHECK_THAT(
      index.allValuesForKeys(EntityNodeIndexQuery::exact("test")),
      Catch::Matchers::UnorderedEquals(
        std::vector<std::string>{"somevalue", "somevalue2"}));
  }
}

} // namespace tb::mdl
