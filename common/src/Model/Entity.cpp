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
#include "Hit.h"
#include "Assets/ModelDefinition.h"
#include "IO/Path.h"
#include "Model/Brush.h"
#include "Model/Layer.h"
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

        const Hit::HitType Entity::EntityHit = Hit::freeHitType();
        const String Entity::DefaultPropertyValue = "";
        const BBox3 Entity::DefaultBounds(-8.0, 8.0);
        
        
        Entity::~Entity() {
            VectorUtils::clearAndDelete(m_brushes);
        }

        Map* Entity::map() const {
            return m_map;
        }
        
        void Entity::setMap(Map* map) {
            if (map == m_map)
                return;
            
            if (m_map != NULL) {
                removeAllLinkSources();
                removeAllLinkTargets();
                removeAllKillSources();
                removeAllKillTargets();
            }
            
            m_map = map;
            
            if (m_map != NULL) {
                addAllLinkTargets();
                addAllKillTargets();
                
                const PropertyValue* targetname = m_properties.property(PropertyKeys::Targetname);
                if (targetname != NULL && !targetname->empty()) {
                    addAllLinkSources(*targetname);
                    addAllKillSources(*targetname);
                }
            }
        }

        Entity* Entity::clone(const BBox3& worldBounds) const {
            return static_cast<Entity*>(doClone(worldBounds));
        }

        EntitySnapshot Entity::takeSnapshot() {
            return EntitySnapshot(*this);
        }

        const BBox3& Entity::bounds() const {
            if (!m_boundsValid)
                validateBounds();
            return m_bounds;
        }

        void Entity::pick(const Ray3& ray, Hits& hits) {
            const BBox3& myBounds = bounds();
            if (!myBounds.contains(ray.origin)) {
                const FloatType distance = myBounds.intersectWithRay(ray);
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    hits.addHit(Hit(EntityHit, distance, hitPoint, this));
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
            m_properties.updateDefinitions(m_definition);
            if (m_definition != NULL)
                m_definition->incUsageCount();
            invalidateBounds();
        }

        bool Entity::worldspawn() const {
            return m_isWorldspawn;
        }

        bool Entity::pointEntity() const {
            if (m_definition == NULL)
                return  m_brushes.empty();
            return m_definition->type() == Assets::EntityDefinition::Type_PointEntity;
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (m_definition == NULL || !pointEntity())
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
            updatePropertyIndex(properties);
            m_properties.setProperties(properties);
            m_properties.updateDefinitions(m_definition);
            invalidateBounds();
            setIsWorldspawn();
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
            
            const Assets::PropertyDefinition* newDefinition = Assets::EntityDefinition::safeGetPropertyDefinition(m_definition, newKey);
            const PropertyValue value = *valuePtr;
            m_properties.renameProperty(key, newKey, newDefinition);
            
            updatePropertyIndex(key, value, newKey, value);
            updateLinks(key, value, newKey, value);
            invalidateBounds();
        }

        void Entity::removeProperty(const PropertyKey& key) {
            const PropertyValue* valuePtr = m_properties.property(key);
            if (valuePtr == NULL)
                return;
            const PropertyValue value = *valuePtr;
            m_properties.removeProperty(key);

            removePropertyFromIndex(key, value);
            removeLinks(key, value);
            invalidateBounds();
        }

        const PropertyValue& Entity::classname(const PropertyValue& defaultClassname) const {
            return property(PropertyKeys::Classname, defaultClassname);
        }

        Vec3 Entity::origin() const {
            const PropertyValue* value = m_properties.property(PropertyKeys::Origin);
            if (value == NULL)
                return Vec3();
            return Vec3::parse(*value);
        }

        Quat3 Entity::rotation() const {
            return Quat3();
        }

        const BrushList& Entity::brushes() const {
            return m_brushes;
        }

        void Entity::addBrushes(const BrushList& brushes) {
            BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                addBrush(*it);
        }

        void Entity::addBrush(Brush* brush) {
            assert(brush != NULL);
            assert(brush->parent() == NULL);
            
            m_brushes.push_back(brush);
            brush->setParent(this);
            if (brush->selected())
                incChildSelectionCount();
            invalidateBounds();
        }

        void Entity::removeBrush(Brush* brush) {
            assert(brush->parent() == this);
            VectorUtils::erase(m_brushes, brush);
            brush->setParent(NULL);
            if (brush->selected())
                decChildSelectionCount();
            invalidateBounds();
        }

        void Entity::removeAllBrushes() {
            BrushList::const_iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Brush* brush = *it;
                brush->setParent(NULL);
                if (brush->selected())
                    decChildSelectionCount();
            }
            m_brushes.clear();
            invalidateBounds();
        }

        Brush* Entity::findBrushByFilePosition(const size_t position) const {
            if (!containsLine(position))
                return NULL;
            BrushList::const_iterator it = Model::find(m_brushes.begin(), m_brushes.end(), MatchObjectByFilePosition(position));
            if (it == m_brushes.end())
                return NULL;
            return *it;
        }
        
        void Entity::childBrushChanged() {
            invalidateBounds();
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

        bool Entity::hasMissingSources() const {
            return (m_linkSources.empty() &&
                    m_killSources.empty() &&
                    m_properties.hasProperty(PropertyKeys::Targetname));
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

        void Entity::updatePropertyIndex(const EntityProperty::List& newProperties) {
            if (m_map == NULL)
                return;
            
            EntityProperty::List oldSorted = m_properties.properties();
            EntityProperty::List newSorted = newProperties;
            
            VectorUtils::sort(oldSorted);
            VectorUtils::sort(newSorted);
            
            size_t i = 0, j = 0;
            while (i < oldSorted.size() && j < newSorted.size()) {
                const EntityProperty& oldProp = oldSorted[i];
                const EntityProperty& newProp = newSorted[j];
                
                const int cmp = oldProp.compare(newProp);
                if (cmp < 0) {
                    removePropertyFromIndex(oldProp.key(), oldProp.value());
                    ++i;
                } else if (cmp > 0) {
                    addPropertyToIndex(newProp.key(), newProp.value());
                    ++j;
                } else {
                    updatePropertyIndex(oldProp.key(), oldProp.value(), newProp.key(), newProp.value());
                    ++i; ++j;
                }
            }
            
            while (i < oldSorted.size()) {
                const EntityProperty& oldProp = oldSorted[i];
                removePropertyFromIndex(oldProp.key(), oldProp.value());
                ++i;
            }
            
            while (j < newSorted.size()) {
                const EntityProperty& newProp = newSorted[j];
                addPropertyToIndex(newProp.key(), newProp.value());
                ++j;
            }
        }

        void Entity::addPropertyToIndex(const PropertyKey& key, const PropertyValue& value) {
            if (m_map != NULL)
                m_map->addEntityPropertyToIndex(this, key, value);
        }
        
        void Entity::removePropertyFromIndex(const PropertyKey& key, const PropertyValue& value) {
            if (m_map != NULL)
                m_map->removeEntityPropertyFromIndex(this, key, value);
        }

        void Entity::updatePropertyIndex(const PropertyKey& oldKey, const PropertyValue& oldValue, const PropertyKey& newKey, const PropertyValue& newValue) {
            removePropertyFromIndex(oldKey, oldValue);
            addPropertyToIndex(newKey, newValue);
        }

        void Entity::addLinks(const PropertyKey& key, const PropertyValue& value) {
            if (isNumberedProperty(PropertyKeys::Target, key)) {
                addLinkTargets(value);
            } else if (isNumberedProperty(PropertyKeys::Killtarget, key)) {
                addKillTargets(value);
            } else if (key == PropertyKeys::Targetname) {
                addAllLinkSources(value);
                addAllKillSources(value);
            }
        }
        
        void Entity::removeLinks(const PropertyKey& key, const PropertyValue& value) {
            if (isNumberedProperty(PropertyKeys::Target, key)) {
                removeLinkTargets(value);
            } else if (isNumberedProperty(PropertyKeys::Killtarget, key)) {
                removeKillTargets(value);
            } else if (key == PropertyKeys::Targetname) {
                removeAllLinkSources();
                removeAllKillSources();
            }
        }
        
        void Entity::updateLinks(const PropertyKey& oldKey, const PropertyValue& oldValue, const PropertyKey& newKey, const PropertyValue& newValue) {
            removeLinks(oldKey, oldValue);
            addLinks(newKey, newValue);
        }

        void Entity::addLinkTargets(const PropertyValue& targetname) {
            if (!targetname.empty() && m_map != NULL) {
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
            if (!targetname.empty() && m_map != NULL) {
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
            if (!targetname.empty()) {
                EntityList::iterator it = m_linkTargets.begin();
                while (it != m_linkTargets.end()) {
                    Entity* target = *it;
                    const PropertyValue& entityTargetname = target->property(PropertyKeys::Targetname);
                    if (entityTargetname == targetname) {
                        target->removeLinkSource(this);
                        it = m_linkTargets.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
        
        void Entity::removeKillTargets(const PropertyValue& targetname) {
            if (!targetname.empty()) {
                EntityList::iterator it = m_killTargets.begin();
                while (it != m_killTargets.end()) {
                    Entity* target = *it;
                    const PropertyValue& entityTargetname = target->property(PropertyKeys::Targetname);
                    if (entityTargetname == targetname) {
                        target->removeKillSource(this);
                        it = m_killTargets.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }

        void Entity::addAllLinkSources(const PropertyValue& targetname) {
            if (!targetname.empty() && m_map != NULL) {
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
            const EntityProperty::List properties = m_properties.numberedProperties(PropertyKeys::Target);
            EntityProperty::List::const_iterator pIt, pEnd;
            for (pIt = properties.begin(), pEnd = properties.end(); pIt != pEnd; ++pIt) {
                const EntityProperty& property = *pIt;
                const String& targetname = property.value();
                if (!targetname.empty()) {
                    const EntityList& linkTargets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                    VectorUtils::append(m_linkTargets, linkTargets);
                }
            }
            
            EntityList::const_iterator eIt, eEnd;
            for (eIt = m_linkTargets.begin(), eEnd = m_linkTargets.end(); eIt != eEnd; ++eIt) {
                Entity* linkTarget = *eIt;
                linkTarget->addLinkSource(this);
            }
        }
        
        void Entity::addAllKillSources(const PropertyValue& targetname) {
            if (!targetname.empty() && m_map != NULL) {
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
                    const String& targetname = property.value();
                    if (!targetname.empty()) {
                        const EntityList& killTargets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                        VectorUtils::append(m_killTargets, killTargets);
                    }
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
                    const String& targetname = property.value();
                    if (targetname.empty()) {
                        result.push_back(property.key());
                    } else {
                        const EntityList& linkTargets = m_map->findEntitiesWithProperty(PropertyKeys::Targetname, targetname);
                        if (linkTargets.empty())
                            result.push_back(property.key());
                    }
                }
            }
        }

        void Entity::doAddToLayer(Layer* layer) {
            layer->addEntity(this);
        }
        
        void Entity::doRemoveFromLayer(Layer* layer) {
            layer->removeEntity(this);
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

        void Entity::doAccept(ObjectVisitor& visitor) {
            visitor.visit(this);
        }

        void Entity::doAccept(ObjectQuery& query) const {
            query.query(this);
        }

        void Entity::doAcceptRecursively(ObjectVisitor& visitor) {
            visitor.visit(this);
            acceptRecursively(m_brushes.begin(), m_brushes.end(), visitor);
        }

        void Entity::doAcceptRecursively(ObjectQuery& query) const {
            query.query(this);
            acceptRecursively(m_brushes.begin(), m_brushes.end(), query);
        }
        
        void Entity::invalidateBounds() {
            m_boundsValid = false;
        }
        
        void Entity::validateBounds() const {
            const Assets::EntityDefinition* def = definition();
            if (def != NULL && def->type() == Assets::EntityDefinition::Type_PointEntity) {
                m_bounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                m_bounds.translate(origin());
            } else if (!m_brushes.empty()) {
                m_bounds = m_brushes[0]->bounds();
                for (size_t i = 1; i < m_brushes.size(); ++i)
                    m_bounds.mergeWith(m_brushes[i]->bounds());
            } else {
                m_bounds = DefaultBounds;
                m_bounds.translate(origin());
            }
            m_boundsValid = true;
        }
        
        void Entity::setIsWorldspawn() {
            m_isWorldspawn = (classname() == PropertyValues::WorldspawnClassname);
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
            VectorUtils::erase(m_linkSources, entity);
        }
        
        void Entity::removeLinkTarget(Entity* entity) {
            VectorUtils::erase(m_linkTargets, entity);
        }
        
        void Entity::removeKillSource(Entity* entity) {
            VectorUtils::erase(m_killSources, entity);
        }
        
        void Entity::removeKillTarget(Entity* entity) {
            VectorUtils::erase(m_killTargets, entity);
        }
        
        Entity::Entity() :
        m_map(NULL),
        m_definition(NULL),
        m_model(NULL),
        m_boundsValid(false),
        m_isWorldspawn(false) {}
        
        void Entity::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
            setOrigin(transformation * origin());
            
            const Mat4x4 rotation = stripTranslation(transformation);
            applyRotation(rotation);
        }

        void Entity::setOrigin(const Vec3& origin) {
            addOrUpdateProperty(PropertyKeys::Origin, origin.rounded().asString());
        }

        void Entity::applyRotation(const Mat4x4& rotation) {}
    }
}
