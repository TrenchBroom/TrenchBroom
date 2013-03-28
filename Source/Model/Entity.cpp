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
        String const Entity::WadKey              = "wad";
        String const Entity::DefKey              = "_def";
        String const Entity::DefaultDefinition   = "Quake.fgd";
        String const Entity::FacePointFormatKey  = "_point_format";

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

        const Entity::RotationInfo Entity::rotationInfo() const {
            RotationType type = RTNone;
            PropertyKey property;
            
            // determine the type of rotation to apply to this entity
            if (classname() != NULL) {
                if (Utility::startsWith(*classname(), "light")) {
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
                        if (offset.x == 0.0f && offset.y == 0.0f) {
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

        void Entity::applyRotation(const Quat& rotation) {
            const RotationInfo info = rotationInfo();
            
            switch (info.type) {
                case RTZAngle: {
                    if (rotation.v.firstComponent() != Axis::AZ)
                        return;
                    
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    float angle = angleValue != NULL ? static_cast<float>(std::atof(angleValue->c_str())) : 0.0f;
                    
                    Vec3f direction;
                    direction.x = std::cos(Math::radians(angle));
                    direction.y = std::sin(Math::radians(angle));

                    direction = rotation * direction;
                    direction.z = 0.0f;
                    direction.normalize();
                    
                    angle = Math::round(Math::degrees(std::acos(direction.x)));
                    if (direction.y < 0.0f)
                        angle = 360.0f - angle;
                    setProperty(info.property, angle, true);
                    break;
                }
                case RTZAngleWithUpDown: {
                    if (rotation.v.firstComponent() != Axis::AZ)
                        return;

                    const PropertyValue* angleValue = propertyForKey(info.property);
                    float angle = angleValue != NULL ? static_cast<float>(std::atof(angleValue->c_str())) : 0.0f;

                    Vec3f direction;
                    if (angle == -1.0f) {
                        direction = Vec3f::PosZ;
                    } else if (angle == -2.0f) {
                        direction = Vec3f::NegZ;
                    } else {
                        direction.x = std::cos(Math::radians(angle));
                        direction.y = std::sin(Math::radians(angle));
                    }

                    if (direction.z > 0.9f) {
                        setProperty(info.property, -1.0f, true);
                    } else if (direction.z < -0.9f) {
                        setProperty(info.property, -2.0f, true);
                    } else {
                        direction.z = 0.0f;
                        direction.normalize();
                        
                        angle = Math::round(Math::degrees(std::acos(direction.x)));
                        if (direction.y < 0.0f)
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
                    Quat zRotation(Math::radians(angles.x), Vec3f::PosZ);
                    Quat yRotation(Math::radians(-angles.y), Vec3f::PosY);
                    
                    direction = zRotation * yRotation * direction;
                    direction = rotation * direction;
                    
                    float zAngle, xAngle;
                    
                    // FIXME: this is still buggy
                    Vec3f xyDirection = direction;
                    if (std::abs(xyDirection.z) == 1.0f) {
                        zAngle = 0.0f;
                    } else {
                        xyDirection.z = 0.0f;
                        xyDirection.normalize();
                        zAngle = Math::round(Math::degrees(std::acos(xyDirection.x)));
                        if (xyDirection.y < 0.0f)
                            zAngle = 360.0f - zAngle;
                    }
                    
                    Vec3f xzDirection = direction;
                    if (std::abs(xzDirection.y) == 1.0f) {
                        xAngle = 0.0f;
                    } else {
                        xzDirection.y = 0.0f;
                        xzDirection.normalize();
                        xAngle = Math::round(Math::degrees(std::acos(xzDirection.x)));
                        if (xzDirection.z < 0.0f)
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

        Entity::Entity(const BBox& worldBounds) : MapObject(), m_worldBounds(worldBounds) {
            init();
        }

        Entity::Entity(const BBox& worldBounds, const Entity& entityTemplate) : MapObject(), m_worldBounds(worldBounds) {
            init();
            setProperties(entityTemplate.properties(), true);
        }

        Entity::~Entity() {
            setMap(NULL);
            Utility::deleteAll(m_brushes);
            setDefinition(NULL);
            m_geometryValid = false;
        }

        void Entity::setProperty(const PropertyKey& key, const PropertyValue& value) {
            setProperty(key, &value);
        }
        
        void Entity::setProperty(const PropertyKey& key, const PropertyValue* value) {
            if (key == ClassnameKey) {
                if (value != classname()) {
                    m_worldspawn = *value == WorldspawnClassname;
                    setDefinition(NULL);
                }
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
            if (replace) {
                m_propertyStore.clear();
                setProperty(SpawnFlagsKey, "0");
            }
            PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it)
                setProperty(it->key(), it->value());
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

        void Entity::removeProperty(const PropertyKey& key) {
            assert(propertyKeyIsMutable(key));
            if (!m_propertyStore.containsProperty(key))
                return;
            
            if (key == ClassnameKey)
                setDefinition(NULL);

            m_propertyStore.removeProperty(key);
            invalidateGeometry();
        }
        
        const Quat Entity::rotation() const {
            const RotationInfo info = rotationInfo();
            switch (info.type) {
                case RTZAngle: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    if (angleValue == NULL)
                        return Quat(0.0f, Vec3f::PosZ);
                    float angle = static_cast<float>(std::atof(angleValue->c_str()));
                    return Quat(Math::radians(angle), Vec3f::PosZ);
                }
                case RTZAngleWithUpDown: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    if (angleValue == NULL)
                        return Quat(0.0f, Vec3f::PosZ);
                    float angle = static_cast<float>(std::atof(angleValue->c_str()));
                    if (angle == -1.0f)
                        return Quat(-Math::Pi / 2.0f, Vec3f::PosY);
                    if (angle == -2.0f)
                        return Quat(Math::Pi / 2.0f, Vec3f::PosY);
                    return Quat(Math::radians(angle), Vec3f::PosZ);
                }
                case RTEulerAngles: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    Vec3f angles = angleValue != NULL ? Vec3f(*angleValue) : Vec3f::Null;
                    
                    Quat zRotation(Math::radians(angles.x), Vec3f::PosZ);
                    Quat yRotation(Math::radians(-angles.y), Vec3f::PosY);
                    return zRotation * yRotation;
                }
                default:
                    return Quat(0.0f, Vec3f::PosZ);
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

        void Entity::translate(const Vec3f& delta, bool lockTextures) {
            if (delta.null())
                return;
            
            Vec3f newOrigin = origin() + delta;
            setProperty(OriginKey, newOrigin, true);
            invalidateGeometry();
        }

        void Entity::rotate90(Axis::Type axis, const Vec3f& rotationCenter, bool clockwise, bool lockTextures) {
            if (m_brushes.empty()) {
                const Vec3f offset = origin() - center();
                const Vec3f newCenter = center().rotated90(axis, rotationCenter, clockwise);
                setProperty(OriginKey, newCenter + offset, true);
            }

            Quat rotation;
            switch (axis) {
                case Axis::AX:
                    rotation = clockwise ? Quat(-Math::Pi / 2.0f, Vec3f::PosX) : Quat(Math::Pi / 2.0f, Vec3f::PosX);
                    break;
                case Axis::AY:
                    rotation = clockwise ? Quat(-Math::Pi / 2.0f, Vec3f::PosY) : Quat(Math::Pi / 2.0f, Vec3f::PosY);
                    break;
                default:
                    rotation = clockwise ? Quat(-Math::Pi / 2.0f, Vec3f::PosZ) : Quat(Math::Pi / 2.0f, Vec3f::PosZ);
                    break;
            }
            
            applyRotation(rotation);
            invalidateGeometry();
        }

        void Entity::rotate(const Quat& rotation, const Vec3f& rotationCenter, bool lockTextures) {
            if (m_brushes.empty()) {
                const Vec3f offset = origin() - center();
                const Vec3f newCenter = rotation * (center() - rotationCenter) + rotationCenter;
                setProperty(OriginKey, newCenter + offset, true);
            }
            
            applyRotation(rotation);
            invalidateGeometry();
        }

        void Entity::flip(Axis::Type axis, const Vec3f& flipCenter, bool lockTextures) {
            if (m_brushes.empty()) {
                const Vec3f offset = origin() - center();
                const Vec3f newCenter = center().flipped(axis, flipCenter);
                setProperty(OriginKey, newCenter + offset, true);
            }
            
            RotationInfo info = rotationInfo();
            switch (info.type) {
                case RTZAngle: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    float angle = angleValue != NULL ? static_cast<float>(std::atof(angleValue->c_str())) : 0.0f;
                    switch (axis) {
                        case Axis::AX:
                            angle = 180.0f - angle;
                            break;
                        case Axis::AY:
                            angle = 360.0f - angle;
                            break;
                        default:
                            break;
                    }
                    setProperty(info.property, angle, true);
                    break;
                }
                case RTZAngleWithUpDown: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    float angle = angleValue != NULL ? static_cast<float>(std::atof(angleValue->c_str())) : 0.0f;
                    switch (axis) {
                        case Axis::AX:
                            if (angle != -1.0f && angle != -2.0f)
                                angle = 180.0f - angle;
                            break;
                        case Axis::AY:
                            if (angle != -1.0f && angle != -2.0f)
                                angle = 360.0f - angle;
                            break;
                        default:
                            if (angle == -1.0f)
                                angle = -2.0f;
                            else if (angle == -2.0f)
                                angle = -1.0f;
                            break;
                    }
                    setProperty(info.property, angle, true);
                    break;
                }
                case RTEulerAngles: {
                    const PropertyValue* angleValue = propertyForKey(info.property);
                    Vec3f angles = angleValue != NULL ? Vec3f(*angleValue) : Vec3f::Null;
                    switch (axis) {
                        case Axis::AX:
                            angles.x = 180.0f - angles.x;
                            break;
                        case Axis::AY:
                            angles.x = 360.0f - angles.x;
                            break;
                        default:
                            angles.z = -angles.z;
                            break;
                    }
                    setProperty(info.property, angles, true);
                    break;
                }
                default:
                    break;
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
