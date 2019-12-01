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

#include <gtest/gtest.h>

#include "TestUtils.h"
#include "base/vec_utils.h"
#include "Model/AttributableNode.h"
#include "Model/AttributableNodeIndex.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        static std::vector<AttributableNode*> findExactExact(const AttributableNodeIndex& index, const AttributeName& name, const AttributeValue& value) {
            return index.findAttributableNodes(AttributableNodeIndexQuery::exact(name), value);
        }

        static std::vector<AttributableNode*> findNumberedExact(const AttributableNodeIndex& index, const AttributeName& name, const AttributeValue& value) {
            return index.findAttributableNodes(AttributableNodeIndexQuery::numbered(name), value);
        }

        TEST(EntityAttributeIndexTest, addAttributableNode) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("test", "somevalue");

            Entity* entity2 = new Entity();
            entity2->addOrUpdateAttribute("test", "somevalue");
            entity2->addOrUpdateAttribute("other", "someothervalue");

            index.addAttributableNode(entity1);
            index.addAttributableNode(entity2);

            ASSERT_TRUE(findExactExact(index, "test", "notfound").empty());

            std::vector<AttributableNode*> attributables = findExactExact(index, "test", "somevalue");
            ASSERT_EQ(2u, attributables.size());
            ASSERT_TRUE(VecUtils::contains(attributables, entity1));
            ASSERT_TRUE(VecUtils::contains(attributables, entity2));

            attributables = findExactExact(index, "other", "someothervalue");
            ASSERT_EQ(1u, attributables.size());
            ASSERT_TRUE(VecUtils::contains(attributables, entity2));

            delete entity1;
            delete entity2;
        }

        TEST(EntityAttributeIndexTest, removeAttributableNode) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("test", "somevalue");

            Entity* entity2 = new Entity();
            entity2->addOrUpdateAttribute("test", "somevalue");
            entity2->addOrUpdateAttribute("other", "someothervalue");

            index.addAttributableNode(entity1);
            index.addAttributableNode(entity2);

            index.removeAttributableNode(entity2);

            const std::vector<AttributableNode*>& attributables = findExactExact(index, "test", "somevalue");
            ASSERT_EQ(1u, attributables.size());
            ASSERT_EQ(entity1, attributables.front());

            delete entity1;
            delete entity2;
        }

        TEST(EntityAttributeIndexTest, addAttribute) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("test", "somevalue");

            Entity* entity2 = new Entity();
            entity2->addOrUpdateAttribute("test", "somevalue");

            index.addAttributableNode(entity1);
            index.addAttributableNode(entity2);

            entity2->addOrUpdateAttribute("other", "someothervalue");
            index.addAttribute(entity2, "other", "someothervalue");

            ASSERT_TRUE(findExactExact(index, "test", "notfound").empty());

            std::vector<AttributableNode*> attributables = findExactExact(index, "test", "somevalue");
            ASSERT_EQ(2u, attributables.size());
            ASSERT_TRUE(VecUtils::contains(attributables, entity1));
            ASSERT_TRUE(VecUtils::contains(attributables, entity2));

            attributables = findExactExact(index, "other", "someothervalue");
            ASSERT_EQ(1u, attributables.size());
            ASSERT_TRUE(VecUtils::contains(attributables, entity2));

            delete entity1;
            delete entity2;
        }

        TEST(EntityAttributeIndexTest, removeAttribute) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("test", "somevalue");

            Entity* entity2 = new Entity();
            entity2->addOrUpdateAttribute("test", "somevalue");
            entity2->addOrUpdateAttribute("other", "someothervalue");

            index.addAttributableNode(entity1);
            index.addAttributableNode(entity2);

            index.removeAttribute(entity2, "other", "someothervalue");

            const std::vector<AttributableNode*>& attributables = findExactExact(index, "test", "somevalue");
            ASSERT_EQ(2u, attributables.size());
            ASSERT_TRUE(VecUtils::contains(attributables, entity1));
            ASSERT_TRUE(VecUtils::contains(attributables, entity2));

            ASSERT_TRUE(findExactExact(index, "other", "someothervalue").empty());

            delete entity1;
            delete entity2;
        }

        TEST(EntityAttributeIndexTest, addNumberedEntityAttribute) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("test1", "somevalue");
            entity1->addOrUpdateAttribute("test2", "somevalue");

            index.addAttributableNode(entity1);

            ASSERT_TRUE(findNumberedExact(index, "test", "notfound").empty());

            std::vector<AttributableNode*> attributables = findNumberedExact(index, "test", "somevalue");
            ASSERT_EQ(1u, attributables.size());
            ASSERT_TRUE(VecUtils::contains(attributables, entity1));

            delete entity1;
        }


        TEST(EntityAttributeIndexTest, addRemoveFloatProperty) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("delay", "3.5");

            index.addAttributableNode(entity1);

            std::vector<AttributableNode*> attributables = findExactExact(index, "delay", "3.5");
            ASSERT_EQ(1u, attributables.size());
            ASSERT_TRUE(VecUtils::contains(attributables, entity1));

            index.removeAttribute(entity1, "delay", "3.5");

            delete entity1;
        }

        TEST(EntityAttributeIndexTest, allNames) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("test", "somevalue");

            Entity* entity2 = new Entity();
            entity2->addOrUpdateAttribute("test", "somevalue");
            entity2->addOrUpdateAttribute("other", "someothervalue");

            index.addAttributableNode(entity1);
            index.addAttributableNode(entity2);

            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<String>{ "test", "other" }, index.allNames());
        }

        TEST(EntityAttributeIndexTest, allValuesForNames) {
            AttributableNodeIndex index;

            Entity* entity1 = new Entity();
            entity1->addOrUpdateAttribute("test", "somevalue");

            Entity* entity2 = new Entity();
            entity2->addOrUpdateAttribute("test", "somevalue2");
            entity2->addOrUpdateAttribute("other", "someothervalue");

            index.addAttributableNode(entity1);
            index.addAttributableNode(entity2);

            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<String>{ "somevalue", "somevalue2" }, index.allValuesForNames(AttributableNodeIndexQuery::exact("test")));
        }
    }
}
