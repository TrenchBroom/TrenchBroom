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

#include "EntityNodeBase.h"

#include "mdl/EntityDefinition.h"
#include "mdl/PropertyDefinition.h"

#include "kdl/ranges/to.h"

#include <ranges>
#include <string>
#include <vector>

namespace tb::mdl
{

namespace
{
template <typename T, typename Eq = std::equal_to<T*>>
const auto* select(const std::vector<T*> ts, const Eq& eq = Eq{})
{
  if (ts.empty())
  {
    return static_cast<T*>(nullptr);
  }

  const auto* result = ts.front();
  for (auto it = std::next(ts.begin()); it != ts.end(); ++it)
  {
    const auto* t = *it;
    if (!eq(result, t))
    {
      return static_cast<T*>(nullptr);
    }
  }
  return result;
}
} // namespace

const EntityDefinition* selectEntityDefinition(const std::vector<EntityNodeBase*>& nodes)
{
  return select(
    nodes
    | std::views::transform([](const auto* node) { return node->entity().definition(); })
    | kdl::ranges::to<std::vector>());
}

const PropertyDefinition* propertyDefinition(
  const EntityNodeBase* node, const std::string& key)
{
  const auto* definition = node->entity().definition();
  return definition ? getPropertyDefinition(definition, key) : nullptr;
}

const PropertyDefinition* selectPropertyDefinition(
  const std::string& key, const std::vector<EntityNodeBase*>& nodes)
{
  return select(
    nodes | std::views::transform([&](const auto* node) {
      return propertyDefinition(node, key);
    })
    | kdl::ranges::to<std::vector>());
}

std::string selectPropertyValue(
  const std::string& key, const std::vector<EntityNodeBase*>& nodes)
{
  const auto* value = select(
    nodes | std::views::transform([&](const auto* node) {
      return node->entity().property(key);
    }) | kdl::ranges::to<std::vector>(),
    [](const auto* lhs, const auto* rhs) {
      return (lhs == nullptr && rhs == nullptr)
             || (lhs != nullptr && rhs != nullptr && *lhs == *rhs);
    });

  return value ? *value : "";
}

EntityNodeBase::EntityNodeBase(Entity entity)
  : m_entity{std::move(entity)}
{
}

EntityNodeBase::~EntityNodeBase() = default;

const Entity& EntityNodeBase::entity() const
{
  return m_entity;
}

Entity EntityNodeBase::setEntity(Entity entity)
{
  const auto notifyChange = NotifyPropertyChange{*this};
  return std::exchange(m_entity, std::move(entity));
}

void EntityNodeBase::setDefinition(const EntityDefinition* definition)
{
  if (m_entity.definition() == definition)
  {
    return;
  }

  const auto notifyChange = NotifyPropertyChange{*this};
  m_entity.setDefinition(definition);
}

EntityNodeBase::NotifyPropertyChange::NotifyPropertyChange(EntityNodeBase& node)
  : m_nodeChange{node}
  , m_node{node}
  , m_oldPhysicalBounds{node.physicalBounds()}
{
  m_node.propertiesWillChange();
}

EntityNodeBase::NotifyPropertyChange::~NotifyPropertyChange()
{
  m_node.propertiesDidChange(m_oldPhysicalBounds);
}

void EntityNodeBase::propertiesWillChange() {}

void EntityNodeBase::propertiesDidChange(const vm::bbox3d& oldPhysicalBounds)
{
  doPropertiesDidChange(oldPhysicalBounds);
}

vm::vec3d EntityNodeBase::linkSourceAnchor() const
{
  return doGetLinkSourceAnchor();
}

vm::vec3d EntityNodeBase::linkTargetAnchor() const
{
  return doGetLinkTargetAnchor();
}

EntityNodeBase::EntityNodeBase() = default;

const std::string& EntityNodeBase::doGetName() const
{
  return m_entity.classname();
}

bool operator==(const EntityNodeBase& lhs, const EntityNodeBase& rhs)
{
  return lhs.entity() == rhs.entity();
}

bool operator!=(const EntityNodeBase& lhs, const EntityNodeBase& rhs)
{
  return !(lhs == rhs);
}

} // namespace tb::mdl
