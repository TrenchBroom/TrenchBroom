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

#pragma once

#include "Model/Tag.h"

#include <kdl/vector_set.h>

#include <string>

namespace TrenchBroom {
    namespace Model {
        /**
         * Manages the tags used in a document and updates smart tags on taggable objects.
         */
        class TagManager {
        private:
            struct TagCmp {
                bool operator()(const SmartTag& lhs, const SmartTag& rhs) const;
                bool operator()(const std::string& lhs, const SmartTag& rhs) const;
                bool operator()(const SmartTag& lhs, const std::string& rhs) const;
                bool operator()(const std::string& lhs, const std::string& rhs) const;
            };

            kdl::vector_set<SmartTag, TagCmp> m_smartTags;
        public:
            /**
             * Returns a vector containing all smart tags registered with this manager.
             */
            const std::vector<SmartTag>& smartTags() const;

            /**
             * Indicates whether a smart tag with the given name is registered with this tag manager.
             *
             * @param name the tag name to check
             * @return true if a tag with the given name is registered and false otherwise
             */
            bool isRegisteredSmartTag(const std::string& name) const;

            /**
             * Returns the smart tag with the given name.
             *
             * @param name the name of the smart tag with the given name
             * @return the smart tag with the given name
             *
             * @throws std::logic_error if no tag with the given name is registered
             */
            const SmartTag& smartTag(const std::string& name) const;

            /**
             * Indicates whether a smart tag with the given index is registered with this tag manager.
             *
             * @param index the tag index to check
             * @return true if a tag with the given index is registered and false otherwise
             */
            bool isRegisteredSmartTag(size_t index) const;

            /**
             * Returns the smart tag with the given index.
             *
             * @param index the index of the smart tag with the given name
             * @return the smart tag with the given index
             *
             * @throws std::logic_error if no tag with the given index is registered
             */
            const SmartTag& smartTag(size_t index) const;

            /**
             * Register the given smart tags with this tag manager.
             * The smart tags are copied into the manager and indexes and types are assigned. If this
             * manager already contains any smart tags, they are cleared before registering the given smart tags.
             *
             * @param tags the smart tags to register
             *
             * @throws std::logic_error if the given vector of smart tags contains more than one smart tag with the same
             * name
             */
            void registerSmartTags(const std::vector<SmartTag>& tags);

            /**
             * Clears all registered smart tags;
             */
            void clearSmartTags();

            /**
             * Update the smart tags of the given taggable object.
             *
             * @param taggable the object to update
             */
            void updateTags(Taggable& taggable) const;
        private:
            size_t freeTagIndex();
        };
    }
}

#endif //TRENCHBROOM_TAGMANAGER_H
