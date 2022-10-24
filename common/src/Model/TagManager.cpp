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

#include "Ensure.h"
#include "Model/Tag.h"
#include "Model/TagType.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace TrenchBroom
{
namespace Model
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
  const auto it = m_smartTags.find(name);
  if (it == std::end(m_smartTags))
  {
    throw std::logic_error("Smart tag not registered");
  }
  return *it;
}

bool TagManager::isRegisteredSmartTag(const size_t index) const
{
  for (const auto& tag : m_smartTags)
  {
    if (tag.index() == index)
    {
      return true;
    }
  }
  return false;
}

const SmartTag& TagManager::smartTag(const size_t index) const
{
  for (const auto& tag : m_smartTags)
  {
    if (tag.index() == index)
    {
      return tag;
    }
  }
  throw std::logic_error("Smart tag not registered");
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
      throw std::logic_error("Smart tag '" + tag.name() + "' already registered");
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
  ensure(index <= Bits, "no more tag types");
  return index;
}
} // namespace Model
} // namespace TrenchBroom
