/*
 Copyright (C) 2020 Kristian Duske

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

#include "Group.h"

namespace TrenchBroom {
    namespace Model {
        Group::Group(std::string name) :
        m_name(std::move(name)) {}

        const std::string& Group::name() const {
            return m_name;
        }

        void Group::setName(std::string name) {
            m_name = std::move(name);
        }

        std::optional<std::string> Group::linkedGroupId() const {
            return m_linkedGroupId;
        }

        void Group::setLinkedGroupId(std::string linkedGroupId) {
            m_linkedGroupId = std::move(linkedGroupId);
        }

        void Group::resetLinkedGroupId() {
            m_linkedGroupId = std::nullopt;
        }

        bool operator==(const Group& lhs, const Group& rhs) {
            return lhs.m_name == rhs.m_name && lhs.m_linkedGroupId == rhs.m_linkedGroupId;
        }

        bool operator!=(const Group& lhs, const Group& rhs) {
            return !(lhs == rhs);
        }
    }
}
