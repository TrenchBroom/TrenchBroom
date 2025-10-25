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

#include "Result.h"
#include "mdl/MapReader.h"

#include <string>
#include <string_view>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb
{
class ParserStatus;

namespace mdl
{

/**
 * MapReader subclass for loading the clipboard contents, rather than an entire .map
 */
class NodeReader : public MapReader
{
private:
  std::vector<Node*> m_nodes;

public:
  /**
   * Creates a new parser where the given string is expected to be formatted in the given
   * source map format, and the created objects are converted to the given target format.
   *
   * @param str the string to parse
   * @param sourceMapFormat the expected format of the given string
   * @param targetMapFormat the format to convert the created objects to
   * @param entityPropertyConfig the entity property config to use
   */
  NodeReader(
    const mdl::GameConfig &config,
      std::string_view str,
    MapFormat sourceMapFormat,
    MapFormat targetMapFormat,
    const EntityPropertyConfig& entityPropertyConfig);

  static Result<std::vector<Node*>> read(
    const mdl::GameConfig& config,
    const std::string& str,
    MapFormat preferredMapFormat,
    const vm::bbox3d& worldBounds,
    const EntityPropertyConfig& entityPropertyConfig,
    ParserStatus& status,
    kdl::task_manager& taskManager);

private:
  static Result<std::vector<Node*>> readAsFormat(
    MapFormat sourceMapFormat,
    MapFormat targetMapFormat,
    const mdl::GameConfig &config,
    const std::string& str,
    const vm::bbox3d& worldBounds,
    const EntityPropertyConfig& entityPropertyConfig,
    ParserStatus& status,
    kdl::task_manager& taskManager);

private: // implement MapReader interface
  Node* onWorldNode(std::unique_ptr<WorldNode> worldNode, ParserStatus& status) override;
  void onLayerNode(std::unique_ptr<Node> layerNode, ParserStatus& status) override;
  void onNode(
    Node* parentNode, std::unique_ptr<Node> node, ParserStatus& status) override;
};

} // namespace mdl
} // namespace tb
