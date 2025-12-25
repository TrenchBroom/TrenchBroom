/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/TagManager.h"

#include "mdl/Tag.h"
#include "mdl/TagType.h"

#include "kd/contracts.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace tb::mdl
{

bool TagManager::TagCmp::operator()(const SmartTag& lhs, const SmartTag& rhs) const
{
  return lhs.name() < rhs.name();
}

bool TagManager::TagCmp::operator()(const std::string& lhs, const SmartTag& rhs) const
{
  return lhs < rhs.name();
}

bool TagManager::TagCmp::operator()(const SmartTag& lhs, const std::string& rhs) const
{
  return lhs.name() < rhs;
}

bool TagManager::TagCmp::operator()(const std::string& lhs, const std::string& rhs) const
{
  return lhs < rhs;
}

const std::vector<SmartTag>& TagManager::smartTags() const
{
  return m_smartTags.get_data();
}

bool TagManager::isRegisteredSmartTag(const std::string& name) const
{
  return m_smartTags.count(name) > 0u;
}

const SmartTag& TagManager::smartTag(const std::string& name) const
{
  if (const auto it = m_smartTags.find(name); it != std::end(m_smartTags))
  {
    return *it;
  }
  throw std::logic_error{"Smart tag not registered"};
}

bool TagManager::isRegisteredSmartTag(const size_t index) const
{
  return std::ranges::any_of(
    m_smartTags, [&](const auto& tag) { return tag.index() == index; });
}

const SmartTag& TagManager::smartTag(const size_t index) const
{
  if (const auto it = std::ranges::find_if(
        m_smartTags, [&](const auto& tag) { return tag.index() == index; });
      it != std::end(m_smartTags))
  {
    return *it;
  }
  throw std::logic_error{"Smart tag not registered"};
}

void TagManager::registerSmartTags(const std::vector<SmartTag>& tags)
{
  m_smartTags = kdl::vector_set<SmartTag, TagCmp>(tags.size());
  for (const auto& tag : tags)
  {
    const size_t nextIndex = freeTagIndex();
    auto [it, inserted] = m_smartTags.insert(tag);

    if (!inserted)
    {
      throw std::logic_error{
        fmt::format("Smart tag '{}' already registered", tag.name())};
    }

    it->setIndex(nextIndex);
  }
}

void TagManager::clearSmartTags()
{
  m_smartTags.clear();
}

void TagManager::updateTags(Taggable& taggable) const
{
  for (const auto& tag : m_smartTags)
  {
    tag.update(taggable);
  }
}

size_t TagManager::freeTagIndex()
{
  static const size_t Bits = (sizeof(TagType::Type) * 8);
  const auto index = m_smartTags.size();
  contract_assert(index <= Bits);

  return index;
}

} // namespace tb::mdl
