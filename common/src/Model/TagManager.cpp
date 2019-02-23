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

#include <algorithm>
#include <stdexcept>

namespace TrenchBroom {
    namespace Model {
        class TagManager::TagCmp {
        public:
            bool operator()(const SmartTag& lhs, const SmartTag& rhs) const {
                return lhs.name() < rhs.name();
            }

            bool operator()(const String& lhs, const SmartTag& rhs) const {
                return lhs < rhs.name();
            }

            bool operator()(const SmartTag& lhs, const String& rhs) const {
                return lhs.name() < rhs;
            }

            bool operator()(const String& lhs, const String& rhs) const {
                return lhs < rhs;
            }
        };

        TagManager::TagManager() :
        m_currentTagTypeIndex(0) {}

        const std::vector<SmartTag>& TagManager::smartTags() const {
            return m_smartTags;
        }

        bool TagManager::isRegisteredSmartTag(const String& name) const {
            const auto it = std::lower_bound(std::begin(m_smartTags), std::end(m_smartTags), name, TagCmp());
            return it != std::end(m_smartTags) && !(it->name() < name || name < it->name());
        }

        const SmartTag& TagManager::smartTag(const String& name) const {
            const auto it = std::lower_bound(std::begin(m_smartTags), std::end(m_smartTags), name, TagCmp());
            if (it == std::end(m_smartTags) || (it->name() < name || name < it->name())) {
                throw std::logic_error("Smart tag not registered");
            }
            return *it;
        }

        void TagManager::registerSmartTag(SmartTag tag) {
            const auto it = std::lower_bound(std::begin(m_smartTags), std::end(m_smartTags), tag, TagCmp());
            if (it == std::end(m_smartTags)) {
                tag.setIndex(freeTagIndex());
                m_smartTags.push_back(std::move(tag));
            } else if (*it < tag || tag < *it) {
                tag.setIndex(freeTagIndex());
                m_smartTags.insert(it, std::move(tag));
            } else {
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

        size_t TagManager::freeTagIndex() {
            static const size_t Bits = (sizeof(Tag::TagType) * 8);

            ensure(m_currentTagTypeIndex <= Bits, "no more tag types");
            return m_currentTagTypeIndex++;
        }
    }
}
