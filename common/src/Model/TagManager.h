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

#ifndef TRENCHBROOM_TAGMANAGER_H
#define TRENCHBROOM_TAGMANAGER_H

#include "StringUtils.h"
#include "Model/Tag.h"

#include <list>

namespace TrenchBroom {
    namespace Model {
        /**
         * Manages the tags used in a document and updates smart tags on taggable objects.
         */
        class TagManager {
        private:
            std::list<SmartTag> m_smartTags;
            class TagCmp;
        public:
            /**
             * Returns a vector containing all smart tags registered with this manager.
             */
            const std::list<SmartTag>& smartTags() const;

            /**
             * Indicates whether a smart tag with the given name is registered with this tag manager.
             *
             * @param name the tag name to check
             * @return true if a tag with the given name is registered and false otherwise
             */
            bool isRegisteredSmartTag(const String& name) const;

            /**
             * Returns the smart tag with the given name.
             *
             * @param name the name of the smart tag with the given name
             * @return the smart tag with the given name
             *
             * @throws std::logic_error if no tag with the given name is registered
             */
            const SmartTag& smartTag(const String& name) const;

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
             * Register the given smart tag with this tag manager.
             *
             * @param tag the smart tag to register
             *
             * @throws std::logic_error if the given smart tag is already registered
             */
            void registerSmartTag(SmartTag tag);

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
