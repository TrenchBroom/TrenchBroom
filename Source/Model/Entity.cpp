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

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        String const Entity::ClassnameKey        = "classname";
        String const Entity::SpawnFlagsKey       = "spawnflags";
        String const Entity::WorldspawnClassname = "worldspawn";
        String const Entity::GroupClassname      = "func_group";
        String const Entity::GroupNameKey        = "__tb_group_name";
        String const Entity::GroupVisibilityKey  = "__tb_group_visible";
        String const Entity::OriginKey           = "origin";
        String const Entity::AngleKey            = "angle";
        String const Entity::MessageKey          = "message";
        String const Entity::ModsKey             = "__tb_mods";
        String const Entity::WadKey              = "wad";

        void Entity::init() {
            m_map = NULL;
            m_definition = NULL;
            m_filePosition = 0;
            setEditState(EditState::Default);
            m_selectedBrushCount = 0;
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
                    m_bounds = BBox(Vec3f(-8, -8, -8), Vec3f(8, 8, 8));
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

        Entity::Entity(const BBox& worldBounds) : MapObject(), m_worldBounds(worldBounds) {
            init();
        }

        Entity::Entity(const BBox& worldBounds, const Entity& entityTemplate) : MapObject(), m_worldBounds(worldBounds) {
            init();
            setProperties(entityTemplate.properties(), true);
        }

        Entity::~Entity() {
            setMap(NULL);
            while (!m_brushes.empty()) delete m_brushes.back(), m_brushes.pop_back();
            m_brushes.clear();
            setDefinition(NULL);
            m_geometryValid = false;
        }

        void Entity::setProperty(const PropertyKey& key, const PropertyValue& value) {
            setProperty(key, &value);
        }
        
        void Entity::setProperty(const PropertyKey& key, const PropertyValue* value) {
            if (key == ClassnameKey) {
                if (value != classname())
                    setDefinition(NULL);
            }
            
            const PropertyValue* oldValue = propertyForKey(key);
            if (oldValue != NULL && oldValue == value)
                return;
            if (value == NULL)
                m_propertyStore.removeProperty(key);
            else
                m_propertyStore.setPropertyValue(key, *value);
            invalidateGeometry();
        }
        
        void Entity::setProperty(const PropertyKey& key, const Vec3f& value, bool round) {
            StringStream valueStr;
            if (round)
                valueStr    << static_cast<int>(Math::round(value.x)) <<
                " "         << static_cast<int>(Math::round(value.y)) <<
                " "         << static_cast<int>(Math::round(value.z));
            else
                valueStr << value.x << " " << value.y << " " << value.z;
            setProperty(key, valueStr.str());
        }
        
        void Entity::setProperty(const PropertyKey& key, int value) {
            StringStream valueStr;
            valueStr << value;
            setProperty(key, valueStr.str());
        }
        
        void Entity::setProperty(const PropertyKey& key, float value, bool round) {
            StringStream valueStr;
            if (round)
                valueStr << static_cast<float>(Math::round(value));
            else
                valueStr << value;
            setProperty(key, valueStr.str());
        }
        
        void Entity::renameProperty(const PropertyKey& oldKey, const PropertyKey& newKey) {
            bool success = m_propertyStore.setPropertyKey(oldKey, newKey);
            assert(success);
        }

        void Entity::setProperties(const PropertyList& properties, bool replace) {
            if (replace)
                m_propertyStore.clear();
            PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it)
                setProperty(it->key(), it->value());
        }
        
        bool Entity::propertyKeyIsMutable(const PropertyKey& key) {
            if (key == ClassnameKey)
                return false;
            if (key == OriginKey)
                return false;
            return true;
        }

        void Entity::removeProperty(const PropertyKey& key) {
            assert(propertyKeyIsMutable(key));
            if (!m_propertyStore.containsProperty(key))
                return;
            
            if (key == ClassnameKey)
                setDefinition(NULL);

            m_propertyStore.removeProperty(key);
            invalidateGeometry();
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
            return m_definition == NULL || m_definition->type() == EntityDefinition::PointEntity;
        }

        EditState::Type Entity::setEditState(EditState::Type editState) {
            if (worldspawn())
                return EditState::Default;
            return MapObject::setEditState(editState);
        }

        void Entity::translate(const Vec3f& delta, bool lockTextures) {
            if (delta.null())
                return;
            
            Vec3f newOrigin = origin() + delta;
            setProperty(OriginKey, newOrigin, true);
            invalidateGeometry();
        }

        void Entity::rotate90(Axis::Type axis, const Vec3f& rotationCenter, bool clockwise, bool lockTextures) {
            if (!m_brushes.empty())
                return;
            
            const Vec3f offset = origin() - center();
            const Vec3f newCenter = center().rotated90(axis, rotationCenter, clockwise);
            setProperty(OriginKey, newCenter + offset, true);
            
            if (offset.x == 0.0f && offset.y == 0.0f) {
                Vec3f direction;
                float ang = static_cast<float>(angle());
                if (ang == -1.0f) {
                    direction = Vec3f::PosZ;
                } else if (ang == -2.0f) {
                    direction = Vec3f::NegZ;
                } else {
                    direction.x = std::cos(Math::radians(ang));
                    direction.y = std::sin(Math::radians(ang));
                    direction.z = 0.0f;
                }
                
                direction.rotate90(axis, clockwise);
                if (direction.z > 0.9f) {
                    setProperty(AngleKey, -1, true);
                } else if (direction.z < -0.9f) {
                    setProperty(AngleKey, -2, true);
                } else {
                    if (direction.z != 0.0f) {
                        direction.z = 0.0f;
                        direction.normalize();
                    }
                    
                    ang = Math::round(Math::degrees(std::acos(direction.x)));
                    if (direction.y < 0.0f)
                        ang = 360.0f - ang;
                    setProperty(AngleKey, ang, true);
                }
            }
            invalidateGeometry();
        }

        void Entity::rotate(const Quat& rotation, const Vec3f& rotationCenter, bool lockTextures) {
            if (!m_brushes.empty())
                return;

            const Vec3f offset = origin() - center();
            const Vec3f newCenter = rotation * (center() - rotationCenter) + rotationCenter;
            setProperty(OriginKey, newCenter + offset, true);
            
            if (offset.x == 0.0f && offset.y == 0.0f) {
                Vec3f direction;
                float ang = static_cast<float>(angle());
                if (ang == -1.0f) {
                    direction = Vec3f::PosZ;
                } else if (ang == -2.0f) {
                    direction = Vec3f::NegZ;
                } else {
                    direction.x = std::cos(Math::radians(ang));
                    direction.y = std::sin(Math::radians(ang));
                    direction.z = 0.0f;
                }
                
                direction = rotation * direction;
                if (direction.z > 0.9f) {
                    setProperty(AngleKey, -1, true);
                } else if (direction.z < -0.9f) {
                    setProperty(AngleKey, -2, true);
                } else {
                    if (direction.z != 0.0f) {
                        direction.z = 0.0f;
                        direction.normalize();
                    }
                    
                    ang = Math::round(Math::degrees(std::acos(direction.x)));
                    if (direction.y < 0.0f)
                        ang = 360.0f - ang;
                    setProperty(AngleKey, ang, true);
                }
            }
            
            invalidateGeometry();
        }

        void Entity::flip(Axis::Type axis, const Vec3f& flipCenter, bool lockTextures) {
            if (!m_brushes.empty())
                return;
            
            const Vec3f offset = origin() - center();
            const Vec3f newCenter = center().flipped(axis, flipCenter);
            setProperty(OriginKey, newCenter + offset, true);
            
            if (offset.x == 0.0f && offset.y == 0.0f) {
                float ang = static_cast<float>(angle());
                switch (axis) {
                    case Axis::AX:
                        if (ang != -1.0f && ang != -2.0f)
                            ang = 180.0f - ang;
                        break;
                    case Axis::AY:
                        if (ang != -1.0f && ang != -2.0f)
                            ang = 360.0f - ang;
                        break;
                    default:
                        if (ang == -1.0f)
                            ang = -2.0f;
                        else if (ang == -2.0f)
                            ang = -1.0f;
                        break;
                }
                
                if (ang != -1.0f && ang != -2.0f)
                    ang -= static_cast<int>(ang / 360.0f) * 360.0f;
                setProperty(AngleKey, ang, true);
            }
            invalidateGeometry();
        }

        void Entity::pick(const Ray& ray, PickResult& pickResults) {
            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math::isnan(dist))
                return;
            
            Vec3f hitPoint = ray.pointAtDistance(dist);
            EntityHit* hit = new EntityHit(*this, hitPoint, dist);
            pickResults.add(hit);
        }
    }
}
