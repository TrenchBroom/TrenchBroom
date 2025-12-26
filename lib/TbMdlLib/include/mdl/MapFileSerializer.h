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

#include "mdl/MapFormat.h"
#include "mdl/NodeSerializer.h"

#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <vector>


namespace tb
{
namespace mdl
{
class BezierPatch;
class Brush;
class BrushNode;
class BrushFace;
class EntityProperty;
class Node;
class PatchNode;

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
  std::unordered_map<const Node*, PrecomputedString> m_nodeToPrecomputedString;

public:
  static std::unique_ptr<NodeSerializer> create(MapFormat format, std::ostream& stream);

protected:
  explicit MapFileSerializer(std::ostream& stream);

private:
  void doBeginFile(
    const std::vector<const Node*>& rootNodes, kdl::task_manager& taskManager) override;
  void doEndFile() override;

  void doBeginEntity(const Node* node) override;
  void doEndEntity(const Node* node) override;
  void doEntityProperty(const EntityProperty& attribute) override;
  void doBrush(const BrushNode* brush) override;
  void doBrushFace(const BrushFace& face) override;

  void doPatch(const PatchNode* patchNode) override;

private:
  void setFilePosition(const Node* node);
  size_t startLine();

private: // threadsafe
  virtual void doWriteBrushFace(std::ostream& stream, const BrushFace& face) const = 0;
  PrecomputedString writeBrushFaces(const Brush& brush) const;
  PrecomputedString writePatch(const BezierPatch& patch) const;
};

} // namespace mdl
} // namespace tb
