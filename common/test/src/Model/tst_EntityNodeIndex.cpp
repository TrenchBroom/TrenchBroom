/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Model/EntityNode.h"
#include "Model/EntityNodeBase.h"
#include "Model/EntityNodeIndex.h"

#include <kdl/vector_utils.h>

#include <string>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
static std::vector<EntityNodeBase*> findExactExact(
  const EntityNodeIndex& index, const std::string& name, const std::string& value)
{
  return index.findEntityNodes(EntityNodeIndexQuery::exact(name), value);
}

static std::vector<EntityNodeBase*> findNumberedExact(
  const EntityNodeIndex& index, const std::string& name, const std::string& value)
{
  return index.findEntityNodes(EntityNodeIndexQuery::numbered(name), value);
}

TEST_CASE("EntityNodeIndexTest.addEntityNode")
{
  EntityNodeIndex index;

  EntityNode* entity1 = new EntityNode({}, {{"test", "somevalue"}});

  EntityNode* entity2 =
    new EntityNode({}, {{"test", "somevalue"}, {"other", "someothervalue"}});

  index.addEntityNode(entity1);
  index.addEntityNode(entity2);

  CHECK(findExactExact(index, "test", "notfound").empty());

  std::vector<EntityNodeBase*> nodes = findExactExact(index, "test", "somevalue");
  CHECK(nodes.size() == 2u);
  CHECK(kdl::vec_contains(nodes, entity1));
  CHECK(kdl::vec_contains(nodes, entity2));

  nodes = findExactExact(index, "other", "someothervalue");
  CHECK(nodes.size() == 1u);
  CHECK(kdl::vec_contains(nodes, entity2));

  delete entity1;
  delete entity2;
}

TEST_CASE("EntityNodeIndexTest.removeEntityNode")
{
  EntityNodeIndex index;

  EntityNode* entity1 = new EntityNode({}, {{"test", "somevalue"}});

  EntityNode* entity2 =
    new EntityNode({}, {{"test", "somevalue"}, {"other", "someothervalue"}});

  index.addEntityNode(entity1);
  index.addEntityNode(entity2);

  index.removeEntityNode(entity2);

  const std::vector<EntityNodeBase*>& nodes = findExactExact(index, "test", "somevalue");
  CHECK(nodes.size() == 1u);
  CHECK(nodes.front() == entity1);

  delete entity1;
  delete entity2;
}

TEST_CASE("EntityNodeIndexTest.addProperty")
{
  EntityNodeIndex index;

  EntityNode* entity1 = new EntityNode({}, {{"test", "somevalue"}});

  EntityNode* entity2 = new EntityNode(
    {},
    {
      {"test", "somevalue"},
    });

  index.addEntityNode(entity1);
  index.addEntityNode(entity2);

  entity2->setEntity(Entity(
    {},
    {
      {"test", "somevalue"},
      {"other", "someothervalue"},
    }));
  index.addProperty(entity2, "other", "someothervalue");

  CHECK(findExactExact(index, "test", "notfound").empty());

  std::vector<EntityNodeBase*> nodes = findExactExact(index, "test", "somevalue");
  CHECK(nodes.size() == 2u);
  CHECK(kdl::vec_contains(nodes, entity1));
  CHECK(kdl::vec_contains(nodes, entity2));

  nodes = findExactExact(index, "other", "someothervalue");
  CHECK(nodes.size() == 1u);
  CHECK(kdl::vec_contains(nodes, entity2));

  delete entity1;
  delete entity2;
}

TEST_CASE("EntityNodeIndexTest.removeProperty")
{
  EntityNodeIndex index;

  EntityNode* entity1 = new EntityNode({}, {{"test", "somevalue"}});

  EntityNode* entity2 =
    new EntityNode({}, {{"test", "somevalue"}, {"other", "someothervalue"}});

  index.addEntityNode(entity1);
  index.addEntityNode(entity2);

  index.removeProperty(entity2, "other", "someothervalue");

  const std::vector<EntityNodeBase*>& nodes = findExactExact(index, "test", "somevalue");
  CHECK(nodes.size() == 2u);
  CHECK(kdl::vec_contains(nodes, entity1));
  CHECK(kdl::vec_contains(nodes, entity2));

  CHECK(findExactExact(index, "other", "someothervalue").empty());

  delete entity1;
  delete entity2;
}

TEST_CASE("EntityNodeIndexTest.addNumberedEntityProperty")
{
  EntityNodeIndex index;

  EntityNode* entity1 =
    new EntityNode({}, {{"test1", "somevalue"}, {"test2", "somevalue"}});

  index.addEntityNode(entity1);

  CHECK(findNumberedExact(index, "test", "notfound").empty());

  std::vector<EntityNodeBase*> nodes = findNumberedExact(index, "test", "somevalue");
  CHECK(nodes.size() == 1u);
  CHECK(kdl::vec_contains(nodes, entity1));

  delete entity1;
}

TEST_CASE("EntityNodeIndexTest.addRemoveFloatProperty")
{
  EntityNodeIndex index;

  EntityNode* entity1 = new EntityNode({}, {{"delay", "3.5"}});

  index.addEntityNode(entity1);

  std::vector<EntityNodeBase*> nodes = findExactExact(index, "delay", "3.5");
  CHECK(nodes.size() == 1u);
  CHECK(kdl::vec_contains(nodes, entity1));

  index.removeProperty(entity1, "delay", "3.5");

  delete entity1;
}

TEST_CASE("EntityNodeIndexTest.allKeys")
{
  EntityNodeIndex index;

  EntityNode* entity1 = new EntityNode({}, {{"test", "somevalue"}});

  EntityNode* entity2 =
    new EntityNode({}, {{"test", "somevalue"}, {"other", "someothervalue"}});

  index.addEntityNode(entity1);
  index.addEntityNode(entity2);

  CHECK_THAT(
    index.allKeys(),
    Catch::Matchers::UnorderedEquals(std::vector<std::string>{"test", "other"}));
}

TEST_CASE("EntityNodeIndexTest.allValuesForKeys")
{
  EntityNodeIndex index;

  EntityNode* entity1 = new EntityNode({}, {{"test", "somevalue"}});

  EntityNode* entity2 =
    new EntityNode({}, {{"test", "somevalue2"}, {"other", "someothervalue"}});

  index.addEntityNode(entity1);
  index.addEntityNode(entity2);

  CHECK_THAT(
    index.allValuesForKeys(EntityNodeIndexQuery::exact("test")),
    Catch::Matchers::UnorderedEquals(
      std::vector<std::string>{"somevalue", "somevalue2"}));
}
} // namespace Model
} // namespace TrenchBroom
