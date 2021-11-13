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

#include <kdl/string_utils.h>

#include <cassert>
#include <ostream>
#include <string>
#include <utility>

namespace TrenchBroom {
namespace Model {
TagAttribute::TagAttribute(const AttributeType type, const std::string& name)
  : m_type(type)
  , m_name(name) {}

TagAttribute::AttributeType TagAttribute::type() const {
  return m_type;
}

const std::string& TagAttribute::name() const {
  return m_name;
}

bool operator==(const TagAttribute& lhs, const TagAttribute& rhs) {
  return lhs.m_name == rhs.m_name;
}

bool operator!=(const TagAttribute& lhs, const TagAttribute& rhs) {
  return !(lhs == rhs);
}

bool operator<(const TagAttribute& lhs, const TagAttribute& rhs) {
  return lhs.m_name < rhs.m_name;
}

std::ostream& operator<<(std::ostream& str, const TagAttribute& attr) {
  return str << "TagAttribute{type: " << attr.m_type << ", name: " << attr.m_name << "}";
}

Tag::Tag(const size_t index, const std::string& name, std::vector<TagAttribute> attributes)
  : m_index(index)
  , m_name(name)
  , m_attributes(std::move(attributes)) {}

Tag::Tag(const std::string& name, std::vector<TagAttribute> attributes)
  : Tag(0, name, std::move(attributes)) {}

Tag::~Tag() = default;

Tag::Tag(const Tag& other) = default;
Tag::Tag(Tag&& other) noexcept = default;

Tag& Tag::operator=(const Tag& other) = default;
Tag& Tag::operator=(Tag&& other) = default;

TagType::Type Tag::type() const {
  return TagType::Type(1) << m_index;
}

size_t Tag::index() const {
  return m_index;
}

void Tag::setIndex(const size_t index) {
  m_index = index;
}

const std::string& Tag::name() const {
  return m_name;
}

const std::vector<TagAttribute>& Tag::attributes() const {
  return m_attributes;
}

bool operator==(const Tag& lhs, const Tag& rhs) {
  return lhs.m_name == rhs.m_name;
}

bool operator!=(const Tag& lhs, const Tag& rhs) {
  return !(lhs == rhs);
}

bool operator<(const Tag& lhs, const Tag& rhs) {
  return lhs.m_name < rhs.m_name;
}

void Tag::appendToStream(std::ostream& str) const {
  str << "Tag{"
      << "index: " << m_index << ", "
      << "name: " << m_name << ", "
      << "attributes: " << kdl::str_join(m_attributes) << "}";
}

std::ostream& operator<<(std::ostream& str, const Tag& tag) {
  tag.appendToStream(str);
  return str;
}

TagReference::TagReference(const Tag& tag)
  : m_tag(&tag) {}

const Tag& TagReference::tag() const {
  return *m_tag;
}

bool operator==(const TagReference& lhs, const TagReference& rhs) {
  return *(lhs.m_tag) == *(rhs.m_tag);
}

bool operator!=(const TagReference& lhs, const TagReference& rhs) {
  return !(lhs == rhs);
}

bool operator<(const TagReference& lhs, const TagReference& rhs) {
  return *(lhs.m_tag) < *(rhs.m_tag);
}

Taggable::Taggable()
  : m_tagMask(0)
  , m_attributeMask(0) {}

void swap(Taggable& lhs, Taggable& rhs) noexcept {
  using std::swap;
  swap(lhs.m_tagMask, rhs.m_tagMask);
  swap(lhs.m_tags, rhs.m_tags);
  swap(lhs.m_attributeMask, rhs.m_attributeMask);
}

Taggable::~Taggable() = default;

bool Taggable::hasAnyTag() const {
  return m_tagMask != 0;
}

bool Taggable::hasTag(const Tag& tag) const {
  return hasTag(tag.type());
}

bool Taggable::hasTag(TagType::Type mask) const {
  return (m_tagMask & mask) != 0;
}

TagType::Type Taggable::tagMask() const {
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
  const auto it = m_tags.find(TagReference(tag));
  if (it == std::end(m_tags)) {
    return false;
  }

  m_tagMask &= ~tag.type();
  m_tags.erase(it);
  assert(!hasTag(tag));

  updateAttributeMask();
  return true;
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

void TagMatcher::enable(TagMatcherCallback& /* callback */, MapFacade& /* facade */) const {}
void TagMatcher::disable(TagMatcherCallback& /* callback */, MapFacade& /* facade */) const {}

bool TagMatcher::canEnable() const {
  return false;
}

bool TagMatcher::canDisable() const {
  return false;
}

std::ostream& operator<<(std::ostream& str, const TagMatcher& matcher) {
  matcher.appendToStream(str);
  return str;
}

SmartTag::SmartTag(
  const std::string& name, std::vector<TagAttribute> attributes,
  std::unique_ptr<TagMatcher> matcher)
  : Tag(name, std::move(attributes))
  , m_matcher(std::move(matcher)) {}

SmartTag::SmartTag(const SmartTag& other)
  : Tag(other.m_index, other.m_name, other.m_attributes)
  , m_matcher(other.m_matcher->clone()) {}

SmartTag::SmartTag(SmartTag&& other) noexcept = default;

SmartTag& SmartTag::operator=(const SmartTag& other) {
  m_index = other.m_index;
  m_name = other.m_name;
  m_attributes = other.m_attributes;
  m_matcher = other.m_matcher->clone();
  return *this;
}

SmartTag& SmartTag::operator=(SmartTag&& other) = default;

bool SmartTag::matches(const Taggable& taggable) const {
  return m_matcher->matches(taggable);
}

void SmartTag::update(Taggable& taggable) const {
  if (matches(taggable)) {
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

bool SmartTag::canEnable() const {
  return m_matcher->canEnable();
}

bool SmartTag::canDisable() const {
  return m_matcher->canDisable();
}

void SmartTag::appendToStream(std::ostream& str) const {
  str << "SmartTag{"
      << "index: " << m_index << ", "
      << "name: " << m_name << ", "
      << "attributes: " << kdl::str_join(m_attributes) << ", "
      << "matcher: " << *m_matcher << "}";
}
} // namespace Model
} // namespace TrenchBroom
