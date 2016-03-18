/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/Layer.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        TEST(AttributableNodeLinkTest, testCreateLink) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            const AttributableNodeList& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const AttributableNodeList& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }
        
        TEST(AttributableNodeLinkTest, testCreateMultiSourceLink) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source1 = world.createEntity();
            Entity* source2 = world.createEntity();
            Entity* target = world.createEntity();
            world.defaultLayer()->addChild(source1);
            world.defaultLayer()->addChild(source2);
            world.defaultLayer()->addChild(target);
            
            source1->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            source2->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            const AttributableNodeList& targets1 = source1->linkTargets();
            ASSERT_EQ(1u, targets1.size());
            ASSERT_EQ(target, targets1.front());
            
            const AttributableNodeList& targets2 = source2->linkTargets();
            ASSERT_EQ(1u, targets2.size());
            ASSERT_EQ(target, targets2.front());
            
            const AttributableNodeList& sources = target->linkSources();
            ASSERT_EQ(2u, sources.size());
            ASSERT_TRUE(VectorUtils::contains(sources, source1));
            ASSERT_TRUE(VectorUtils::contains(sources, source2));
        }
        
        
        TEST(AttributableNodeLinkTest, testCreateMultiTargetLink) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
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
            
            const AttributableNodeList& targets = source->linkTargets();
            ASSERT_EQ(2u, targets.size());
            ASSERT_TRUE(VectorUtils::contains(targets, target1));
            ASSERT_TRUE(VectorUtils::contains(targets, target2));
            
            const AttributableNodeList& sources1 = target1->linkSources();
            ASSERT_EQ(1u, sources1.size());
            ASSERT_EQ(source, sources1.front());
            
            const AttributableNodeList& sources2 = target2->linkSources();
            ASSERT_EQ(1u, sources2.size());
            ASSERT_EQ(source, sources2.front());
        }

        TEST(AttributableNodeLinkTest, testLoadLink) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            const AttributableNodeList& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const AttributableNodeList& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST(AttributableNodeLinkTest, testRemoveLinkByChangingSource) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            source->addOrUpdateAttribute(AttributeNames::Target, "other_name");
            
            const AttributableNodeList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }
        
        TEST(AttributableNodeLinkTest, testRemoveLinkByChangingTarget) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            target->addOrUpdateAttribute(AttributeNames::Targetname, "other_name");
            
            const AttributableNodeList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST(AttributableNodeLinkTest, testRemoveLinkByRemovingSource) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(source);
            
            const AttributableNodeList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());

            delete source;
        }
        
        TEST(AttributableNodeLinkTest, testRemoveLinkByRemovingTarget) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Target, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            world.defaultLayer()->removeChild(target);
            
            const AttributableNodeList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
            
            delete target;
        }

        TEST(AttributableNodeLinkTest, testCreateKillLink) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            const AttributableNodeList& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const AttributableNodeList& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }
        
        TEST(AttributableNodeLinkTest, testLoadKillLink) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);

            const AttributableNodeList& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const AttributableNodeList& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }
        
        TEST(AttributableNodeLinkTest, testRemoveKillLinkByChangingSource) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            source->addOrUpdateAttribute(AttributeNames::Killtarget, "other_name");
            
            const AttributableNodeList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }
        
        TEST(AttributableNodeLinkTest, testRemoveKillLinkByChangingTarget) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            target->addOrUpdateAttribute(AttributeNames::Targetname, "other_name");
            
            const AttributableNodeList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }
        
        TEST(AttributableNodeLinkTest, testRemoveKillLinkByRemovingSource) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            world.defaultLayer()->removeChild(source);
            
            const AttributableNodeList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
            
            delete source;
        }
        
        TEST(AttributableNodeLinkTest, testRemoveKillLinkByRemovingTarget) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            Entity* source = world.createEntity();
            Entity* target = world.createEntity();
            
            source->addOrUpdateAttribute(AttributeNames::Killtarget, "target_name");
            target->addOrUpdateAttribute(AttributeNames::Targetname, "target_name");
            
            world.defaultLayer()->addChild(source);
            world.defaultLayer()->addChild(target);
            
            world.defaultLayer()->removeChild(target);
            
            const AttributableNodeList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const AttributableNodeList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
            
            delete target;
        }
    }
}
