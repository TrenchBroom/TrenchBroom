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
class WorldNode;

class Validator {
protected:
  using IssueList = std::vector<Issue*>;

private:
  IssueType m_type;
  std::string m_description;
  std::vector<std::unique_ptr<IssueQuickFix>> m_quickFixes;

public:
  virtual ~Validator();

  IssueType type() const;
  const std::string& description() const;
  std::vector<const IssueQuickFix*> quickFixes() const;

  void validate(WorldNode* worldNode, IssueList& issues) const;
  void validate(LayerNode* layerNode, IssueList& issues) const;
  void validate(GroupNode* groupNode, IssueList& issues) const;
  void validate(EntityNode* entityNode, IssueList& issues) const;
  void validate(BrushNode* brushNode, IssueList& issues) const;

protected:
  Validator(IssueType type, const std::string& description);
  void addQuickFix(std::unique_ptr<IssueQuickFix> quickFix);

private:
  virtual void doValidate(WorldNode* worldNode, IssueList& issues) const;
  virtual void doValidate(LayerNode* layerNode, IssueList& issues) const;
  virtual void doValidate(GroupNode* groupNode, IssueList& issues) const;
  virtual void doValidate(EntityNode* entityNode, IssueList& issues) const;
  virtual void doValidate(BrushNode* brushNode, IssueList& issues) const;
  virtual void doValidate(EntityNodeBase* node, IssueList& issues) const;
};
} // namespace Model
} // namespace TrenchBroom
