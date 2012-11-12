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
            m_origin = Vec3f::Null;
            m_angle = 0.0f;
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
                    m_bounds.translate(m_origin);
                }
            } else {
                PointEntityDefinition* pointDefinition = static_cast<PointEntityDefinition*>(m_definition);
                m_bounds = pointDefinition->bounds();
                m_bounds.translate(m_origin);
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
            } else if (key == OriginKey) {
                if (value == NULL)
                    m_origin = Vec3f(0, 0, 0);
                else
                    m_origin = Vec3f(*value);
            } else if (key == AngleKey) {
                if (value != NULL)
                    m_angle = static_cast<float>(atof(value->c_str()));
                else
                    m_angle = Math::nan();
            }
            
            const PropertyValue* oldValue = propertyForKey(key);
            if (oldValue != NULL && oldValue == value)
                return;
            m_properties[key] = *value;
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
        
        void Entity::setProperties(const Properties& properties, bool replace) {
            if (replace)
                m_properties.clear();
            Properties::const_iterator it;
            for (it = properties.begin(); it != properties.end(); ++it)
                setProperty(it->first, it->second);
        }
        
        void Entity::deleteProperty(const PropertyKey& key) {
            if (m_properties.count(key) == 0)
                return;
            
            if (key == ClassnameKey)
                setDefinition(NULL);
            else if (key == OriginKey)
                m_origin = Vec3f();
            else if (key == AngleKey)
                m_angle = Math::nan();
            
            m_properties.erase(key);
            invalidateGeometry();
        }
        
        void Entity::addBrush(Brush& brush) {
            if (m_definition != NULL && m_definition->type() == EntityDefinition::PointEntity)
                return;
            
            brush.setEntity(this);
            m_brushes.push_back(&brush);
            invalidateGeometry();
        }
        
        void Entity::addBrushes(const BrushList& brushes) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::PointEntity)
                return;
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                brush->setEntity(this);
                m_brushes.push_back(brush);
            }
            invalidateGeometry();
        }
        
        void Entity::removeBrush(Brush& brush) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::PointEntity)
                return;
            
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

        void Entity::rotate90(Axis::Type axis, const Vec3f& center, bool clockwise, bool lockTextures) {
            if (m_definition != NULL && m_definition->type() == EntityDefinition::BrushEntity)
                return;
            
            setProperty(OriginKey, m_origin.rotated90(axis, center, clockwise), true);
            
            Vec3f direction;
            if (m_angle >= 0) {
                direction.x = cos(2.0f * Math::Pi - Math::radians(m_angle));
                direction.y = sin(2.0f * Math::Pi - Math::radians(m_angle));
                direction.z = 0.0f;
            } else if (m_angle == -1) {
                direction = Vec3f::PosZ;
            } else if (m_angle == -2) {
                direction = Vec3f::NegZ;
            } else {
                return;
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
                
                m_angle = Math::round(Math::degrees(acos(direction.x)));
                Vec3f cross = direction.crossed(Vec3f::PosX);
                if (!cross.null() && cross.z < 0.0f)
                    m_angle = 360 - m_angle;
                setProperty(AngleKey, m_angle, true);
            }
            invalidateGeometry();
        }

        void Entity::rotate(const Quat& rotation, const Vec3f& rotationCenter, bool lockTextures) {
            if (m_definition != NULL && m_definition->type() == EntityDefinition::BrushEntity)
                return;
            
            Vec3f offset = center() - origin();
            Vec3f newCenter = rotation * (center() - rotationCenter) + rotationCenter;
            setProperty(OriginKey, newCenter - offset, true);
            
            Vec3f direction;
            if (m_angle >= 0.0f) {
                direction.x = cos(2.0f * Math::Pi - Math::radians(m_angle));
                direction.y = sin(2.0f * Math::Pi - Math::radians(m_angle));
                direction.z = 0.0f;
            } else if (m_angle == -1.0f) {
                direction = Vec3f::PosZ;
            } else if (m_angle == -2.0f) {
                direction = Vec3f::NegZ;
            } else {
                return;
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
                
                m_angle = Math::round(Math::degrees(acos(direction.x)));
                Vec3f cross = direction.crossed(Vec3f::PosX);
                if (!cross.null() && cross.z < 0.0f)
                    m_angle = 360.0f - m_angle;
                setProperty(AngleKey, m_angle, true);
            }
            invalidateGeometry();
        }

        void Entity::flip(Axis::Type axis, const Vec3f& flipCenter, bool lockTextures) {
            if (m_definition != NULL && m_definition->type() == EntityDefinition::BrushEntity)
                return;
            
            Vec3f offset = center() - origin();
            Vec3f newCenter = center().flipped(axis, flipCenter);
            setProperty(OriginKey, newCenter + offset, true);
            setProperty(AngleKey, 0, true);
            
            if (m_angle >= 0)
                m_angle = (m_angle + 180.0f) - static_cast<int>(m_angle / 360.0f) * m_angle;
            else if (m_angle == -1)
                m_angle = -2;
            else if (m_angle == -2)
                m_angle = -1;
            setProperty(AngleKey, m_angle, true);
            invalidateGeometry();
        }

        void Entity::pick(const Ray& ray, PickResult& pickResults) {
            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math::isnan(dist))
                return;
            
            Vec3f hitPoint = ray.pointAtDistance(dist);
            EntityHit* hit = new EntityHit(*this, hitPoint, dist);
            pickResults.add(*hit);
        }
    }
}