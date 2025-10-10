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

#pragma once

#include "mdl/Entity.h"
#include "mdl/Node.h"

#include "vm/bbox.h"

#include <string>
#include <vector>

namespace tb::mdl
{
struct PropertyDefinition;

const EntityDefinition* selectEntityDefinition(const std::vector<EntityNodeBase*>& nodes);
const PropertyDefinition* propertyDefinition(
  const EntityNodeBase* node, const std::string& key);
const PropertyDefinition* selectPropertyDefinition(
  const std::string& key, const std::vector<EntityNodeBase*>& nodes);
std::string selectPropertyValue(
  const std::string& key, const std::vector<EntityNodeBase*>& nodes);

class EntityNodeBase : public Node
{
protected:
  explicit EntityNodeBase(Entity entity);

  Entity m_entity;

public:
  ~EntityNodeBase() override;

public: // entity access
  const Entity& entity() const;
  Entity setEntity(Entity entity);

public: // definition
  void setDefinition(const EntityDefinition* definition);

private: // property management internals
  class NotifyPropertyChange
  {
  private:
    NotifyNodeChange m_nodeChange;
    EntityNodeBase& m_node;
    vm::bbox3d m_oldPhysicalBounds;

  public:
    explicit NotifyPropertyChange(EntityNodeBase& node);
    ~NotifyPropertyChange();
  };

  void propertiesWillChange();
  void propertiesDidChange(const vm::bbox3d& oldPhysicalBounds);

public: // Entity linking
  vm::vec3d linkSourceAnchor() const;
  vm::vec3d linkTargetAnchor() const;

protected:
  EntityNodeBase();

private: // implemenation of node interface
  const std::string& doGetName() const override;

private: // subclassing interface
  virtual void doPropertiesDidChange(const vm::bbox3d& oldBounds) = 0;
  virtual vm::vec3d doGetLinkSourceAnchor() const = 0;
  virtual vm::vec3d doGetLinkTargetAnchor() const = 0;

private: // hide copy constructor and assignment operator
  EntityNodeBase(const EntityNodeBase&);
  EntityNodeBase& operator=(const EntityNodeBase&);
};

bool operator==(const EntityNodeBase& lhs, const EntityNodeBase& rhs);
bool operator!=(const EntityNodeBase& lhs, const EntityNodeBase& rhs);

} // namespace tb::mdl
