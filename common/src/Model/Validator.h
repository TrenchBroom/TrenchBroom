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

#include "Model/IssueType.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
class EntityNodeBase;
class BrushNode;
class EntityNode;
class GroupNode;
class Issue;
class IssueQuickFix;
class LayerNode;
class Node;
class PatchNode;
class WorldNode;

class Validator {
private:
  IssueType m_type;
  std::string m_description;
  std::vector<std::unique_ptr<IssueQuickFix>> m_quickFixes;

public:
  virtual ~Validator();

  IssueType type() const;
  const std::string& description() const;
  std::vector<const IssueQuickFix*> quickFixes() const;

  void validate(Node& node, std::vector<std::unique_ptr<Issue>>& issues) const;

protected:
  Validator(IssueType type, const std::string& description);
  void addQuickFix(std::unique_ptr<IssueQuickFix> quickFix);

private:
  virtual void doValidate(WorldNode& worldNode, std::vector<std::unique_ptr<Issue>>& issues) const;
  virtual void doValidate(LayerNode& layerNode, std::vector<std::unique_ptr<Issue>>& issues) const;
  virtual void doValidate(GroupNode& groupNode, std::vector<std::unique_ptr<Issue>>& issues) const;
  virtual void doValidate(
    EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const;
  virtual void doValidate(BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const;
  virtual void doValidate(PatchNode& patchNode, std::vector<std::unique_ptr<Issue>>& issues) const;
  virtual void doValidate(EntityNodeBase& node, std::vector<std::unique_ptr<Issue>>& issues) const;
};
} // namespace Model
} // namespace TrenchBroom
