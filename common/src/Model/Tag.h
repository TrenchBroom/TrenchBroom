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

#ifndef TRENCHBROOM_TAG_H
#define TRENCHBROOM_TAG_H

#include "StringUtils.h"

#include <set>
#include <variant>

namespace TrenchBroom {
    namespace Model {
        /**
         * This class represents an attribute of a tag. A tag can have multiple attributes, but the names must
         * be unique.
         */
        class TagAttribute {
        private:
            String m_name;
        public:
            /**
             * Creates a new tag attribute with the given name;
             *
             * @param name the tag name
             */
            explicit TagAttribute(String name);

            /**
             * Returns the name of this tag attribute.
             */
            const String& name() const;

            friend bool operator==(const TagAttribute& lhs, const TagAttribute& rhs);
            friend bool operator<(const TagAttribute& lhs, const TagAttribute& rhs);
        };

        /**
         * Tags can be attached to any class that implement the Taggable interface. Each tag has a name and a type,
         * whereby the type is just a unique unsigned integer that acts as a bitmask index.
         *
         * Furthermore, a tag can have attributes.
         */
        class Tag {
        public:
            using TagType = unsigned long;
            static TagType freeTagType();
        private:
            TagType m_type;
            String m_name;
            std::set<TagAttribute> m_attributes;
        public:
            /**
             * Creates a new tag with the given name and attributes.
             *
             * @param name the tag's name
             * @param attributes the tag's attributes
             */
            Tag(String name, std::set<TagAttribute> attributes);

            /**
             * Returns the type of this tag.
             */
            TagType type() const;

            /**
             * Returns the name of this tag.
             */
            const String& name() const;

            friend bool operator==(const Tag& lhs, const Tag& rhs);
            friend bool operator<(const Tag& lhs, const Tag& rhs);
        };

        /**
         * Tag references are used to store the tags attached to an object implementing the Taggable interface.
         */
        class TagReference {
        private:
            const Tag& m_tag;
        public:
            /**
             * Creates a new reference to the given tag.
             *
             * @param tag the referenced tag
             */
            explicit TagReference(const Tag& tag);

            friend bool operator==(const TagReference& lhs, const TagReference& rhs);
            friend bool operator<(const TagReference& lhs, const TagReference& rhs);
        };

        /**
         * Implementing this interface gives a class the ability to be tagged.
         */
        class Taggable {
        private:
            Tag::TagType m_tagMask;
            std::set<TagReference> m_tags;
        public:
            /**
             * Creates a new instance.
             */
            Taggable();

            /**
             * Indicates whether this object has the given tag.
             *
             * @param tag the tag to check
             * @return true if this object has the given tag and false otherwise
             */
            bool hasTag(const Tag& tag) const;

            /**
             * Adds the given tag to this object.
             *
             * @param tag the tag to add
             * @return true if this object did not already have the given tag and false otherwise
             */
            bool addTag(const Tag& tag);

            /**
             * Removes the given tag from this object.
             *
             * @param tag the tag to remove
             * @return true if this object had the given tag and false otherwise
             */
            bool removeTag(const Tag& tag);
        };
    }
}

#endif //TRENCHBROOM_TAG_H
