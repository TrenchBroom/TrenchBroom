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

#include "QuakeEntityRotationPolicy.h"

#include "VecMath.h"
#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"

namespace TrenchBroom {
    namespace Model {
        Quat3 QuakeEntityRotationPolicy::getRotation(const Entity& entity) {
            const RotationInfo info = rotationInfo(entity);
            switch (info.type) {
                case RTZAngle: {
                    const PropertyValue angleValue = entity.property(info.property);
                    if (angleValue.empty())
                        return Quat3(Vec3::PosZ, 0.0);
                    const FloatType angle = static_cast<FloatType>(std::atof(angleValue.c_str()));
                    return Quat3(Vec3::PosZ, Math::radians(angle));
                }
                case RTZAngleWithUpDown: {
                    const PropertyValue angleValue = entity.property(info.property);
                    if (angleValue.empty())
                        return Quat3(Vec3::PosZ, 0.0);
                    const FloatType angle = static_cast<FloatType>(std::atof(angleValue.c_str()));
                    if (angle == -1.0)
                        return Quat3(Vec3::PosY, -Math::Constants<FloatType>::PiOverTwo);
                    if (angle == -2.0)
                        return Quat3(Vec3::PosY,  Math::Constants<FloatType>::PiOverTwo);
                    return Quat3(Vec3::PosZ, Math::radians(angle));
                }
                case RTEulerAngles: {
                    const PropertyValue angleValue = entity.property(info.property);
                    const Vec3 angles = angleValue.empty() ? Vec3::Null : Vec3(angleValue);
                    
                    // yaw / pitch
                    // const Quatf zRotation(Vec3f::PosZ, Mathf::radians( angles.x()));
                    // const Quatf yRotation(Vec3f::PosY, Mathf::radians(-angles.y()));
                    
                    // pitch / yaw / roll
                    const Quat3 pitch(  Vec3::PosY, Math::radians(angles.x()));
                    const Quat3 yaw(    Vec3::PosZ, Math::radians(angles.y()));
                    const Quat3 roll(   Vec3::PosX, Math::radians(angles.z()));
                    return pitch * yaw * roll;
                }
                default:
                    return Quat3(Vec3::PosZ, 0.0);
            }
        }

        QuakeEntityRotationPolicy::RotationInfo QuakeEntityRotationPolicy::rotationInfo(const Entity& entity) {
            RotationType type = RTNone;
            PropertyKey property;
            
            // determine the type of rotation to apply to this entity
            const PropertyValue classname = entity.classname();
            if (classname != PropertyValues::NoClassname) {
                if (StringUtils::isPrefix(classname, "light")) {
                    if (entity.hasProperty(PropertyKeys::Mangle)) {
                        // spotlight without a target, update mangle
                        type = RTEulerAngles;
                        property = PropertyKeys::Mangle;
                    } else if (!entity.hasProperty(PropertyKeys::Target)) {
                        // not a spotlight, but might have a rotatable model, so change angle or angles
                        if (entity.hasProperty(PropertyKeys::Angles)) {
                            type = RTEulerAngles;
                            property = PropertyKeys::Angles;
                        } else {
                            type = RTZAngle;
                            property = PropertyKeys::Angle;
                        }
                    } else {
                        // spotlight with target, don't modify
                    }
                } else {
                    const bool brushEntity = !entity.brushes().empty() || (entity.definition() != NULL && entity.definition()->type() == Assets::EntityDefinition::BrushEntity);
                    if (brushEntity) {
                        if (entity.hasProperty(PropertyKeys::Angles)) {
                            type = RTEulerAngles;
                            property = PropertyKeys::Angles;
                        } else if (entity.hasProperty(PropertyKeys::Angle)) {
                            type = RTZAngleWithUpDown;
                            property = PropertyKeys::Angle;
                        }
                    } else {
                        // point entity
                        
                        // if the origin of the definition's bounding box is not in its center, don't apply the rotation
                        const Vec3 offset = entity.origin() - entity.bounds().center();
                        if (offset.x() == 0.0 && offset.y() == 0.0) {
                            if (entity.hasProperty(PropertyKeys::Angles)) {
                                type = RTEulerAngles;
                                property = PropertyKeys::Angles;
                            } else {
                                type = RTZAngle;
                                property = PropertyKeys::Angle;
                            }
                        }
                    }
                }
            }
            
            return RotationInfo(type, property);
        }

        void QuakeEntityRotationPolicy::applyRotation(Entity& entity, const Mat4x4& transformation) {
            const RotationInfo info = rotationInfo(entity);
            const Quatf rotation = getRotation(entity);

            Vec3 direction = rotation * Vec3::PosX;
            direction = transformation * direction;
            direction.normalize();
            
            switch (info.type) {
                case RTZAngle:
                    setAngle(entity, info.property, direction);
                    break;
                case RTZAngleWithUpDown:
                    if (direction.z() > 0.9)
                        entity.addOrUpdateProperty(info.property, 1.0);
                    else if (direction.z() < -0.9)
                        entity.addOrUpdateProperty(info.property, -1.0);
                    else
                        setAngle(entity, info.property, direction);
                    break;
                case RTEulerAngles: {
                    FloatType zAngle, xAngle;
                    
                    if (Math::eq(std::abs(direction.z()), 1.0))
                        zAngle = 0.0;
                    else
                        zAngle = getAngle(direction);
                    
                    if (Math::eq(std::abs(direction.y()), 1.0)) {
                        xAngle = 0.0;
                    } else {
                        Vec3 xzDirection = direction;
                        using std::swap;
                        swap(xzDirection[1], xzDirection[2]);
                        xAngle = getAngle(xzDirection);
                    }
                    
                    entity.addOrUpdateProperty(info.property, Vec3(zAngle, xAngle, 0.0).round());
                    break;
                }
                default:
                    break;
            }
        }

        void QuakeEntityRotationPolicy::setAngle(Entity& entity, const PropertyKey& key, const Vec3& direction) {
            const FloatType angle = getAngle(direction);
            entity.addOrUpdateProperty(key, Math::round(angle));
        }

        FloatType QuakeEntityRotationPolicy::getAngle(Vec3 direction) {
            direction[2] = 0.0;
            direction.normalize();
            
            FloatType angle = Math::round(Math::degrees(std::acos(direction.x())));
            if (Math::neg(direction.y()))
                angle = 360.0 - angle;
            while (Math::neg(angle))
                angle += 360.0;
            return angle;
        }
    }
}
