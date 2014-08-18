/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        TEST(EntityLinkTest, testCreateLink) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            map.addEntity(source);
            map.addEntity(target);
            
            source->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            const EntityList& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const EntityList& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }
        
        TEST(EntityLinkTest, testCreateMultiSourceLink) {
            Map map(MapFormat::Standard);
            Entity* source1 = map.createEntity();
            Entity* source2 = map.createEntity();
            Entity* target = map.createEntity();
            map.addEntity(source1);
            map.addEntity(source2);
            map.addEntity(target);
            
            source1->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            source2->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            const EntityList& targets1 = source1->linkTargets();
            ASSERT_EQ(1u, targets1.size());
            ASSERT_EQ(target, targets1.front());
            
            const EntityList& targets2 = source2->linkTargets();
            ASSERT_EQ(1u, targets2.size());
            ASSERT_EQ(target, targets2.front());
            
            const EntityList& sources = target->linkSources();
            ASSERT_EQ(2u, sources.size());
            ASSERT_TRUE(VectorUtils::contains(sources, source1));
            ASSERT_TRUE(VectorUtils::contains(sources, source2));
        }
        
        
        TEST(EntityLinkTest, testCreateMultiTargetLink) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target1 = map.createEntity();
            Entity* target2 = map.createEntity();
            map.addEntity(source);
            map.addEntity(target1);
            map.addEntity(target2);
            
            source->addOrUpdateProperty(PropertyKeys::Target + "1", "target_name1");
            source->addOrUpdateProperty(PropertyKeys::Target + "2", "target_name2");

            // here we need to query for all entities having a numbered "target" property,
            // not just those having a "target" property
            target1->addOrUpdateProperty(PropertyKeys::Targetname, "target_name1");
            target2->addOrUpdateProperty(PropertyKeys::Targetname, "target_name2");
            
            const EntityList& targets = source->linkTargets();
            ASSERT_EQ(2u, targets.size());
            ASSERT_TRUE(VectorUtils::contains(targets, target1));
            ASSERT_TRUE(VectorUtils::contains(targets, target2));
            
            const EntityList& sources1 = target1->linkSources();
            ASSERT_EQ(1u, sources1.size());
            ASSERT_EQ(source, sources1.front());
            
            const EntityList& sources2 = target2->linkSources();
            ASSERT_EQ(1u, sources2.size());
            ASSERT_EQ(source, sources2.front());
        }

        TEST(EntityLinkTest, testLoadLink) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);
            
            const EntityList& targets = source->linkTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const EntityList& sources = target->linkSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }

        TEST(EntityLinkTest, testRemoveLinkByChangingSource) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);
            
            source->addOrUpdateProperty(PropertyKeys::Target, "other_name");
            
            const EntityList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }
        
        TEST(EntityLinkTest, testRemoveLinkByChangingTarget) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);
            
            target->addOrUpdateProperty(PropertyKeys::Targetname, "other_name");
            
            const EntityList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
        }

        TEST(EntityLinkTest, testRemoveLinkByRemovingSource) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);

            map.removeEntity(source);
            
            const EntityList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());

            delete source;
        }
        
        TEST(EntityLinkTest, testRemoveLinkByRemovingTarget) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Target, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);

            map.removeEntity(target);
            
            const EntityList& targets = source->linkTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->linkSources();
            ASSERT_TRUE(sources.empty());
            
            delete target;
        }

        TEST(EntityLinkTest, testCreateKillLink) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            map.addEntity(source);
            map.addEntity(target);
            
            source->addOrUpdateProperty(PropertyKeys::Killtarget, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            const EntityList& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const EntityList& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }
        
        TEST(EntityLinkTest, testLoadKillLink) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Killtarget, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);

            const EntityList& targets = source->killTargets();
            ASSERT_EQ(1u, targets.size());
            ASSERT_EQ(target, targets.front());
            
            const EntityList& sources = target->killSources();
            ASSERT_EQ(1u, sources.size());
            ASSERT_EQ(source, sources.front());
        }
        
        TEST(EntityLinkTest, testRemoveKillLinkByChangingSource) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Killtarget, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);
            
            source->addOrUpdateProperty(PropertyKeys::Killtarget, "other_name");
            
            const EntityList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }
        
        TEST(EntityLinkTest, testRemoveKillLinkByChangingTarget) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Killtarget, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);
            
            target->addOrUpdateProperty(PropertyKeys::Targetname, "other_name");
            
            const EntityList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
        }
        
        TEST(EntityLinkTest, testRemoveKillLinkByRemovingSource) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Killtarget, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);
            
            map.removeEntity(source);
            
            const EntityList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
            
            delete source;
        }
        
        TEST(EntityLinkTest, testRemoveKillLinkByRemovingTarget) {
            Map map(MapFormat::Standard);
            Entity* source = map.createEntity();
            Entity* target = map.createEntity();
            
            source->addOrUpdateProperty(PropertyKeys::Killtarget, "target_name");
            target->addOrUpdateProperty(PropertyKeys::Targetname, "target_name");
            
            map.addEntity(source);
            map.addEntity(target);
            
            map.removeEntity(target);
            
            const EntityList& targets = source->killTargets();
            ASSERT_TRUE(targets.empty());
            
            const EntityList& sources = target->killSources();
            ASSERT_TRUE(sources.empty());
            
            delete target;
        }
    }
}
