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

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("EntityNodeLinkTest.testCreateLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity());
            EntityNode* targetNode = new Model::EntityNode(Model::Entity());
            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            sourceNode->setEntity(Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));

            targetNode->setEntity(Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = sourceNode->linkTargets();
            CHECK(targets.size() == 1u);
            CHECK(targets.front() == targetNode);

            const std::vector<EntityNodeBase*>& sources = targetNode->linkSources();
            CHECK(sources.size() == 1u);
            CHECK(sources.front() == sourceNode);
        }

        TEST_CASE("EntityNodeLinkTest.testCreateMultiSourceLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode1 = new Model::EntityNode(Model::Entity());
            EntityNode* sourceNode2 = new Model::EntityNode(Model::Entity());
            EntityNode* targetNode = new Model::EntityNode(Model::Entity());
            world.defaultLayer()->addChild(sourceNode1);
            world.defaultLayer()->addChild(sourceNode2);
            world.defaultLayer()->addChild(targetNode);

            sourceNode1->setEntity(Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));

            sourceNode2->setEntity(Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));

            targetNode->setEntity(Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            const std::vector<EntityNodeBase*>& targets1 = sourceNode1->linkTargets();
            CHECK(targets1.size() == 1u);
            CHECK(targets1.front() == targetNode);

            const std::vector<EntityNodeBase*>& targets2 = sourceNode2->linkTargets();
            CHECK(targets2.size() == 1u);
            CHECK(targets2.front() == targetNode);

            const std::vector<EntityNodeBase*>& sources = targetNode->linkSources();
            CHECK(sources.size() == 2u);
            CHECK(kdl::vec_contains(sources, sourceNode1));
            CHECK(kdl::vec_contains(sources, sourceNode2));
        }


        TEST_CASE("EntityNodeLinkTest.testCreateMultiTargetLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity());
            EntityNode* targetNode1 = new Model::EntityNode(Model::Entity());
            EntityNode* targetNode2 = new Model::EntityNode(Model::Entity());
            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode1);
            world.defaultLayer()->addChild(targetNode2);

            sourceNode->setEntity(Entity({
                { EntityPropertyKeys::Target + "1", "target_name1"},
                { EntityPropertyKeys::Target + "2", "target_name2"}
            }));

            // here we need to query for all entities having a numbered "target" property,
            // not just those having a "target" property
            targetNode1->setEntity(Entity({
                { EntityPropertyKeys::Targetname, "target_name1"}
            }));

            targetNode2->setEntity(Entity({
                { EntityPropertyKeys::Targetname, "target_name2"}
            }));

            const std::vector<EntityNodeBase*>& targets = sourceNode->linkTargets();
            CHECK(targets.size() == 2u);
            CHECK(kdl::vec_contains(targets, targetNode1));
            CHECK(kdl::vec_contains(targets, targetNode2));

            const std::vector<EntityNodeBase*>& sources1 = targetNode1->linkSources();
            CHECK(sources1.size() == 1u);
            CHECK(sources1.front() == sourceNode);

            const std::vector<EntityNodeBase*>& sources2 = targetNode2->linkSources();
            CHECK(sources2.size() == 1u);
            CHECK(sources2.front() == sourceNode);
        }

        TEST_CASE("EntityNodeLinkTest.testLoadLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            const std::vector<EntityNodeBase*>& targets = sourceNode->linkTargets();
            CHECK(targets.size() == 1u);
            CHECK(targets.front() == targetNode);

            const std::vector<EntityNodeBase*>& sources = targetNode->linkSources();
            CHECK(sources.size() == 1u);
            CHECK(sources.front() == sourceNode);
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByChangingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            sourceNode->setEntity(Entity({
                { EntityPropertyKeys::Target, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = sourceNode->linkTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->linkSources();
            CHECK(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByChangingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            targetNode->setEntity(Entity({
                { EntityPropertyKeys::Targetname, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = sourceNode->linkTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->linkSources();
            CHECK(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByRemovingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            world.defaultLayer()->removeChild(sourceNode);

            const std::vector<EntityNodeBase*>& targets = sourceNode->linkTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->linkSources();
            CHECK(sources.empty());

            delete sourceNode;
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveLinkByRemovingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Target, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            world.defaultLayer()->removeChild(targetNode);

            const std::vector<EntityNodeBase*>& targets = sourceNode->linkTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->linkSources();
            CHECK(sources.empty());

            delete targetNode;
        }

        TEST_CASE("EntityNodeLinkTest.testCreateKillLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity());
            EntityNode* targetNode = new Model::EntityNode(Model::Entity());
            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            sourceNode->setEntity(Entity({
                { EntityPropertyKeys::Killtarget, "target_name"}
            }));

            targetNode->setEntity(Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = sourceNode->killTargets();
            CHECK(targets.size() == 1u);
            CHECK(targets.front() == targetNode);

            const std::vector<EntityNodeBase*>& sources = targetNode->killSources();
            CHECK(sources.size() == 1u);
            CHECK(sources.front() == sourceNode);
        }

        TEST_CASE("EntityNodeLinkTest.testLoadKillLink", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            const std::vector<EntityNodeBase*>& targets = sourceNode->killTargets();
            CHECK(targets.size() == 1u);
            CHECK(targets.front() == targetNode);

            const std::vector<EntityNodeBase*>& sources = targetNode->killSources();
            CHECK(sources.size() == 1u);
            CHECK(sources.front() == sourceNode);
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByChangingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            sourceNode->setEntity(Entity({
                { EntityPropertyKeys::Killtarget, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = sourceNode->killTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->killSources();
            CHECK(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByChangingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            targetNode->setEntity(Entity({
                { EntityPropertyKeys::Targetname, "other_name"}
            }));

            const std::vector<EntityNodeBase*>& targets = sourceNode->killTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->killSources();
            CHECK(sources.empty());
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByRemovingSource", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            world.defaultLayer()->removeChild(sourceNode);

            const std::vector<EntityNodeBase*>& targets = sourceNode->killTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->killSources();
            CHECK(sources.empty());

            delete sourceNode;
        }

        TEST_CASE("EntityNodeLinkTest.testRemoveKillLinkByRemovingTarget", "[EntityNodeLinkTest]") {
            WorldNode world(Model::Entity(), MapFormat::Standard);
            EntityNode* sourceNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Killtarget, "target_name"}
            }));
            EntityNode* targetNode = new Model::EntityNode(Model::Entity({
                { EntityPropertyKeys::Targetname, "target_name"}
            }));

            world.defaultLayer()->addChild(sourceNode);
            world.defaultLayer()->addChild(targetNode);

            world.defaultLayer()->removeChild(targetNode);

            const std::vector<EntityNodeBase*>& targets = sourceNode->killTargets();
            CHECK(targets.empty());

            const std::vector<EntityNodeBase*>& sources = targetNode->killSources();
            CHECK(sources.empty());

            delete targetNode;
        }
    }
}
