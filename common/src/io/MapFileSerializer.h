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

#include "io/NodeSerializer.h"
#include "mdl/MapFormat.h"

#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <vector>


namespace tb::mdl
{
class BezierPatch;
class Brush;
class BrushNode;
class BrushFace;
class EntityProperty;
class Node;
class PatchNode;
} // namespace tb::mdl

namespace tb::io
{

class MapFileSerializer : public NodeSerializer
{
private:
  using LineStack = std::vector<size_t>;
  LineStack m_startLineStack;
  size_t m_line;
  std::ostream& m_stream;

  struct PrecomputedString
  {
    std::string string;
    size_t lineCount;
  };
  std::unordered_map<const mdl::Node*, PrecomputedString> m_nodeToPrecomputedString;

public:
  static std::unique_ptr<NodeSerializer> create(
    mdl::MapFormat format, std::ostream& stream);

protected:
  explicit MapFileSerializer(std::ostream& stream);

private:
  void doBeginFile(
    const std::vector<const mdl::Node*>& rootNodes,
    kdl::task_manager& taskManager) override;
  void doEndFile() override;

  void doBeginEntity(const mdl::Node* node) override;
  void doEndEntity(const mdl::Node* node) override;
  void doEntityProperty(const mdl::EntityProperty& attribute) override;
  void doBrush(const mdl::BrushNode* brush) override;
  void doBrushFace(const mdl::BrushFace& face) override;

  void doPatch(const mdl::PatchNode* patchNode) override;

private:
  void setFilePosition(const mdl::Node* node);
  size_t startLine();

private: // threadsafe
  virtual void doWriteBrushFace(
    std::ostream& stream, const mdl::BrushFace& face) const = 0;
  PrecomputedString writeBrushFaces(const mdl::Brush& brush) const;
  PrecomputedString writePatch(const mdl::BezierPatch& patch) const;
};

} // namespace tb::io
