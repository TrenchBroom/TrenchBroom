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

namespace TrenchBroom {
    namespace View {
        MovementRestriction::MovementRestriction() :
        m_restriction(Restriction_None),
        m_verticalRestriction(false) {}
        
        void MovementRestriction::toggleRestriction() {
            switch (m_restriction) {
                case Restriction_None:
                    m_restriction = Restriction_X;
                    break;
                case Restriction_X:
                    m_restriction = Restriction_Y;
                    break;
                case Restriction_Y:
                    m_restriction = Restriction_Z;
                    break;
                case Restriction_Z:
                    m_restriction = Restriction_None;
                    break;
                switchDefault()
            }
        }
        
        void MovementRestriction::toggleVerticalRestriction(const bool verticalRestriction) {
            m_verticalRestriction = verticalRestriction;
        }
        
        void MovementRestriction::toggleRestriction(const Math::Axis::Type axis) {
            if (isRestricted(axis)) {
                m_restriction = Restriction_None;
                return;
            }
            
            switch (axis) {
                case Math::Axis::AX:
                    m_restriction = Restriction_X;
                    break;
                case Math::Axis::AY:
                    m_restriction = Restriction_Y;
                    break;
                default:
                    m_restriction = Restriction_Z;
                    break;
            }
        }

        bool MovementRestriction::isRestricted(const Math::Axis::Type axis) const {
            if (!m_verticalRestriction && m_restriction == Restriction_None)
                return false;
            switch (axis) {
                case Math::Axis::AX:
                    return !m_verticalRestriction && m_restriction == Restriction_X;
                case Math::Axis::AY:
                    return !m_verticalRestriction && m_restriction == Restriction_Y;
                default:
                    return  m_verticalRestriction || m_restriction == Restriction_Z;
            }
        }
        
        Vec3 MovementRestriction::apply(const Vec3& v) const {
            return v.dot(xAxis()) * Vec3::PosX + v.dot(yAxis()) * Vec3::PosY + v.dot(zAxis()) * Vec3::PosZ;
        }

        const Vec3& MovementRestriction::xAxis() const {
            return canMoveAlong(Math::Axis::AX) ? Vec3::PosX : Vec3::Null;
        }
        
        const Vec3& MovementRestriction::yAxis() const {
            return canMoveAlong(Math::Axis::AY) ? Vec3::PosY : Vec3::Null;
        }
        
        const Vec3& MovementRestriction::zAxis() const {
            return canMoveAlong(Math::Axis::AZ) ? Vec3::PosZ : Vec3::Null;
        }

        bool MovementRestriction::canMoveAlong(const Math::Axis::Type axis) const {
            if (!m_verticalRestriction && m_restriction == Restriction_None)
                return true;
            switch (axis) {
                case Math::Axis::AX:
                    return !m_verticalRestriction && m_restriction == Restriction_X;
                case Math::Axis::AY:
                    return !m_verticalRestriction && m_restriction == Restriction_Y;
                default:
                    return  m_verticalRestriction || m_restriction == Restriction_Z;
            }
        }
    }
}
