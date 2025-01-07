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

#include "io/DiskIO.h"
#include "io/ImageFileSystem.h"
#include "mdl/MapFormat.h"
#include "mdl/Node.h"

#include "kdl/task_manager.h"

#include "vm/polygon.h"
#include "vm/segment.h"

#include <filesystem>
#include <memory>
#include <string>

namespace tb
{
bool uvCoordsEqual(const vm::vec2f& tc1, const vm::vec2f& tc2);
bool pointExactlyIntegral(const vm::vec3d& point);
bool uvListsEqual(
  const std::vector<vm::vec2f>& uvs, const std::vector<vm::vec2f>& transformedVertUVs);

std::unique_ptr<kdl::task_manager> createTestTaskManager();

namespace mdl
{
class Material;
class Texture;
} // namespace mdl

namespace io
{

template <typename FS>
auto openFS(const std::filesystem::path& path)
{
  return Disk::openFile(path) | kdl::and_then([](auto file) {
           return createImageFileSystem<FS>(std::move(file));
         })
         | kdl::transform([&](auto fs) {
             fs->setMetadata(io::makeImageFileSystemMetadata(path));
             return fs;
           })
         | kdl::value();
}

std::string readTextFile(const std::filesystem::path& path);

} // namespace io

namespace mdl
{
class Brush;
class BrushFace;
class BrushNode;
class Game;
struct GameConfig;
class GroupNode;
class Node;

BrushFace createParaxial(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const std::string& materialName = "");

std::vector<vm::vec3d> asVertexList(const std::vector<vm::segment3d>& edges);
std::vector<vm::vec3d> asVertexList(const std::vector<vm::polygon3d>& faces);

void assertMaterial(
  const std::string& expected, const BrushNode* brush, const vm::vec3d& faceNormal);
void assertMaterial(
  const std::string& expected,
  const BrushNode* brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3);
void assertMaterial(
  const std::string& expected,
  const BrushNode* brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& v4);
void assertMaterial(
  const std::string& expected,
  const BrushNode* brush,
  const std::vector<vm::vec3d>& vertices);
void assertMaterial(
  const std::string& expected, const BrushNode* brush, const vm::polygon3d& vertices);

void assertMaterial(
  const std::string& expected, const Brush& brush, const vm::vec3d& faceNormal);
void assertMaterial(
  const std::string& expected,
  const Brush& brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3);
void assertMaterial(
  const std::string& expected,
  const Brush& brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& v4);
void assertMaterial(
  const std::string& expected,
  const Brush& brush,
  const std::vector<vm::vec3d>& vertices);
void assertMaterial(
  const std::string& expected, const Brush& brush, const vm::polygon3d& vertices);

void transformNode(
  Node& node, const vm::mat4x4d& transformation, const vm::bbox3d& worldBounds);

struct GameAndConfig
{
  std::shared_ptr<mdl::Game> game;
  std::unique_ptr<mdl::GameConfig> gameConfig;
};
GameAndConfig loadGame(const std::string& gameName);

const mdl::BrushFace* findFaceByPoints(
  const std::vector<mdl::BrushFace>& faces,
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2);
void checkFaceUVCoordSystem(const mdl::BrushFace& face, bool expectParallel);
void checkBrushUVCoordSystem(const mdl::BrushNode* brushNode, bool expectParallel);

void setLinkId(Node& node, std::string linkId);

template <typename Child>
auto findFirstChildOfType(const std::vector<Node*>& children)
{
  return std::find_if(children.begin(), children.end(), [](const auto* child) {
    return dynamic_cast<const Child*>(child) != nullptr;
  });
}

template <typename Child>
Child* getFirstChildOfType(std::vector<Node*>& children)
{
  if (const auto it = findFirstChildOfType<Child>(children); it != children.end())
  {
    auto* child = static_cast<Child*>(*it);
    children.erase(it);
    return child;
  }
  throw std::runtime_error{"Missing child"};
}

template <typename... Children>
std::tuple<Children*...> getChildrenAs(const Node& node)
{
  // take a copy
  auto children = node.children();
  return std::tuple<Children*...>{getFirstChildOfType<Children>(children)...};
}

template <typename Child>
Child* getChildAs(const Node& node)
{
  // take a copy
  auto children = node.children();
  return getFirstChildOfType<Child>(children);
}

} // namespace mdl

namespace ui
{
class MapDocument;

struct DocumentGameConfig
{
  std::shared_ptr<MapDocument> document;
  std::shared_ptr<mdl::Game> game;
  std::unique_ptr<mdl::GameConfig> gameConfig;
  std::unique_ptr<kdl::task_manager> taskManager;
};

DocumentGameConfig loadMapDocument(
  const std::filesystem::path& mapPath,
  const std::string& gameName,
  mdl::MapFormat mapFormat);

DocumentGameConfig newMapDocument(const std::string& gameName, mdl::MapFormat mapFormat);
} // namespace ui

enum class Component
{
  R,
  G,
  B,
  A
};

enum class ColorMatch
{
  Exact,
  Approximate
};

int getComponentOfPixel(
  const mdl::Texture& texture, std::size_t x, std::size_t y, Component component);
void checkColor(
  const mdl::Texture& texture,
  std::size_t x,
  std::size_t y,
  int r,
  int g,
  int b,
  int a,
  ColorMatch match = ColorMatch::Exact);

int getComponentOfPixel(
  const mdl::Material& material, std::size_t x, std::size_t y, Component component);
void checkColor(
  const mdl::Material& material,
  std::size_t x,
  std::size_t y,
  int r,
  int g,
  int b,
  int a,
  ColorMatch match = ColorMatch::Exact);

} // namespace tb
