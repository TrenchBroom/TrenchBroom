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

#include "Macros.h"
#include "Model/TagType.h"

#include <kdl/vector_set.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class ConstTagVisitor;
        class TagManager;
        class TagVisitor;

        /**
         * This class represents an attribute of a tag. A tag can have multiple attributes, but the names must
         * be unique.
         */
        class TagAttribute {
        public:
            using AttributeType = unsigned long;
        private:
            AttributeType m_type;
            std::string m_name;
        public:
            /**
             * Creates a new tag attribute with the given type and name.
             *
             * @param type the attribute type
             * @param name the attribute name
             */
            explicit TagAttribute(AttributeType type, const std::string& name);

            /**
             * Returns the type of this attribute.
             */
            AttributeType type() const;

            /**
             * Returns the name of this tag attribute.
             */
            const std::string& name() const;

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
        protected:
            size_t m_index;
            std::string m_name;
            std::vector<TagAttribute> m_attributes;

            /**
             * Creates a new tag with the given index, name and attributes.
             *
             * @param index the tag's index
             * @param name the tag's name
             * @param attributes the tag's attributes
             */
            Tag(size_t index, const std::string& name, std::vector<TagAttribute> attributes);
        public:
            /**
             * Creates a new tag with the given name and attributes. The tag's type will be set automatically.
             *
             * @param name the tag's name
             * @param attributes the tag's attributes
             */
            Tag(const std::string& name, std::vector<TagAttribute> attributes);

            virtual ~Tag();

            Tag(const Tag& other);
            Tag(Tag&& other) noexcept;

            Tag& operator=(const Tag& other);
            Tag& operator=(Tag&& other);

            /**
             * Returns the type of this tag.
             */
            TagType::Type type() const;

            /**
             * Returns the index of this tag.
             */
            size_t index() const;

            /**
             * Sets the tag's index.
             *
             * @param index the index
             */
            void setIndex(size_t index);

            /**
             * Returns the name of this tag.
             */
            const std::string& name() const;

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
            const Tag* m_tag;
        public:
            /**
             * Creates a new reference to the given tag.
             *
             * @param tag the referenced tag
             */
            explicit TagReference(const Tag& tag);

            defineCopyAndMove(TagReference)

            /**
             * Returns the referenced tag.
             */
            const Tag& tag() const;

            friend bool operator==(const TagReference& lhs, const TagReference& rhs);
            friend bool operator<(const TagReference& lhs, const TagReference& rhs);
        };

        /**
         * Implementing this interface gives a class the ability to be tagged.
         */
        class Taggable {
        private:
            TagType::Type m_tagMask;
            kdl::vector_set<TagReference> m_tags;
            TagAttribute::AttributeType m_attributeMask;
        public:
            /**
             * Creates a new instance.
             */
            Taggable();
            defineCopyAndMove(Taggable)

            friend void swap(Taggable& lhs, Taggable& rhs) noexcept;

            virtual ~Taggable();

            /**
             * Indicates whether this object has any tag.
             *
             * @return true if this object has any tag and false otherwise
             */
            bool hasAnyTag() const;

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
            bool hasTag(TagType::Type mask) const;

            /**
             * Returns a bit mask indicating which tags this object is tagged with.
             *
             * @return the tag mask
             */
            TagType::Type tagMask() const;

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
             * Accepts the given tag visitor.
             *
             * @param visitor the visitor to accept
             */
            void accept(TagVisitor& visitor);

            /**
             * Accepts the given tag visitor.
             *
             * @param visitor the visitor to accept
             */
            void accept(ConstTagVisitor& visitor) const;
        private:
            void updateAttributeMask();
        private:
            virtual void doAcceptTagVisitor(TagVisitor& visitor) = 0;
            virtual void doAcceptTagVisitor(ConstTagVisitor& visitor) const = 0;
        };

        class MapFacade;

        /**
         * A mechanism to query user input when enabling or disabling a tag matcher.
         */
        class TagMatcherCallback {
        public:
            virtual ~TagMatcherCallback();
            virtual size_t selectOption(const std::vector<std::string>& options) = 0;
        };

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
            virtual bool matches(const Taggable& taggable) const = 0;

            /**
             * Modifies the current selection so that this tag matcher would match it.
             *
             * @param callback a callback mechanism to query  user input
             * @param facade the map facade to issue commands with
             */
            virtual void enable(TagMatcherCallback& callback, MapFacade& facade) const;

            /**
             * Modifies the current selection so that this tag matcher would not match it.
             *
             * @param callback a callback mechanism to query  user input
             * @param facade the map facade to issue commands with
             */
            virtual void disable(TagMatcherCallback& callback, MapFacade& facade) const;

            /**
             * Indicates whether this tag matcher can modify the selection so that it would match it.
             *
             * @return true if this tag matcher can modify the selection appropriately and false otherwise
             */
            virtual bool canEnable() const;

            /**
             * Indicates whether this tag matcher can modify the selection so that it would not match it.
             *
             * @return true if this tag matcher can modify the selection appropriately and false otherwise
             */
            virtual bool canDisable() const;

            /**
             * Returns a new copy of this tag matcher.
             */
            virtual std::unique_ptr<TagMatcher> clone() const = 0;
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
            SmartTag(const std::string& name, std::vector<TagAttribute> attributes, std::unique_ptr<TagMatcher> matcher);

            SmartTag(const SmartTag& other);
            SmartTag(SmartTag&& other) noexcept;

            SmartTag& operator=(const SmartTag& other);
            SmartTag& operator=(SmartTag&& other);

            /**
             * Indicates whether this smart tag matches the given taggable.
             *
             * @param taggable the taggable to match
             * @return true if this smart tag matches the given taggable and false otherwise
             */
            bool matches(const Taggable& taggable) const;

            /**
             * Updates the given tag depending on whether or not the matcher matches against it.
             *
             * @param taggable the taggable to update
             */
            void update(Taggable& taggable) const;

            /**
             * Modifies the current selection so that this tag would match it.
             *
             * @param callback a callback mechanism to query  user input
             * @param facade the map facade to issue commands with
             */
            void enable(TagMatcherCallback& callback, MapFacade& facade) const;

            /**
             * Modifies the current selection so that this tag would not match it.
             *
             * @param callback a callback mechanism to query  user input
             * @param facade the map facade to issue commands with
             */
            void disable(TagMatcherCallback& callback, MapFacade& facade) const;

            /**
             * Indicates whether this tag supports modifying the selection so that it would match it.
             *
             * @return true if this tag can modify the selection appropriately and false otherwise
             */
            bool canEnable() const;

            /**
             * Indicates whether this tag supports modifying the selection so that it would not match it.
             *
             * @return true if this tag can modify the selection appropriately and false otherwise
             */
            bool canDisable() const;
        };
    }
}


