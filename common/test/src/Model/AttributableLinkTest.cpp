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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/World.h"

#include <kdl/vector_utils.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("AttributableNodeLinkTest.testCreateLink", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("AttributableNodeLinkTest.testCreateMultiSourceLink", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source1 = world.createEntity();
            Entity* source2 = world.createEntity();
            Entity* target = world.createEntity();
            world.defaultLayer()->addChild(source1);
            world.defaultLayer()->addChild(source2);
            world.defaultLayer()->addChild(target);

            source1->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            source2->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            const std::vector<AttributableNode*>& targets1 = source1->linkTargets();
            ASSERT_EQ(1u, targets1.size());
            ASSERT_EQ(target, targets1.front());

            const std::vector<AttributableNode*>& targets2 = source2->linkTargets();
            ASSERT_EQ(1u, targets2.size());
            ASSERT_EQ(target, targets2.front());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_EQ(2u, sources.size());
            ASSERT_TRUE(kdl::vec_contains(sources, source1));
            ASSERT_TRUE(kdl::vec_contains(sources, source2));
        }


        TEST_CASE("AttributableNodeLinkTest.testCreateMultiTargetLink", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target1 = world.createEntity();
            Entity* target2 = world.createEntity();
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target1);
            world.defaultLayer()->addChild(target2);

            source->addOrUpdateAttribute(AttributeNames::Target + "1", "target_name1");
            source->addOrUpdateAttribute(AttributeNames::Target + "2", "target_name2");

            // here we need to query for all entities having a numbered "target" property,
            // not just those having a "target" property
            target1->addOrUpdateAttribute(AttributeNames::Targetname, "target_name1");
            target2->addOrUpdateAttribute(AttributeNames::Targetname, "target_name2");

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_EQ(2u, targets.size());
            ASSERT_TRUE(kdl::vec_contains(targets, target1));
            ASSERT_TRUE(kdl::vec_contains(targets, target2));

            const std::vector<AttributableNode*>& sources1 = target1->linkSources();
            ASSERT_EQ(1u, sources1.size());
            ASSERT_EQ(source, sources1.front());

            const std::vector<AttributableNode*>& sources2 = target2->linkSources();
            ASSERT_EQ(1u, sources2.size());
            ASSERT_EQ(source, sources2.front());
        }

        TEST_CASE("AttributableNodeLinkTest.testLoadLink", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveLinkByChangingSource", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->addOrUpdateAttribute(AttributeNames::Target, "other_name");

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveLinkByChangingTarget", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            target->addOrUpdateAttribute(AttributeNames::Targetname, "other_name");

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveLinkByRemovingSource", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(source);

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());

            delete source;
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveLinkByRemovingTarget", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(target);

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());

            delete target;
        }

        TEST_CASE("AttributableNodeLinkTest.testCreateKillLink", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("AttributableNodeLinkTest.testLoadKillLink", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveKillLinkByChangingSource", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->addOrUpdateAttribute(AttributeNames::Killtarget, "other_name");

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveKillLinkByChangingTarget", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            target->addOrUpdateAttribute(AttributeNames::Targetname, "other_name");

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveKillLinkByRemovingSource", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(source);

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());

            delete source;
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveKillLinkByRemovingTarget", "[AttributableNodeLinkTest]") {
            World world(MapFormat::Standard);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();

            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(target);

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());

            delete target;
        }
    }
}
