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

#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"

#include <kdl/vector_utils.h>

#include <vector>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("AttributableNodeLinkTest.testCreateLink", "[AttributableNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity());
            EntityNode* target = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                {AttributeNames::Target, "target_name"}
            }));

            target->setEntity(Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("AttributableNodeLinkTest.testCreateMultiSourceLink", "[AttributableNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source1 = world.createEntity(Model::Entity());
            EntityNode* source2 = world.createEntity(Model::Entity());
            EntityNode* target = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source1);
            world.defaultLayer()->addChild(source2);
            world.defaultLayer()->addChild(target);

            source1->setEntity(Entity({
                {AttributeNames::Target, "target_name"}
            }));

            source2->setEntity(Entity({
                {AttributeNames::Target, "target_name"}
            }));

            target->setEntity(Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

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
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity());
            EntityNode* target1 = world.createEntity(Model::Entity());
            EntityNode* target2 = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target1);
            world.defaultLayer()->addChild(target2);

            source->setEntity(Entity({
                {AttributeNames::Target + "1", "target_name1"},
                {AttributeNames::Target + "2", "target_name2"}
            }));

            // here we need to query for all entities having a numbered "target" property,
            // not just those having a "target" property
            target1->setEntity(Entity({
                {AttributeNames::Targetname, "target_name1"}
            }));

            target2->setEntity(Entity({
                {AttributeNames::Targetname, "target_name2"}
            }));

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
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

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
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                {AttributeNames::Target, "other_name"}
            }));

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveLinkByChangingTarget", "[AttributableNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            target->setEntity(Entity({
                {AttributeNames::Targetname, "other_name"}
            }));

            const std::vector<AttributableNode*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveLinkByRemovingSource", "[AttributableNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

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
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

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
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity());
            EntityNode* target = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                {AttributeNames::Killtarget, "target_name"}
            }));

            target->setEntity(Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("AttributableNodeLinkTest.testLoadKillLink", "[AttributableNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

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
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                {AttributeNames::Killtarget, "other_name"}
            }));

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveKillLinkByChangingTarget", "[AttributableNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            target->setEntity(Entity({
                {AttributeNames::Targetname, "other_name"}
            }));

            const std::vector<AttributableNode*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<AttributableNode*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("AttributableNodeLinkTest.testRemoveKillLinkByRemovingSource", "[AttributableNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

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
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                {AttributeNames::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                {AttributeNames::Targetname, "target_name"}
            }));

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
