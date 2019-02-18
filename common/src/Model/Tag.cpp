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

#include "Model/TagManager.h"

namespace TrenchBroom {
    namespace Model {
        TagAttribute::TagAttribute(String name) :
        m_name(std::move(name) ){}

        const String& TagAttribute::name() const {
            return m_name;
        }

        bool operator==(const TagAttribute& lhs, const TagAttribute& rhs) {
            return lhs.m_name == rhs.m_name;
        }

        bool operator<(const TagAttribute& lhs, const TagAttribute& rhs) {
            return lhs.m_name < rhs.m_name;
        }

        Tag::TagType Tag::freeTagType() {
            static const size_t Bits = (sizeof(TagType) * 8);
            static size_t currentShift = 0;

            ensure(currentShift <= Bits, "No more tag types");
            return 1u << currentShift++;
        }

        Tag::Tag(Tag::TagType type, String name, std::set<TagAttribute> attributes) :
        m_type(type),
        m_name(std::move(name)),
        // m_attributes(std::move(attributes))
        {}

        Tag::Tag(String name, std::set<TagAttribute> attributes) :
        Tag(freeTagType(), name, attributes) {}

        Tag::TagType Tag::type() const {
            return m_type;
        }

        const String& Tag::name() const {
            return m_name;
        }

        bool operator==(const Tag& lhs, const Tag& rhs) {
            return lhs.m_name == rhs.m_name;
        }

        bool operator<(const Tag& lhs, const Tag& rhs) {
            return lhs.m_name < rhs.m_name;
        }

        TagReference::TagReference(const Tag& tag) :
        m_tag(tag) {}

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
            return (m_tagMask & tag.type()) != 0;
        }

        bool Taggable::addTag(const Tag& tag) {
            if (hasTag(tag)) {
                return false;
            } else {
                m_tagMask |= tag.type();
                m_tags.emplace(tag);
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

                return true;
            }
        }

        void Taggable::initializeTags(TagManager& tagManager) {
            clearTags();
            updateTags(tagManager);
        }

        void Taggable::updateTags(TagManager& tagManager) {
            tagManager.updateTags(*this);
        }

        void Taggable::clearTags() {
            m_tagMask = 0;
            m_tags.clear();
        }

        bool Taggable::evaluateTagMatcher(const TagMatcher& matcher) const {
            return doEvaluateTagMatcher(matcher);
        }

        TagMatcher::~TagMatcher() = default;

        bool TagMatcher::matches(const Taggable& taggable) const {
            return taggable.evaluateTagMatcher(*this);
        }

        bool TagMatcher::matches(const BrushFace& face) const {
            return false;
        }

        bool TagMatcher::matches(const World& world) const {
            return false;
        }

        bool TagMatcher::matches(const Layer& layer) const {
            return false;
        }

        bool TagMatcher::matches(const Group& group) const {
            return false;
        }

        bool TagMatcher::matches(const Entity& entity) const {
            return false;
        }

        bool TagMatcher::matches(const Brush& brush) const {
            return false;
        }

        SmartTag::SmartTag(String name, std::set<TagAttribute> attributes, std::unique_ptr<TagMatcher> matcher) :
        Tag(name, attributes),
        m_matcher(std::move(matcher)) {}

        SmartTag::SmartTag(const SmartTag& other) :
        Tag(other.m_type, other.m_name, other.m_attributes),
        m_matcher(other.m_matcher->clone()) {}

        SmartTag::SmartTag(SmartTag&& other) noexcept = default;

        SmartTag& SmartTag::operator=(const SmartTag& other) {
            m_type = other.m_type;
            m_name = other.m_name;
            m_attributes = other.m_attributes;
            m_matcher = other.m_matcher->clone();
            return *this;
        }

        SmartTag& SmartTag::operator=(SmartTag&& other) noexcept = default;

        void SmartTag::update(Taggable& taggable) const {
            if (m_matcher->matches(taggable)) {
                taggable.addTag(*this);
            } else {
                taggable.removeTag(*this);
            }
        }
    }
}
