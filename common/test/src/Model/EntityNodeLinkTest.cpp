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

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/EntityNodeBase.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"

#include <kdl/vector_utils.h>

#include <vector>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("EntityNodeLinkTest.testCreateLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity());
            EntityNode* target = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                { PropertyKeys::Target, "target_name"}
            }));

            target->setEntity(Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<EntityNodeBase*>& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("EntityNodeLinkTest.testCreateMultiSourceLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source1 = world.createEntity(Model::Entity());
            EntityNode* source2 = world.createEntity(Model::Entity());
            EntityNode* target = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source1);
            world.defaultLayer()->addChild(source2);
            world.defaultLayer()->addChild(target);

            source1->setEntity(Entity({
                { PropertyKeys::Target, "target_name"}
            }));

            source2->setEntity(Entity({
                { PropertyKeys::Target, "target_name"}
            }));

            target->setEntity(Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            const std::vector<EntityNodeBase*>& targets1 = source1->linkTargets();
            ASSERT_EQ(1u, targets1.size());
            ASSERT_EQ(target, targets1.front());

            const std::vector<EntityNodeBase*>& targets2 = source2->linkTargets();
            ASSERT_EQ(1u, targets2.size());
            ASSERT_EQ(target, targets2.front());

            const std::vector<EntityNodeBase*>& sources = target->linkSources();
            ASSERT_EQ(2u, sources.size());
            ASSERT_TRUE(kdl::vec_contains(sources, source1));
            ASSERT_TRUE(kdl::vec_contains(sources, source2));
        }


        TEST_CASE("EntityNodeLinkTest.testCreateMultiTargetLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity());
            EntityNode* target1 = world.createEntity(Model::Entity());
            EntityNode* target2 = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target1);
            world.defaultLayer()->addChild(target2);

            source->setEntity(Entity({
                { PropertyKeys::Target + "1", "target_name1"},
                { PropertyKeys::Target + "2", "target_name2"}
            }));

            // here we need to query for all entities having a numbered "target" property,
            // not just those having a "target" property
            target1->setEntity(Entity({
                { PropertyKeys::Targetname, "target_name1"}
            }));

            target2->setEntity(Entity({
                { PropertyKeys::Targetname, "target_name2"}
            }));

            const std::vector<EntityNodeBase*>& targets = source->linkTargets();
            ASSERT_EQ(2u, targets.size());
            ASSERT_TRUE(kdl::vec_contains(targets, target1));
            ASSERT_TRUE(kdl::vec_contains(targets, target2));

            const std::vector<EntityNodeBase*>& sources1 = target1->linkSources();
            ASSERT_EQ(1u, sources1.size());
            ASSERT_EQ(source, sources1.front());

            const std::vector<EntityNodeBase*>& sources2 = target2->linkSources();
            ASSERT_EQ(1u, sources2.size());
            ASSERT_EQ(source, sources2.front());
        }

        TEST_CASE("EntityNodeLinkTest.testLoadLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            const std::vector<EntityNodeBase*>& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<EntityNodeBase*>& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByChangingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                { PropertyKeys::Target, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByChangingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            target->setEntity(Entity({
                { PropertyKeys::Targetname, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByRemovingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(source);

            const std::vector<EntityNodeBase*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());

            delete source;
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByRemovingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Target, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(target);

            const std::vector<EntityNodeBase*>& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());

            delete target;
        }

        TEST_CASE("EntityNodeLinkTest.testCreateKillLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity());
            EntityNode* target = world.createEntity(Model::Entity());
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                { PropertyKeys::Killtarget, "target_name"}
            }));

            target->setEntity(Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<EntityNodeBase*>& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("EntityNodeLinkTest.testLoadKillLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            const std::vector<EntityNodeBase*>& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());

            const std::vector<EntityNodeBase*>& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByChangingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            source->setEntity(Entity({
                { PropertyKeys::Killtarget, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByChangingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            target->setEntity(Entity({
                { PropertyKeys::Targetname, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByRemovingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(source);

            const std::vector<EntityNodeBase*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());

            delete source;
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByRemovingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* source = world.createEntity(Model::Entity({
                { PropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* target = world.createEntity(Model::Entity({
                { PropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(target);

            const std::vector<EntityNodeBase*>& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());

            const std::vector<EntityNodeBase*>& sources = target->killSources();
            ASSERT_TRUE(sources.empty());

            delete target;
        }
    }
}
