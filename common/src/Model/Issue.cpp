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

#include "Model/Issue.h"

#include "Ensure.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Node.h"
#include "Model/PatchNode.h"

#include <kdl/overload.h>
#include <kdl/vector_utils.h>

#include <string>

namespace TrenchBroom
{
namespace Model
{
Issue::Issue(const IssueType type, Node& node, std::string description)
  : m_seqId{nextSeqId()}
  , m_type{type}
  , m_node{node}
  , m_description{std::move(description)}
{
}

Issue::~Issue() = default;

size_t Issue::seqId() const
{
  return m_seqId;
}

size_t Issue::lineNumber() const
{
  return doGetLineNumber();
}

const std::string& Issue::description() const
{
  return m_description;
}

IssueType Issue::type() const
{
  return m_type;
}

Node& Issue::node() const
{
  return m_node;
}

bool Issue::addSelectableNodes(std::vector<Model::Node*>& nodes) const
{
  if (m_node.parent() == nullptr)
  {
    return false;
  }

  m_node.accept(kdl::overload(
    [](WorldNode*) {},
    [](LayerNode*) {},
    [&](GroupNode* group) { nodes.push_back(group); },
    [&](auto&& thisLambda, EntityNode* entity) {
      if (!entity->hasChildren())
      {
        nodes.push_back(entity);
      }
      else
      {
        entity->visitChildren(thisLambda);
      }
    },
    [&](BrushNode* brush) { nodes.push_back(brush); },
    [&](PatchNode* patch) { nodes.push_back(patch); }));

  return true;
}

bool Issue::hidden() const
{
  return m_node.issueHidden(type());
}

size_t Issue::nextSeqId()
{
  static size_t seqId = 0;
  return seqId++;
}

size_t Issue::doGetLineNumber() const
{
  return m_node.lineNumber();
}

BrushFaceIssue::BrushFaceIssue(
  const IssueType type, BrushNode& node, const size_t faceIndex, std::string description)
  : Issue{type, node, std::move(description)}
  , m_faceIndex{faceIndex}
{
}

BrushFaceIssue::~BrushFaceIssue() = default;

size_t BrushFaceIssue::faceIndex() const
{
  return m_faceIndex;
}

const BrushFace& BrushFaceIssue::face() const
{
  const auto& brushNode = static_cast<const BrushNode&>(node());
  const auto& brush = brushNode.brush();
  return brush.face(m_faceIndex);
}

size_t BrushFaceIssue::doGetLineNumber() const
{
  return face().lineNumber();
}

EntityPropertyIssue::EntityPropertyIssue(
  const IssueType type,
  EntityNodeBase& entityNode,
  std::string propertyKey,
  std::string description)
  : Issue{type, entityNode, std::move(description)}
  , m_propertyKey{std::move(propertyKey)}
{
}

EntityPropertyIssue::~EntityPropertyIssue() = default;

const std::string& EntityPropertyIssue::propertyKey() const
{
  return m_propertyKey;
}

const std::string& EntityPropertyIssue::propertyValue() const
{
  static const auto NoValue = std::string{};
  const auto& entityNode = static_cast<EntityNodeBase&>(node());
  const auto* value = entityNode.entity().property(propertyKey());
  return value ? *value : NoValue;
}
} // namespace Model
} // namespace TrenchBroom
