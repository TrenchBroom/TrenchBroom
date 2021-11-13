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

#include <kdl/compact_trie_forward.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
class EntityNodeBase;
class EntityProperty;

using EntityNodeStringIndex = kdl::compact_trie<EntityNodeBase*>;

class EntityNodeIndexQuery {
public:
  typedef enum
  {
    Type_Exact,
    Type_Prefix,
    Type_Numbered,
    Type_Any
  } Type;

private:
  Type m_type;
  std::string m_pattern;

public:
  static EntityNodeIndexQuery exact(const std::string& pattern);
  static EntityNodeIndexQuery prefix(const std::string& pattern);
  static EntityNodeIndexQuery numbered(const std::string& pattern);
  static EntityNodeIndexQuery any();

  std::set<EntityNodeBase*> execute(const EntityNodeStringIndex& index) const;
  bool execute(const EntityNodeBase* node, const std::string& value) const;
  std::vector<Model::EntityProperty> execute(const EntityNodeBase* node) const;

private:
  explicit EntityNodeIndexQuery(Type type, const std::string& pattern = "");
};

class EntityNodeIndex {
private:
  std::unique_ptr<EntityNodeStringIndex> m_keyIndex;
  std::unique_ptr<EntityNodeStringIndex> m_valueIndex;

public:
  EntityNodeIndex();
  ~EntityNodeIndex();

  void addEntityNode(EntityNodeBase* node);
  void removeEntityNode(EntityNodeBase* node);

  void addProperty(EntityNodeBase* node, const std::string& key, const std::string& value);
  void removeProperty(EntityNodeBase* node, const std::string& key, const std::string& value);

  std::vector<EntityNodeBase*> findEntityNodes(
    const EntityNodeIndexQuery& keyQuery, const std::string& value) const;
  std::vector<std::string> allKeys() const;
  std::vector<std::string> allValuesForKeys(const EntityNodeIndexQuery& keyQuery) const;
};
} // namespace Model
} // namespace TrenchBroom
