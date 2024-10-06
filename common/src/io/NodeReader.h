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

#include "io/MapReader.h"

#include <string>
#include <string_view>
#include <vector>

namespace tb::io
{
class ParserStatus;

/**
 * MapReader subclass for loading the clipboard contents, rather than an entire .map
 */
class NodeReader : public MapReader
{
private:
  std::vector<mdl::Node*> m_nodes;

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
    std::string_view str,
    mdl::MapFormat sourceMapFormat,
    mdl::MapFormat targetMapFormat,
    const mdl::EntityPropertyConfig& entityPropertyConfig);

  static std::vector<mdl::Node*> read(
    const std::string& str,
    mdl::MapFormat preferredMapFormat,
    const vm::bbox3d& worldBounds,
    const mdl::EntityPropertyConfig& entityPropertyConfig,
    ParserStatus& status);

private:
  static std::vector<mdl::Node*> readAsFormat(
    mdl::MapFormat sourceMapFormat,
    mdl::MapFormat targetMapFormat,
    const std::string& str,
    const vm::bbox3d& worldBounds,
    const mdl::EntityPropertyConfig& entityPropertyConfig,
    ParserStatus& status);

private: // implement MapReader interface
  mdl::Node* onWorldNode(
    std::unique_ptr<mdl::WorldNode> worldNode, ParserStatus& status) override;
  void onLayerNode(std::unique_ptr<mdl::Node> layerNode, ParserStatus& status) override;
  void onNode(
    mdl::Node* parentNode,
    std::unique_ptr<mdl::Node> node,
    ParserStatus& status) override;
};

} // namespace tb::io
