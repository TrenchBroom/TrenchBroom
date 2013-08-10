/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "QuakeEntityRotator.h"

#include "VecMath.h"
#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"

namespace TrenchBroom {
    namespace Model {
        Quatf QuakeEntityRotationPolicy::getRotation(const Entity& entity) {
            const RotationInfo info = rotationInfo(entity);
            switch (info.type) {
                case RTZAngle: {
                    const PropertyValue angleValue = entity.property(info.property);
                    if (angleValue.empty())
                        return Quatf(Vec3f::PosZ, 0.0f);
                    const float angle = static_cast<float>(std::atof(angleValue.c_str()));
                    return Quatf(Vec3f::PosZ, Mathf::radians(angle));
                }
                case RTZAngleWithUpDown: {
                    const PropertyValue angleValue = entity.property(info.property);
                    if (angleValue.empty())
                        return Quatf(Vec3f::PosZ, 0.0f);
                    const float angle = static_cast<float>(std::atof(angleValue.c_str()));
                    if (angle == -1.0f)
                        return Quatf(Vec3f::PosY, -Mathf::Pi / 2.0f);
                    if (angle == -2.0f)
                        return Quatf(Vec3f::PosY,  Mathf::Pi / 2.0f);
                    return Quatf(Vec3f::PosZ, Mathf::radians(angle));
                }
                case RTEulerAngles: {
                    const PropertyValue angleValue = entity.property(info.property);
                    const Vec3f angles = angleValue.empty() ? Vec3f::Null : Vec3f(angleValue);
                    
                    // yaw / pitch
                    // const Quatf zRotation(Vec3f::PosZ, Mathf::radians( angles.x()));
                    // const Quatf yRotation(Vec3f::PosY, Mathf::radians(-angles.y()));
                    
                    // pitch / yaw / roll
                    const Quatf pitch(Vec3f::PosY, Mathf::radians(angles.x()));
                    const Quatf yaw(Vec3f::PosZ, Mathf::radians(angles.y()));
                    const Quatf roll(Vec3f::PosX, Mathf::radians(angles.z()));
                    return pitch * yaw * roll;
                }
                default:
                    return Quatf(Vec3f::PosZ, 0.0f);
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
                    const bool brushEntity = entity.brushes().empty() || (entity.definition() != NULL && entity.definition()->type() == Assets::EntityDefinition::BrushEntity);
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
                        const Vec3f offset = entity.origin() - entity.bounds().center();
                        if (offset.x() == 0.0f && offset.y() == 0.0f) {
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
    }
}
