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

#include "MovementRestriction.h"

#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace View {
        MovementRestriction::MovementRestriction() :
        m_horizontalRestriction(ARNone),
        m_xAxis(Vec3::PosX),
        m_yAxis(Vec3::PosY),
        m_verticalRestriction(false) {}
        
        void MovementRestriction::toggleHorizontalRestriction(const Renderer::Camera& camera) {
            switch (m_horizontalRestriction) {
                case ARNone:
                    if (camera.right().firstComponent() == Math::Axis::AX)
                        m_yAxis = Vec3::Null;
                    else
                        m_xAxis = Vec3::Null;
                    m_horizontalRestriction = ARLeftRight;
                    break;
                case ARLeftRight:
                    if (m_xAxis == Vec3::Null) {
                        m_xAxis = Vec3::PosX;
                        m_yAxis = Vec3::Null;
                    } else {
                        m_xAxis = Vec3::Null;
                        m_yAxis = Vec3::PosY;
                    }
                    m_horizontalRestriction = ARForwardBack;
                    break;
                case ARForwardBack:
                    m_xAxis = Vec3f::PosX;
                    m_yAxis = Vec3f::PosY;
                    m_horizontalRestriction = ARNone;
                    break;
            }
        }
        
        void MovementRestriction::setVerticalRestriction(const bool verticalRestriction) {
            m_verticalRestriction = verticalRestriction;
        }
        
        bool MovementRestriction::isRestricted(const Math::Axis::Type axis) const {
            switch (axis) {
                case Math::Axis::AX:
                    return m_xAxis.null();
                case Math::Axis::AY:
                    return m_yAxis.null();
                default:
                    return m_verticalRestriction;
            }
        }
        
        Vec3 MovementRestriction::apply(const Vec3& v) const {
            if (m_verticalRestriction)
                return v.dot(Vec3::PosZ) * Vec3::PosZ;
            return v.dot(m_xAxis) * Vec3::PosX + v.dot(m_yAxis) * Vec3::PosY;
        }
    }
}
