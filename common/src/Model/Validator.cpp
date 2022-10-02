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

#include <kdl/vector_utils.h>

#include <cassert>
#include <string>

namespace TrenchBroom {
namespace Model {
Validator::~Validator() {
  kdl::vec_clear_and_delete(m_quickFixes);
}

IssueType Validator::type() const {
  return m_type;
}

const std::string& Validator::description() const {
  return m_description;
}

const std::vector<IssueQuickFix*>& Validator::quickFixes() const {
  return m_quickFixes;
}

void Validator::validate(WorldNode* worldNode, IssueList& issues) const {
  doValidate(worldNode, issues);
}

void Validator::validate(LayerNode* layerNode, IssueList& issues) const {
  doValidate(layerNode, issues);
}

void Validator::validate(GroupNode* groupNode, IssueList& issues) const {
  doValidate(groupNode, issues);
}

void Validator::validate(EntityNode* entityNode, IssueList& issues) const {
  doValidate(entityNode, issues);
}

void Validator::validate(BrushNode* brushNode, IssueList& issues) const {
  doValidate(brushNode, issues);
}

Validator::Validator(const IssueType type, const std::string& description)
  : m_type(type)
  , m_description(description) {}

void Validator::addQuickFix(IssueQuickFix* quickFix) {
  ensure(quickFix != nullptr, "quickFix is null");
  assert(!kdl::vec_contains(m_quickFixes, quickFix));
  m_quickFixes.push_back(quickFix);
}

void Validator::doValidate(WorldNode* worldNode, IssueList& issues) const {
  doValidate(static_cast<EntityNodeBase*>(worldNode), issues);
}
void Validator::doValidate(LayerNode*, IssueList&) const {}
void Validator::doValidate(GroupNode*, IssueList&) const {}
void Validator::doValidate(EntityNode* entityNode, IssueList& issues) const {
  doValidate(static_cast<EntityNodeBase*>(entityNode), issues);
}
void Validator::doValidate(BrushNode*, IssueList&) const {}
void Validator::doValidate(EntityNodeBase*, IssueList&) const {}
} // namespace Model
} // namespace TrenchBroom
