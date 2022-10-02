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

#include "Validator.h"

#include "Ensure.h"
#include "Model/EntityNode.h"
#include "Model/IssueQuickFix.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/vector_utils.h>

#include <cassert>
#include <string>

namespace TrenchBroom {
namespace Model {
Validator::~Validator() = default;

IssueType Validator::type() const {
  return m_type;
}

const std::string& Validator::description() const {
  return m_description;
}

std::vector<const IssueQuickFix*> Validator::quickFixes() const {
  return kdl::vec_transform(m_quickFixes, [](const auto& quickFix) {
    return const_cast<const IssueQuickFix*>(quickFix.get());
  });
}

void Validator::validate(Node& node, std::vector<std::unique_ptr<Issue>>& issues) const {
  node.accept(kdl::overload(
    [&](WorldNode* worldNode) {
      doValidate(*worldNode, issues);
    },
    [&](LayerNode* layerNode) {
      doValidate(*layerNode, issues);
    },
    [&](GroupNode* groupNode) {
      doValidate(*groupNode, issues);
    },
    [&](EntityNode* entityNode) {
      doValidate(*entityNode, issues);
    },
    [&](BrushNode* brushNode) {
      doValidate(*brushNode, issues);
    },
    [&](PatchNode* patchNode) {
      doValidate(*patchNode, issues);
    }));
}

Validator::Validator(const IssueType type, const std::string& description)
  : m_type(type)
  , m_description(description) {}

void Validator::addQuickFix(std::unique_ptr<IssueQuickFix> quickFix) {
  ensure(quickFix != nullptr, "quickFix is null");
  assert(!kdl::vec_contains(m_quickFixes, quickFix));
  m_quickFixes.push_back(std::move(quickFix));
}

void Validator::doValidate(
  WorldNode& worldNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  doValidate(static_cast<EntityNodeBase&>(worldNode), issues);
}
void Validator::doValidate(LayerNode&, std::vector<std::unique_ptr<Issue>>&) const {}
void Validator::doValidate(GroupNode&, std::vector<std::unique_ptr<Issue>>&) const {}
void Validator::doValidate(
  EntityNode& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  doValidate(static_cast<EntityNodeBase&>(entityNode), issues);
}
void Validator::doValidate(BrushNode&, std::vector<std::unique_ptr<Issue>>&) const {}
void Validator::doValidate(PatchNode&, std::vector<std::unique_ptr<Issue>>&) const {}
void Validator::doValidate(EntityNodeBase&, std::vector<std::unique_ptr<Issue>>&) const {}
} // namespace Model
} // namespace TrenchBroom
