/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Entity.h"

#include "Model/Brush.h"
#include "Model/EntityDefinition.h"
#include "Model/Filter.h"
#include "Model/Picker.h"
#include "Model/Map.h"
#include "Utility/List.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        String const Entity::ClassnameKey        = "classname";
        String const Entity::NoClassnameValue    = "missing classname";
        String const Entity::SpawnFlagsKey       = "spawnflags";
        String const Entity::WorldspawnClassname = "worldspawn";
        String const Entity::GroupClassname      = "func_group";
        String const Entity::GroupNameKey        = "_group_name";
        String const Entity::GroupVisibilityKey  = "_group_visible";
        String const Entity::OriginKey           = "origin";
        String const Entity::AngleKey            = "angle";
        String const Entity::AnglesKey           = "angles";
        String const Entity::MangleKey           = "mangle";
        String const Entity::MessageKey          = "message";
        String const Entity::ModKey              = "_mod";
        String const Entity::TargetKey           = "target";
        String const Entity::KillTargetKey       = "killtarget";
        String const Entity::TargetnameKey       = "targetname";
        String const Entity::WadKey              = "wad";
        String const Entity::DefKey              = "_def";
        String const Entity::DefaultDefinition   = "Quake.fgd";
        String const Entity::FacePointFormatKey  = "_point_format";

        void Entity::addLinkTarget(Entity& entity) {
            m_linkTargets.push_back(&entity);
        }
        
        void Entity::removeLinkTarget(Entity& entity) {
            Utility::erase(m_linkTargets, &entity);
        }
        
        void Entity::addLinkSource(Entity& entity) {
            m_linkSources.push_back(&entity);
        }
        
        void Entity::removeLinkSource(Entity& entity) {
            Utility::erase(m_linkSources, &entity);
        }
        
        void Entity::addKillTarget(Entity& entity) {
            m_killTargets.push_back(&entity);
        }
        
        void Entity::removeKillTarget(Entity& entity) {
            Utility::erase(m_killTargets, &entity);
        }
        
        void Entity::addKillSource(Entity& entity) {
            m_killSources.push_back(&entity);
        }
        
        void Entity::removeKillSource(Entity& entity) {
            Utility::erase(m_killSources, &entity);
        }

        void Entity::addLinkTarget(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList targets = m_map->entitiesWithTargetname(targetname);
                EntityList::const_iterator it, end;
                for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                    Entity& entity = **it;
                    entity.addLinkSource(*this);
                    m_linkTargets.push_back(&entity);
                }
            }
        }
        
        void Entity::removeLinkTarget(const PropertyValue& targetname) {
            EntityList::iterator it = m_linkTargets.begin();
            while (it != m_linkTargets.end()) {
                Entity& target = **it;
                const PropertyValue* currentTargetname = target.propertyForKey(TargetnameKey);
                if (currentTargetname == NULL) { // gracefully remove this one
                    it = m_linkTargets.erase(it);
                    continue;
                }
                
                if (*currentTargetname == targetname) {
                    target.removeLinkSource(*this);
                    it = m_linkTargets.erase(it);
                    continue;
                }
		    
                ++it;
            }
        }
        
        void Entity::addKillTarget(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList targets = m_map->entitiesWithTargetname(targetname);
                EntityList::const_iterator it, end;
                for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                    Entity& entity = **it;
                    entity.addKillSource(*this);
                    m_killTargets.push_back(&entity);
                }
            }
        }
        
        void Entity::removeKillTarget(const PropertyValue& targetname) {
            EntityList::iterator it = m_killTargets.begin();
            while (it != m_killTargets.end()) {
                Entity& target = **it;
                const PropertyValue* currentTargetname = target.propertyForKey(TargetnameKey);
                if (currentTargetname == NULL) { // gracefully remove this one
                    it = m_killTargets.erase(it);
                    continue;
                }
                
                if (*currentTargetname == targetname) {
                    target.removeKillSource(*this);
                    it = m_killTargets.erase(it);
                    continue;
                }
		    
                ++it;
            }
        }
        
        void Entity::addAllLinkTargets() {
            if (m_map != NULL) {
                StringList::const_iterator nameIt, nameEnd;
                EntityList::const_iterator entityIt, entityEnd;
                
                const StringList l_linkTargetnames = linkTargetnames();
                for (nameIt = l_linkTargetnames.begin(), nameEnd = l_linkTargetnames.end(); nameIt != nameEnd; ++nameIt) {
                    const String& targetname = *nameIt;
                    const EntityList linkTargets = m_map->entitiesWithTargetname(targetname);
                    m_linkTargets.insert(m_linkTargets.end(), linkTargets.begin(), linkTargets.end());
                }
                
                for (entityIt = m_linkTargets.begin(), entityEnd = m_linkTargets.end(); entityIt != entityEnd; ++entityIt) {
                    Entity& target = **entityIt;
                    target.addLinkSource(*this);
                }
            }
        }
        
        void Entity::addAllKillTargets() {
            if (m_map != NULL) {
                StringList::const_iterator nameIt, nameEnd;
                EntityList::const_iterator entityIt, entityEnd;
                
                const StringList l_killTargetnames = killTargetnames();
                for (nameIt = l_killTargetnames.begin(), nameEnd = l_killTargetnames.end(); nameIt != nameEnd; ++nameIt) {
                    const String& targetname = *nameIt;
                    const EntityList killTargets = m_map->entitiesWithTargetname(targetname);
                    m_killTargets.insert(m_killTargets.end(), killTargets.begin(), killTargets.end());
                }
                
                for (entityIt = m_killTargets.begin(), entityEnd = m_killTargets.end(); entityIt != entityEnd; ++entityIt) {
                    Entity& target = **entityIt;
                    target.addKillSource(*this);
                }
            }
        }

        void Entity::removeAllLinkTargets() {
            EntityList::const_iterator it, end;
            for (it = m_linkTargets.begin(), end = m_linkTargets.end(); it != end; ++it) {
                Entity& entity = **it;
                entity.removeLinkSource(*this);
            }
            m_linkTargets.clear();
        }
        
        void Entity::removeAllKillTargets() {
            EntityList::const_iterator it, end;
            for (it = m_killTargets.begin(), end = m_killTargets.end(); it != end; ++it) {
                Entity& entity = **it;
                entity.removeKillSource(*this);
            }
            m_killTargets.clear();
        }

        void Entity::addAllLinkSources(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList linkSources = m_map->entitiesWithTarget(targetname);
                EntityList::const_iterator it, end;
                for (it = linkSources.begin(), end = linkSources.end(); it != end; ++it) {
                    Entity& source = **it;
                    source.addLinkTarget(*this);
                    m_linkSources.push_back(&source);
                }
            }
        }
        
        void Entity::addAllKillSources(const PropertyValue& targetname) {
            if (m_map != NULL) {
                const EntityList killSources = m_map->entitiesWithKillTarget(targetname);
                EntityList::const_iterator it, end;
                for (it = killSources.begin(), end = killSources.end(); it != end; ++it) {
                    Entity& source = **it;
                    source.addKillTarget(*this);
                    m_killSources.push_back(&source);
                }
            }
        }
        
        void Entity::removeAllLinkSources() {
            EntityList::const_iterator it, end;
            for (it = m_linkSources.begin(), end = m_linkSources.end(); it != end; ++it) {
                Entity& source = **it;
                source.removeLinkTarget(*this);
            }
            m_linkSources.clear();
        }
        
        void Entity::removeAllKillSources() {
            EntityList::const_iterator it, end;
            for (it = m_killSources.begin(), end = m_killSources.end(); it != end; ++it) {
                Entity& source = **it;
                source.removeKillTarget(*this);
            }
            m_killSources.clear();
        }

        void Entity::init() {
            m_map = NULL;
            m_worldspawn = false;
            m_definition = NULL;
            setEditState(EditState::Default);
            m_selectedBrushCount = 0;
            m_hiddenBrushCount = 0;
            setProperty(SpawnFlagsKey, "0");
            invalidateGeometry();
        }

        void Entity::validateGeometry() const {
            assert(!m_geometryValid);
            
            if (m_definition == NULL || m_definition->type() == EntityDefinition::BrushEntity) {
                if (!m_brushes.empty()) {
                    m_bounds = m_brushes[0]->bounds();
                    for (unsigned int i = 1; i < m_brushes.size(); i++)
                        m_bounds.mergeWith(m_brushes[i]->bounds());
                } else {
                    m_bounds = BBoxf(Vec3f(-8, -8, -8), Vec3f(8, 8, 8));
                    m_bounds.translate(origin());
                }
            } else {
                PointEntityDefinition* pointDefinition = static_cast<PointEntityDefinition*>(m_definition);
                m_bounds = pointDefinition->bounds();
                m_bounds.translate(origin());
            }
            
            m_center = m_bounds.center();
            m_geometryValid = true;
        }

        const Entity::RotationInfo Entity::rotationInfo() const {
            RotationType type = RTNone;
            PropertyKey property;
            
            // determine the type of rotation to apply to this entity
            const String* classn = classname();
            if (classn != NULL) {
                if (Utility::startsWith(*classn, "light")) {
                    if (propertyForKey(MangleKey) != NULL) {
                        // spotlight without a target, update mangle
                        type = RTEulerAngles;
                        property = MangleKey;
                    } else if (propertyForKey(TargetKey) == NULL) {
                        // not a spotlight, but might have a rotatable model, so change angle or angles
                        if (propertyForKey(AnglesKey) != NULL) {
                            type = RTEulerAngles;
                            property = AnglesKey;
                        } else {
                            type = RTZAngle;
                            property = AngleKey;
                        }
                    } else {
                        // spotlight with target, don't modify
                    }
                } else {
                    bool brushEntity = !m_brushes.empty() || (m_definition != NULL && m_definition->type() == EntityDefinition::BrushEntity);
                    if (brushEntity) {
                        if (propertyForKey(AnglesKey) != NULL) {
                            type = RTEulerAngles;
                            property = AnglesKey;
                        } else if (propertyForKey(AngleKey) != NULL) {
                            type = RTZAngleWithUpDown;
                            property = AngleKey;
                        }
                    } else {
                        // point entity
                        
                        // if the origin of the definition's bounding box is not in its center, don't apply the rotation
                        const Vec3f offset = origin() - center();
                        if (offset.x() == 0.0f && offset.y() == 0.0f) {
                            if (propertyForKey(AnglesKey) != NULL) {
                                type = RTEulerAngles;
                                property = AnglesKey;
                            } else {
                                type = RTZAngle;
                                property = AngleKey;
                            }
                        }
                    }
                }
            }
            
            return RotationInfo(type, property);
        }

        void Entity::applyRotation(const Mat4f& rotation) {
            const RotationInfo info = rotationInfo();
            
            switch (info.type) {
                case RTZAngle: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    float angle = angleValue != NULL ? static_cast<float>(std::atof(angleValue->c_str())) : 0.0f;
                    
                    Vec3f direction;
                    direction[0] = std::cos(Math<float>::radians(angle));
                    direction[1] = std::sin(Math<float>::radians(angle));

                    direction = rotation * direction;
                    direction[2] = 0.0f;
                    direction.normalize();
                    
                    angle = Math<float>::round(Math<float>::degrees(std::acos(direction.x())));
                    if (direction.y() < 0.0f)
                        angle = 360.0f - angle;
                    setProperty(info.property, angle, true);
                    break;
                }
                case RTZAngleWithUpDown: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    float angle = angleValue != NULL ? static_cast<float>(std::atof(angleValue->c_str())) : 0.0f;

                    Vec3f direction;
                    if (angle == -1.0f) {
                        direction = Vec3f::PosZ;
                    } else if (angle == -2.0f) {
                        direction = Vec3f::NegZ;
                    } else {
                        direction[0] = std::cos(Math<float>::radians(angle));
                        direction[1] = std::sin(Math<float>::radians(angle));
                    }

                    direction = rotation * direction;
                    direction.normalize();

                    if (direction.z() > 0.9f) {
                        setProperty(info.property, -1.0f, true);
                    } else if (direction.z() < -0.9f) {
                        setProperty(info.property, -2.0f, true);
                    } else {
                        direction[2] = 0.0f;
                        direction.normalize();
                        
                        angle = Math<float>::round(Math<float>::degrees(std::acos(direction.x())));
                        if (direction.y() < 0.0f)
                            angle = 360.0f - angle;
                        while (angle < 0.0f)
                            angle += 360.0f;
                        setProperty(info.property, angle, true);
                    }
                    break;
                }
                case RTEulerAngles: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    Vec3f angles = angleValue != NULL ? Vec3f(*angleValue) : Vec3f::Null;
                    
                    Vec3f direction = Vec3f::PosX;
                    Quatf zRotation(Math<float>::radians(angles.x()), Vec3f::PosZ);
                    Quatf yRotation(Math<float>::radians(-angles.y()), Vec3f::PosY);
                    
                    direction = zRotation * yRotation * direction;
                    direction = rotation * direction;
                    direction.normalize();
                    
                    float zAngle, xAngle;
                    
                    // FIXME: this is still buggy
                    Vec3f xyDirection = direction;
                    if (std::abs(xyDirection.z()) == 1.0f) {
                        zAngle = 0.0f;
                    } else {
                        xyDirection[2] = 0.0f;
                        xyDirection.normalize();
                        zAngle = Math<float>::round(Math<float>::degrees(std::acos(xyDirection.x())));
                        if (xyDirection.y() < 0.0f)
                            zAngle = 360.0f - zAngle;
                    }
                    
                    Vec3f xzDirection = direction;
                    if (std::abs(xzDirection.y()) == 1.0f) {
                        xAngle = 0.0f;
                    } else {
                        xzDirection[1] = 0.0f;
                        xzDirection.normalize();
                        xAngle = Math<float>::round(Math<float>::degrees(std::acos(xzDirection.x())));
                        if (xzDirection.z() < 0.0f)
                            xAngle = 360.0f - xAngle;
                    }
                    
                    angles = Vec3f(zAngle, xAngle, 0.0f);
                    setProperty(info.property, angles, true);
                    break;
                }
                default:
                    break;
            }
        }

        Entity::Entity(const BBoxf& worldBounds) : MapObject(), m_worldBounds(worldBounds) {
            init();
        }

        Entity::Entity(const BBoxf& worldBounds, const Entity& entityTemplate) : MapObject(), m_worldBounds(worldBounds) {
            init();
            setProperties(entityTemplate.properties(), true);
        }

        Entity::~Entity() {
            setMap(NULL);
            Utility::deleteAll(m_brushes);
            setDefinition(NULL);
            m_geometryValid = false;
        }

        void Entity::setMap(Map* map) {
            if (m_map == map)
                return;
            
            removeAllLinkTargets();
            removeAllKillTargets();
            removeAllLinkSources();
            removeAllKillSources();
            
            m_map = map;
            
            addAllLinkTargets();
            addAllKillTargets();

            const PropertyValue* targetname = propertyForKey(TargetnameKey);
            if (targetname != NULL && !targetname->empty()) {
                addAllLinkSources(*targetname);
                addAllKillSources(*targetname);
            }
        }

        bool Entity::propertyIsMutable(const PropertyKey& key) {
            if (key == ModKey)
                return false;
            if (key == DefKey)
                return false;
            if (key == WadKey)
                return false;
            if (key == FacePointFormatKey)
                return false;
            return true;
        }
        
        bool Entity::propertyKeyIsMutable(const PropertyKey& key) {
            if (key == ClassnameKey)
                return false;
            if (key == OriginKey)
                return false;
            if (key == SpawnFlagsKey)
                return false;
            if (key == ModKey)
                return false;
            if (key == DefKey)
                return false;
            if (key == WadKey)
                return false;
            if (key == FacePointFormatKey)
                return false;
            return true;
        }

        void Entity::renameProperty(const PropertyKey& oldKey, const PropertyKey& newKey) {
            const PropertyValue* value = propertyForKey(oldKey);
            removeProperty(oldKey);
            setProperty(newKey, *value);
        }
        
        void Entity::removeProperty(const PropertyKey& key) {
            assert(propertyKeyIsMutable(key));
            if (!m_propertyStore.containsProperty(key))
                return;
            
            setProperty(key, static_cast<const PropertyValue*>(NULL));
        }
        
        void Entity::setProperties(const PropertyList& properties, bool replace) {
            if (replace) {
                m_propertyStore.clear();
                setProperty(SpawnFlagsKey, "0");
            }
            PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it)
                setProperty(it->key(), it->value());
        }
        
        void Entity::setProperty(const PropertyKey& key, const Vec3f& value, bool round) {
            StringStream valueStr;
            if (round)
                setProperty(key, value.rounded().asString());
            else
                setProperty(key, value.asString());
        }
        
        void Entity::setProperty(const PropertyKey& key, int value) {
            StringStream valueStr;
            valueStr << value;
            setProperty(key, valueStr.str());
        }
        
        void Entity::setProperty(const PropertyKey& key, float value, bool round) {
            StringStream valueStr;
            if (round)
                valueStr << static_cast<float>(Math<float>::round(value));
            else
                valueStr << value;
            setProperty(key, valueStr.str());
        }
        
        void Entity::setProperty(const PropertyKey& key, const PropertyValue& value) {
            setProperty(key, &value);
        }
        
        void Entity::setProperty(const PropertyKey& key, const PropertyValue* value) {
            const PropertyValue* oldValue = propertyForKey(key);
            if (oldValue == value)
                return;
            if (oldValue != NULL && value != NULL && *oldValue == *value)
                return;
            
            if (key == ClassnameKey && value != classname()) {
                m_worldspawn = *value == WorldspawnClassname;
                setDefinition(NULL);
            }
            
            if (isNumberedProperty(TargetKey, key)) {
                if (oldValue != NULL && !oldValue->empty())
                    removeLinkTarget(*oldValue);
                if (value != NULL && !value->empty())
                    addLinkTarget(*value);
                if (m_map != NULL)
                    m_map->updateEntityTarget(*this, value, oldValue);
            } else if (isNumberedProperty(KillTargetKey, key)) {
                if (oldValue != NULL && !oldValue->empty())
                    removeKillTarget(*oldValue);
                if (value != NULL && !value->empty())
                    addKillTarget(*value);
                if (m_map != NULL)
                    m_map->updateEntityKillTarget(*this, value, oldValue);
            } else if (key == TargetnameKey) {
                removeAllLinkSources();
                removeAllKillSources();
                if (value != NULL && !value->empty()) {
                    addAllLinkSources(*value);
                    addAllKillSources(*value);
                }
                if (m_map != NULL)
                    m_map->updateEntityTargetname(*this, value, oldValue);
            }
            
            if (value == NULL)
                m_propertyStore.removeProperty(key);
            else
                m_propertyStore.setPropertyValue(key, *value);
            invalidateGeometry();
        }
        
        StringList Entity::linkTargetnames() const {
            StringList targetnames;

            const PropertyList& properties = m_propertyStore.properties();
            PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const Property& property = *it;
                if (isNumberedProperty(TargetKey, property.key()))
                    targetnames.push_back(property.value());
            }
            return targetnames;
        }
        
        StringList Entity::killTargetnames() const {
            StringList targetnames;
            
            const PropertyList& properties = m_propertyStore.properties();
            PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const Property& property = *it;
                if (isNumberedProperty(KillTargetKey, property.key()))
                    targetnames.push_back(property.value());
            }
            return targetnames;
        }

        const Quatf Entity::rotation() const {
            const RotationInfo info = rotationInfo();
            switch (info.type) {
                case RTZAngle: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    if (angleValue == NULL)
                        return Quatf(0.0f, Vec3f::PosZ);
                    float angle = static_cast<float>(std::atof(angleValue->c_str()));
                    return Quatf(Math<float>::radians(angle), Vec3f::PosZ);
                }
                case RTZAngleWithUpDown: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    if (angleValue == NULL)
                        return Quatf(0.0f, Vec3f::PosZ);
                    float angle = static_cast<float>(std::atof(angleValue->c_str()));
                    if (angle == -1.0f)
                        return Quatf(-Math<float>::Pi / 2.0f, Vec3f::PosY);
                    if (angle == -2.0f)
                        return Quatf(Math<float>::Pi / 2.0f, Vec3f::PosY);
                    return Quatf(Math<float>::radians(angle), Vec3f::PosZ);
                }
                case RTEulerAngles: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    Vec3f angles = angleValue != NULL ? Vec3f(*angleValue) : Vec3f::Null;
                    
                    Quatf zRotation(Math<float>::radians( angles.x()), Vec3f::PosZ);
                    Quatf yRotation(Math<float>::radians(-angles.y()), Vec3f::PosY);
                    return zRotation * yRotation;
                }
                default:
                    return Quatf(0.0f, Vec3f::PosZ);
            }
        }

        void Entity::addBrush(Brush& brush) {
            brush.setEntity(this);
            m_brushes.push_back(&brush);
            invalidateGeometry();
        }
        
        void Entity::addBrushes(const BrushList& brushes) {
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                brush->setEntity(this);
                m_brushes.push_back(brush);
            }
            invalidateGeometry();
        }
        
        void Entity::removeBrush(Brush& brush) {
            brush.setEntity(NULL);
            m_brushes.erase(std::remove(m_brushes.begin(), m_brushes.end(), &brush), m_brushes.end());
            invalidateGeometry();
        }

        void Entity::setDefinition(EntityDefinition* definition) {
            if (m_definition != NULL)
                m_definition->decUsageCount();
            m_definition = definition;
            if (m_definition != NULL)
                m_definition->incUsageCount();
            invalidateGeometry();
        }

        bool Entity::selectable() const {
            return m_brushes.empty();
        }

        EditState::Type Entity::setEditState(EditState::Type editState) {
            if (worldspawn())
                return EditState::Default;
            return MapObject::setEditState(editState);
        }

        void Entity::transform(const Mat4f& pointTransform, const Mat4f& vectorTransform, const bool lockTextures, const bool invertOrientation) {
            Vec3f newOrigin = pointTransform * origin();
            setProperty(OriginKey, newOrigin, true);
            applyRotation(vectorTransform);
            invalidateGeometry();
            
        }

        void Entity::pick(const Rayf& ray, PickResult& pickResults) {
            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math<float>::isnan(dist))
                return;
            
            Vec3f hitPoint = ray.pointAtDistance(dist);
            EntityHit* hit = new EntityHit(*this, hitPoint, dist);
            pickResults.add(hit);
        }
    }
}
