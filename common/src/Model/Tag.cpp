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

#include "Tag.h"

#include "IO/Path.h"
#include "Model/TagManager.h"

namespace TrenchBroom {
    namespace Model {
        TagAttribute::TagAttribute(const AttributeType type, String name) :
        m_type(type),
        m_name(std::move(name) ){}

        TagAttribute::AttributeType TagAttribute::type() const {
            return m_type;
        }

        const String& TagAttribute::name() const {
            return m_name;
        }

        bool operator==(const TagAttribute& lhs, const TagAttribute& rhs) {
            return lhs.m_name == rhs.m_name;
        }

        bool operator<(const TagAttribute& lhs, const TagAttribute& rhs) {
            return lhs.m_name < rhs.m_name;
        }

        Tag::Tag(const size_t index, String name, std::vector<TagAttribute> attributes) :
        m_index(index),
        m_name(std::move(name)),
        m_attributes(std::move(attributes)) {}

        Tag::Tag(String name, std::vector<TagAttribute> attributes) :
        Tag(0, name, attributes) {}

        Tag::~Tag() = default;

        Tag::Tag(const Tag& other) = default;
        Tag::Tag(Tag&& other) noexcept = default;

        Tag& Tag::operator=(const Tag& other) = default;
        Tag& Tag::operator=(Tag&& other) = default;

        Tag::TagType Tag::type() const {
            return 1UL << m_index;
        }

        size_t Tag::index() const {
            return m_index;
        }

        void Tag::setIndex(const size_t index) {
            m_index = index;
        }

        const String& Tag::name() const {
            return m_name;
        }

        const std::vector<TagAttribute>& Tag::attributes() const {
            return m_attributes;
        }

        bool operator==(const Tag& lhs, const Tag& rhs) {
            return lhs.m_name == rhs.m_name;
        }

        bool operator<(const Tag& lhs, const Tag& rhs) {
            return lhs.m_name < rhs.m_name;
        }

        TagReference::TagReference(const Tag& tag) :
        m_tag(tag) {}

        const Tag& TagReference::tag() const {
            return m_tag;
        }

        bool operator==(const TagReference& lhs, const TagReference& rhs) {
            return lhs.m_tag == rhs.m_tag;
        }

        bool operator<(const TagReference& lhs, const TagReference& rhs) {
            return lhs.m_tag < rhs.m_tag;
        }

        Taggable::Taggable() :
        m_tagMask(0) {}

        Taggable::~Taggable() = default;

        bool Taggable::hasTag(const Tag& tag) const {
            return hasTag(tag.type());
        }

        bool Taggable::hasTag(Tag::TagType mask) const {
            return (m_tagMask & mask) != 0;
        }

        Tag::TagType Taggable::tagMask() const {
            return m_tagMask;
        }

        bool Taggable::addTag(const Tag& tag) {
            if (hasTag(tag)) {
                return false;
            } else {
                m_tagMask |= tag.type();
                m_tags.emplace(tag);

                updateAttributeMask();
                return true;
            }
        }

        bool Taggable::removeTag(const Tag& tag) {
            if (!hasTag(tag)) {
                return false;
            } else {
                m_tagMask &= ~tag.type();
                auto it = m_tags.find(TagReference(tag));
                assert(it != std::end(m_tags));
                m_tags.erase(it);

                assert(!hasTag(tag));

                updateAttributeMask();

                return true;
            }
        }

        void Taggable::initializeTags(TagManager& tagManager) {
            clearTags();
            updateTags(tagManager);
        }

        void Taggable::updateTags(TagManager& tagManager) {
            tagManager.updateTags(*this);
            updateAttributeMask();
        }

        void Taggable::clearTags() {
            m_tagMask = 0;
            m_tags.clear();
            updateAttributeMask();
        }

        bool Taggable::hasAttribute(const TagAttribute& attribute) const {
            return (m_attributeMask & attribute.type()) != 0;
        }

        void Taggable::accept(TagVisitor& visitor) {
            doAcceptTagVisitor(visitor);
        }

        void Taggable::accept(ConstTagVisitor& visitor) const {
            doAcceptTagVisitor(visitor);
        }

        void Taggable::updateAttributeMask() {
            m_attributeMask = 0;
            for (const auto& tagRef : m_tags) {
                const auto& tag = tagRef.tag();
                for (const auto& attribute : tag.attributes()) {
                    m_attributeMask |= attribute.type();
                }
            }
        }

        TagMatcherCallback::~TagMatcherCallback() = default;

        TagMatcher::~TagMatcher() = default;

        void TagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {}
        void TagMatcher::disable(TagMatcherCallback& callback, MapFacade& facade) const {}

        bool TagMatcher::canEnable(MapFacade& facade) const {
            return false;
        }

        bool TagMatcher::canDisable(MapFacade& facade) const {
            return false;
        }

        SmartTag::SmartTag(String name, std::vector<TagAttribute> attributes, std::unique_ptr<TagMatcher> matcher) :
        Tag(name, attributes),
        m_matcher(std::move(matcher)) {}

        SmartTag::SmartTag(const SmartTag& other) :
        Tag(other.m_index, other.m_name, other.m_attributes),
        m_matcher(other.m_matcher->clone()) {}

        SmartTag::SmartTag(SmartTag&& other) noexcept = default;

        SmartTag& SmartTag::operator=(const SmartTag& other) {
            m_index = other.m_index;
            m_name = other.m_name;
            m_attributes = other.m_attributes;
            m_matcher = other.m_matcher->clone();
            return *this;
        }

        SmartTag& SmartTag::operator=(SmartTag&& other) = default;

        void SmartTag::update(Taggable& taggable) const {
            if (m_matcher->matches(taggable)) {
                taggable.addTag(*this);
            } else {
                taggable.removeTag(*this);
            }
        }

        void SmartTag::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            m_matcher->enable(callback, facade);
        }

        void SmartTag::disable(TagMatcherCallback& callback, MapFacade& facade) const {
            m_matcher->disable(callback, facade);
        }

        bool SmartTag::canEnable(MapFacade& facade) const {
            return m_matcher->canEnable(facade);
        }

        bool SmartTag::canDisable(MapFacade& facade) const {
            return m_matcher->canDisable(facade);
        }
    }
}
