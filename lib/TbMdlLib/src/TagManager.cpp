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
namespace
{

struct TagCmp
{
  using is_transparent = void;

  bool operator()(const SmartTag& lhs, const SmartTag& rhs) const
  {
    return lhs.name() < rhs.name();
  }

  bool operator()(const std::string& lhs, const SmartTag& rhs) const
  {
    return lhs < rhs.name();
  }

  bool operator()(const SmartTag& lhs, const std::string& rhs) const
  {
    return lhs.name() < rhs;
  }

  bool operator()(const std::string& lhs, const std::string& rhs) const
  {
    return lhs < rhs;
  }
};

size_t freeTagIndex(const size_t count)
{
  static const size_t Bits = (sizeof(TagType::Type) * 8);
  const auto index = count;
  contract_assert(index <= Bits);

  return index;
}

} // namespace

const std::vector<SmartTag>& TagManager::smartTags() const
{
  return m_smartTags;
}

bool TagManager::isRegisteredSmartTag(const std::string& name) const
{
  const auto iTag = std::ranges::lower_bound(m_smartTags, name, TagCmp{});
  return iTag != m_smartTags.end() && iTag->name() == name;
}

const SmartTag& TagManager::smartTag(const std::string& name) const
{
  if (const auto iTag = std::ranges::lower_bound(m_smartTags, name, TagCmp{});
      iTag != m_smartTags.end() && iTag->name() == name)
  {
    return *iTag;
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
  auto newTags = kdl::flat_set<SmartTag, TagCmp>{};
  for (const auto& tag : tags)
  {
    auto indexedTag = tag;
    indexedTag.setIndex(freeTagIndex(newTags.size()));

    if (!newTags.insert(std::move(indexedTag)).second)
    {
      throw std::logic_error{
        fmt::format("Smart tag '{}' already registered", tag.name())};
    }
  }

  m_smartTags = newTags.extract();
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

} // namespace tb::mdl
