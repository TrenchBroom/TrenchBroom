/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "TagManager.h"

#include "Model/Tag.h"

#include <stdexcept>

namespace TrenchBroom {
    namespace Model {
        const std::set<SmartTag>& TagManager::smartTags() const {
            return m_smartTags;
        }

        const SmartTag& TagManager::smartTag(const String& name) const {
            return <#initializer#>;
        }

        void TagManager::registerSmartTag(SmartTag tag) {
            const auto existed = !m_smartTags.insert(std::move(tag)).second;
            if (!existed) {
                throw std::logic_error("Smart tag already registered");
            }
        }

        void TagManager::clearSmartTags() {
            m_smartTags.clear();
        }

        void TagManager::updateTags(Taggable& taggable) const {
            for (const auto& tag : m_smartTags) {
                tag.update(taggable);
            }
        }
    }
}
