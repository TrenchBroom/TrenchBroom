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

#include <vector>

namespace TrenchBroom {
    namespace Model {
        /**
         * This class represents an attribute of a tag. A tag can have multiple attributes, but the names must
         * be unique.
         */
        class TagAttribute {
        public:
            using AttributeType = unsigned long;
        private:
            AttributeType m_type;
            String m_name;
        public:
            /**
             * Creates a new tag attribute with the given type and name.
             *
             * @param type the attribute type
             * @param name the attribute name
             */
            explicit TagAttribute(AttributeType type, String name);

            /**
             * Returns the type of this attribute.
             */
            AttributeType type() const;

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
        protected:
            TagType m_type;
            String m_name;
            std::vector<TagAttribute> m_attributes;

            /**
             * Creates a new tag with the given type, name and attributes. The tag's type will be set automatically.
             *
             * @param type the tag's type
             * @param name the tag's name
             * @param attributes the tag's attributes
             */
            Tag(TagType type, String name, std::vector<TagAttribute> attributes);
        public:
            /**
             * Creates a new tag with the given name and attributes. The tag's type will be set automatically.
             *
             * @param name the tag's name
             * @param attributes the tag's attributes
             */
            Tag(String name, std::vector<TagAttribute> attributes);

            /**
             * Returns the type of this tag.
             */
            TagType type() const;

            /**
             * Sets the tag type. Can be called only once.
             *
             * @param type the type
             */
            void setType(TagType type);

            /**
             * Returns the name of this tag.
             */
            const String& name() const;

            /**
             * Returns the attributes of this tag.
             */
            const std::vector<TagAttribute>& attributes() const;

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

            /**
             * Returns the referenced tag.
             */
            const Tag& tag() const;

            friend bool operator==(const TagReference& lhs, const TagReference& rhs);
            friend bool operator<(const TagReference& lhs, const TagReference& rhs);
        };

        class TagManager;
        class TagMatcher;

        /**
         * Implementing this interface gives a class the ability to be tagged.
         */
        class Taggable {
        private:
            Tag::TagType m_tagMask;
            std::set<TagReference> m_tags;
            TagAttribute::AttributeType m_attributeMask;
        public:
            /**
             * Creates a new instance.
             */
            Taggable();

            virtual ~Taggable();

            /**
             * Indicates whether this object has the given tag.
             *
             * @param tag the tag to check
             * @return true if this object has the given tag and false otherwise
             */
            bool hasTag(const Tag& tag) const;

            /**
             * Indicates whether this object has any of the tags encoded in the given bit mask.
             *
             * @param mask the mask to check
             * @return true if this tag has any of the given tags
             */
            bool hasTag(Tag::TagType mask) const;

            /**
             * Returns a bit mask indicating which tags this object is tagged with.
             *
             * @return the tag mask
             */
            Tag::TagType tagMask() const;

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

            /**
             * Clears the tags of this object and adds all matching smart tags registered with the given tag manager.
             *
             * @param tagManager the tag manager
             */
            virtual void initializeTags(TagManager& tagManager);

            /**
             * Updates the tags of this object using the given tag manager.
             *
             * @param tagManager the tag manager
             */
            virtual void updateTags(TagManager& tagManager);

            /**
             * Removes all tags from this object.
             */
            virtual void clearTags();

            /**
             * Indicates whether any of the tags associated with this object has the given tag attribute.
             *
             * @param attribute the attribute to check
             * @return true if any tag of this object has the given attribute and false otherwise
             */
            bool hasAttribute(const TagAttribute& attribute) const;

            /**
             * Evaluates the given matcher against this taggable object. This method is used as part of a double dispatch
             * to retrieve the type information of this taggable object.
             *
             * @param matcher the matcher to evaluate against this object
             * @return true if the given matcher matches this object and false otherwise
             */
            bool evaluateTagMatcher(const TagMatcher& matcher) const;
        private:
            void updateAttributeMask();
        private:
            virtual bool doEvaluateTagMatcher(const TagMatcher& matcher) const = 0;
        };

        class BrushFace;
        class Brush;
        class Entity;
        class Group;
        class Layer;
        class World;

        /**
         * Decides whether a taggable object should be tagged with a particular smart tag.
         */
        class TagMatcher {
        public:
            virtual ~TagMatcher();
        public:
            /**
             * Evaluates this tag matcher against the given taggable by calling Taggable::evaluateTagMatcher.
             *
             * @param taggable the taggable to match against
             * @return true if this matcher matches the given taggable and false otherwise
             */
            bool matches(const Taggable& taggable) const;

            virtual std::unique_ptr<TagMatcher> clone() const = 0;
        public:
            virtual bool matches(const BrushFace& face) const;
            virtual bool matches(const World& world) const;
            virtual bool matches(const Layer& layer) const;
            virtual bool matches(const Group& group) const;
            virtual bool matches(const Entity& entity) const;
            virtual bool matches(const Brush& brush) const;
        };

        /**
         * A smart tag is applied to any object that satisfies a matcher automatically.
         */
        class SmartTag : public Tag {
        private:
            std::unique_ptr<TagMatcher> m_matcher;
        public:
            /**
             * Creates a new smart tag with the given name, attributes and matcher.
             *
             * @param name the name of this tag
             * @param attributes the attributes of this tag
             * @param matcher the matcher that decides whether to apply this tag to a given taggable
             */
            SmartTag(String name, std::vector<TagAttribute> attributes, std::unique_ptr<TagMatcher> matcher);

            SmartTag(const SmartTag& other);
            SmartTag(SmartTag&& other) noexcept;

            SmartTag& operator=(const SmartTag& other);
            SmartTag& operator=(SmartTag&& other);

            /**
             * Updates the given tag depending on whether or not the matcher matches against it.
             *
             * @param taggable the taggable to update
             */
            void update(Taggable& taggable) const;
        };
    }
}

#endif //TRENCHBROOM_TAG_H
