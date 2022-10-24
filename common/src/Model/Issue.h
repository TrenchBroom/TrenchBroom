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

#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class BrushFace;
class BrushNode;
class EntityNodeBase;
class Node;

class Issue
{
protected:
  size_t m_seqId;
  IssueType m_type;
  Node& m_node;
  std::string m_description;

public:
  explicit Issue(IssueType type, Node& node, std::string description);
  virtual ~Issue();

  size_t seqId() const;
  size_t lineNumber() const;
  const std::string& description() const;

  IssueType type() const;
  Node& node() const;

  bool addSelectableNodes(std::vector<Model::Node*>& nodes) const;

  bool hidden() const;

protected:
  static size_t nextSeqId();

private: // subclassing interface
  virtual size_t doGetLineNumber() const;
};

class BrushFaceIssue : public Issue
{
private:
  size_t m_faceIndex;

public:
  BrushFaceIssue(
    IssueType type, BrushNode& node, size_t faceIndex, std::string description);
  ~BrushFaceIssue() override;

  size_t faceIndex() const;
  const BrushFace& face() const;

private:
  size_t doGetLineNumber() const override;
};

class EntityPropertyIssue : public Issue
{
private:
  std::string m_propertyKey;

public:
  EntityPropertyIssue(
    IssueType type,
    EntityNodeBase& entityNode,
    std::string propertyKey,
    std::string description);
  ~EntityPropertyIssue() override;

  const std::string& propertyKey() const;
  const std::string& propertyValue() const;
};
} // namespace Model
} // namespace TrenchBroom
