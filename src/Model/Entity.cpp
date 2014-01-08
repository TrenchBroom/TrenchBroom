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

#include "Entity.h"

#include "CollectionUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/ModelDefinition.h"
#include "IO/Path.h"
#include "Model/Brush.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"

namespace TrenchBroom {
    namespace Model {
        EntitySnapshot::EntitySnapshot(Entity& entity) :
        m_entity(&entity),
        m_properties(m_entity->properties()) {}

        void EntitySnapshot::restore() {
            m_entity->setProperties(m_properties);
        }

        const String Entity::DefaultPropertyValue = "";
        const Hit::HitType Entity::EntityHit = Hit::freeHitType();
        
        Entity::~Entity() {
            VectorUtils::clearAndDelete(m_brushes);
        }

        Map* Entity::map() const {
            return m_map;
        }
        
        void Entity::setMap(Map* map) {
            if (map == m_map)
                return;
            
            removeAllLinkSources();
            removeAllLinkTargets();
            removeAllKillSources();
            removeAllKillTargets();
            
            m_map = map;
            
            addAllLinkTargets();
            addAllKillTargets();
            
            const PropertyValue* targetname = m_properties.property(PropertyKeys::Targetname);
            if (targetname != NULL && !targetname->empty()) {
                addAllLinkSources(*targetname);
                addAllKillSources(*targetname);
            }
        }

        Entity* Entity::clone(const BBox3& worldBounds) const {
            return static_cast<Entity*>(doClone(worldBounds));
        }

        EntitySnapshot Entity::takeSnapshot() {
            return EntitySnapshot(*this);
        }

        BBox3 Entity::bounds() const {
            const Assets::EntityDefinition* def = definition();
            if (def != NULL && def->type() == Assets::EntityDefinition::PointEntity) {
                BBox3 bounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                bounds.translate(origin());
                return bounds;
            }
            if (m_brushes.empty())
                return BBox3(-8.0, +8.0);
            BBox3 bounds = m_brushes[0]->bounds();
            for (size_t i = 1; i < m_brushes.size(); ++i)
                bounds.mergeWith(m_brushes[i]->bounds());
            return bounds;
        }

        void Entity::pick(const Ray3& ray, PickResult& result) {
            const BBox3 myBounds = bounds();
            if (!myBounds.contains(ray.origin)) {
                const FloatType distance = myBounds.intersectWithRay(ray);
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    Hit hit(EntityHit, distance, hitPoint, this);
                    result.addHit(hit);
                }
            }
        }

        Assets::EntityDefinition* Entity::definition() const {
            return m_definition;
        }
        
        void Entity::setDefinition(Assets::EntityDefinition* definition) {
            if (m_definition == definition)
                return;
            if (m_definition != NULL)
                m_definition->decUsageCount();
            m_definition = definition;
            if (m_definition != NULL)
                m_definition->incUsageCount();
        }

        bool Entity::worldspawn() const {
            return classname() == PropertyValues::WorldspawnClassname;
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (m_definition == NULL || m_definition->type() != Assets::EntityDefinition::PointEntity)
                return Assets::ModelSpecification();
            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(m_definition);
            return pointDefinition->model(m_properties);
        }

        Assets::EntityModel* Entity::model() const {
            return m_model;
        }
        
        void Entity::setModel(Assets::EntityModel* model) {
            m_model = model;
        }
        
        const EntityProperty::List& Entity::properties() const {
            return m_properties.properties();
        }
        
        void Entity::setProperties(const EntityProperty::List& properties) {
            m_properties.setProperties(properties);
        }

        bool Entity::hasProperty(const PropertyKey& key) const {
            return m_properties.hasProperty(key);
        }

        const PropertyValue& Entity::property(const PropertyKey& key, const PropertyValue& defaultValue) const {
            const PropertyValue* value = m_properties.property(key);
            if (value == NULL)
                return defaultValue;
            return *value;
        }
        
        void Entity::renameProperty(const PropertyKey& key, const PropertyKey& newKey) {
            if (key == newKey)
                return;
            
            const PropertyValue* valuePtr = m_properties.property(key);
            if (valuePtr == NULL)
                return;
            
            const PropertyValue value = *valuePtr;
            m_properties.renameProperty(key, newKey);
            
            const EntityProperty oldProperty(key, value);
            const EntityProperty newProperty(newKey, value);
            updatePropertyIndex(oldProperty, newProperty);
            updateLinks(oldProperty, newProperty);
        }

        void Entity::removeProperty(const PropertyKey& key) {
            const PropertyValue* valuePtr = m_properties.property(key);
            if (valuePtr == NULL)
                return;
            const PropertyValue value = *valuePtr;
            m_properties.removeProperty(key);
            
            const EntityProperty property(key, value);
            removePropertyFromIndex(property);
            removeLinks(property);
        }

        const PropertyValue& Entity::classname(const PropertyValue& defaultClassname) const {
            return property(PropertyKeys::Classname, defaultClassname);
        }

        Vec3 Entity::origin() const {
            const PropertyValue* value = m_properties.property(PropertyKeys::Origin);
            if (value == NULL)
                return Vec3();
            return Vec3(*value);
        }

        Quat3 Entity::rotation() const {
            return Quat3();
        }

        const BrushList& Entity::brushes() const {
            return m_brushes;
        }

        void Entity::addBrush(Brush* brush) {
            assert(brush->parent() == NULL);
            m_brushes.push_back(brush);
            brush->setParent(this);
        }

        void Entity::removeBrush(Brush* brush) {
            assert(brush->parent() == this);
            VectorUtils::remove(m_brushes, brush);
            brush->setParent(NULL);
        }

        Brush* Entity::findBrushByFilePosition(const size_t position) const {
            if (!containsLine(position))
                return NULL;
            BrushList::const_iterator it = Model::find(m_brushes.begin(), m_brushes.end(), MatchObjectByFilePosition(position));
            if (it == m_brushes.end())
                return NULL;
            return *it;
        }

        const EntityList& Entity::linkSources() const {
            return m_linkSources;
        }
        
        const EntityList& Entity::linkTargets() const {
            return m_linkTargets;
        }
        
        const EntityList& Entity::killSources() const {
            return m_killSources;
        }
        
        const EntityList& Entity::killTargets() const {
            return m_killTargets;
        }

        PropertyKeyList Entity::findMissingLinkTargets() const {
            PropertyKeyList result;
            findMissingTargets(PropertyKeys::Target, result);
            return result;
        }
        
        PropertyKeyList Entity::findMissingKillTargets() const {
            PropertyKeyList result;
            findMissingTargets(PropertyKeys::Killtarget, result);
            return result;
        }

        void Entity::addPropertyToIndex(const EntityProperty& property) {
            if (m_map != NULL)
                m_map->addEntityPropertyToIndex(this, property);
        }
        
        void Entity::removePropertyFromIndex(const EntityProperty& property) {
            if (m_map != NULL)
                m_map->removeEntityPropertyFromIndex(this, property);
        }

        void Entity::updatePropertyIndex(const EntityProperty& oldProperty, const EntityProperty& newProperty) {
            removePropertyFromIndex(oldProperty);
            addPropertyToIndex(newProperty);
        }

        void Entity::addLinks(const EntityProperty& property) {
            if (isNumberedProperty(PropertyKeys::Target, property.key)) {
                addLinkTargets(property.value);
            } else if (isNumberedProperty(PropertyKeys::Killtarget, property.key)) {
                addKillTargets(property.value);
            } else if (property.key == PropertyKeys::Targetname) {
                addAllLinkSources(property.value);
                addAllKillSources(property.value);
            }
        }
        
        void Entity::removeLinks(const EntityProperty& property) {
            if (isNumberedProperty(PropertyKeys::Target, property.key)) {
                removeLinkTargets(property.value);
            } else if (isNumberedProperty(PropertyKeys::Killtarget, property.key)) {
                removeKillTargets(property.value);
            } else if (property.key == PropertyKeys::Targetname) {
                removeAllLinkSources();
                removeAllKillSources();
            }
        }
        
        void Entity::updateLinks(const EntityProperty& oldProperty, const EntityProperty& newProperty) {
            removeLinks(oldProperty);
            addLinks(newProperty);
        }

        void Entity::addLinkTargets(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList& targets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                EntityList::const_iterator it, end;
                for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                    Entity* target = *it;
                    target->addLinkSource(this);
                    m_linkTargets.push_back(target);
                }
            }
        }
        
        void Entity::addKillTargets(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList& targets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                EntityList::const_iterator it, end;
                for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                    Entity* target = *it;
                    target->addKillSource(this);
                    m_killTargets.push_back(target);
                }
            }
        }

        void Entity::removeLinkTargets(const PropertyValue& targetname) {
            EntityList::iterator it = m_linkTargets.begin();
            while (it != m_linkTargets.end()) {
                Entity* target = *it;
                const PropertyValue& entityTargetname = target->property(PropertyKeys::Targetname);
                if (entityTargetname == targetname) {
                    target->removeLinkSource(this);
                    it = m_linkTargets.erase(it);
                }
            }
        }
        
        void Entity::removeKillTargets(const PropertyValue& targetname) {
            EntityList::iterator it = m_killTargets.begin();
            while (it != m_killTargets.end()) {
                Entity* target = *it;
                const PropertyValue& entityTargetname = target->property(PropertyKeys::Targetname);
                if (entityTargetname == targetname) {
                    target->removeKillSource(this);
                    it = m_killTargets.erase(it);
                }
            }
        }

        void Entity::addAllLinkSources(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList& linkSources = m_map->findEntitiesWithNumberedProperty(PropertyKeys::Target, targetname);
                EntityList::const_iterator it, end;
                for (it = linkSources.begin(), end = linkSources.end(); it != end; ++it) {
                    Entity* linkSource = *it;
                    linkSource->addLinkTarget(this);
                    m_linkSources.push_back(linkSource);
                }
            }
        }
        
        void Entity::addAllLinkTargets() {
            if (m_map != NULL) {
                const EntityProperty::List properties = m_properties.numberedProperties(PropertyKeys::Target);
                EntityProperty::List::const_iterator pIt, pEnd;
                for (pIt = properties.begin(), pEnd = properties.end(); pIt != pEnd; ++pIt) {
                    const EntityProperty& property = *pIt;
                    const String& targetname = property.value;
                    const EntityList& linkTargets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                    VectorUtils::append(m_linkTargets, linkTargets);
                }
                
                EntityList::const_iterator eIt, eEnd;
                for (eIt = m_linkTargets.begin(), eEnd = m_linkTargets.end(); eIt != eEnd; ++eIt) {
                    Entity* linkTarget = *eIt;
                    linkTarget->addLinkSource(this);
                }
            }
        }
        
        void Entity::addAllKillSources(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList& killSources = m_map->findEntitiesWithNumberedProperty(PropertyKeys::Killtarget, targetname);
                EntityList::const_iterator it, end;
                for (it = killSources.begin(), end = killSources.end(); it != end; ++it) {
                    Entity* killSource = *it;
                    killSource->addKillTarget(this);
                    m_killSources.push_back(killSource);
                }
            }
        }
        
        void Entity::addAllKillTargets() {
            if (m_map != NULL) {
                const EntityProperty::List properties = m_properties.numberedProperties(PropertyKeys::Killtarget);
                EntityProperty::List::const_iterator pIt, pEnd;
                for (pIt = properties.begin(), pEnd = properties.end(); pIt != pEnd; ++pIt) {
                    const EntityProperty& property = *pIt;
                    const String& targetname = property.value;
                    const EntityList& killTargets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                    VectorUtils::append(m_killTargets, killTargets);
                }
                
                EntityList::const_iterator eIt, eEnd;
                for (eIt = m_killTargets.begin(), eEnd = m_killTargets.end(); eIt != eEnd; ++eIt) {
                    Entity* killTarget = *eIt;
                    killTarget->addLinkSource(this);
                }
            }
        }
        
        void Entity::removeAllLinkSources() {
            EntityList::const_iterator it, end;
            for (it = m_linkSources.begin(), end = m_linkSources.end(); it != end; ++it) {
                Entity* entity = *it;
                entity->removeLinkTarget(this);
            }
            m_linkSources.clear();
        }
        
        void Entity::removeAllLinkTargets() {
            EntityList::const_iterator it, end;
            for (it = m_linkTargets.begin(), end = m_linkTargets.end(); it != end; ++it) {
                Entity* entity = *it;
                entity->removeLinkSource(this);
            }
            m_linkTargets.clear();
        }
        
        void Entity::removeAllKillSources() {
            EntityList::const_iterator it, end;
            for (it = m_killSources.begin(), end = m_killSources.end(); it != end; ++it) {
                Entity* entity = *it;
                entity->removeKillTarget(this);
            }
            m_killSources.clear();
        }
        
        void Entity::removeAllKillTargets() {
            EntityList::const_iterator it, end;
            for (it = m_killTargets.begin(), end = m_killTargets.end(); it != end; ++it) {
                Entity* entity = *it;
                entity->removeKillSource(this);
            }
            m_killTargets.clear();
        }
        
        void Entity::findMissingTargets(const PropertyKey& prefix, PropertyKeyList& result) const {
            if (m_map != NULL) {
                const EntityProperty::List properties = m_properties.numberedProperties(prefix);
                EntityProperty::List::const_iterator pIt, pEnd;
                for (pIt = properties.begin(), pEnd = properties.end(); pIt != pEnd; ++pIt) {
                    const EntityProperty& property = *pIt;
                    const String& targetname = property.value;
                    const EntityList& linkTargets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                    if (linkTargets.empty())
                        result.push_back(property.key);
                }
            }
        }

        bool Entity::doContains(const Object& object) const {
            return object.containedBy(*this);
        }
        
        bool Entity::doContains(const Entity& entity) const {
            return bounds().contains(entity.bounds());
        }
        
        bool Entity::doContains(const Brush& brush) const {
            return brush.containedBy(*this);
        }

        bool Entity::doContainedBy(const Object& object) const {
            return object.contains(*this);
        }
        
        bool Entity::doContainedBy(const Entity& entity) const {
            return entity.contains(*this);
        }
        
        bool Entity::doContainedBy(const Brush& brush) const {
            return brush.contains(*this);
        }

        bool Entity::doIntersects(const Object& object) const {
            return object.intersects(*this);
        }
        
        bool Entity::doIntersects(const Entity& entity) const {
            return bounds().intersects(entity.bounds());
        }
        
        bool Entity::doIntersects(const Brush& brush) const {
            return brush.intersects(*this);
        }

        void Entity::doVisit(ObjectVisitor& visitor) {
            visitor.visit(this);
        }

        void Entity::addLinkSource(Entity* entity) {
            m_linkSources.push_back(entity);
        }
        
        void Entity::addLinkTarget(Entity* entity) {
            m_linkTargets.push_back(entity);
        }
        
        void Entity::addKillSource(Entity* entity) {
            m_killSources.push_back(entity);
        }
        
        void Entity::addKillTarget(Entity* entity) {
            m_killTargets.push_back(entity);
        }
        
        void Entity::removeLinkSource(Entity* entity) {
            VectorUtils::remove(m_linkSources, entity);
        }
        
        void Entity::removeLinkTarget(Entity* entity) {
            VectorUtils::remove(m_linkTargets, entity);
        }
        
        void Entity::removeKillSource(Entity* entity) {
            VectorUtils::remove(m_killSources, entity);
        }
        
        void Entity::removeKillTarget(Entity* entity) {
            VectorUtils::remove(m_killTargets, entity);
        }
        
        Entity::Entity() :
        Object(OTEntity),
        m_map(NULL),
        m_definition(NULL),
        m_model(NULL) {}
        
        void Entity::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
            const Mat4x4 translation = translationMatrix(transformation);
            const Vec3 orig = origin();
            setOrigin(translation * orig);
            
            const Mat4x4 rotation = stripTranslation(transformation);
            applyRotation(rotation);
        }

        void Entity::setOrigin(const Vec3& origin) {
            addOrUpdateProperty(PropertyKeys::Origin, origin.rounded().asString());
        }

        void Entity::applyRotation(const Mat4x4& rotation) {}
    }
}
