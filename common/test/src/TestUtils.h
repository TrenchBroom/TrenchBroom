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

#include "IO/DiskIO.h"
#include "IO/ImageFileSystem.h"
#include "Model/MapFormat.h"
#include "Model/Node.h"

#include "vm/polygon.h"
#include "vm/segment.h"

#include <filesystem>
#include <memory>
#include <string>

namespace TrenchBroom
{
bool uvCoordsEqual(const vm::vec2f& tc1, const vm::vec2f& tc2);
bool pointExactlyIntegral(const vm::vec3d& point);
bool uvListsEqual(
  const std::vector<vm::vec2f>& uvs, const std::vector<vm::vec2f>& transformedVertUVs);

namespace Assets
{
class Material;
class Texture;
} // namespace Assets

namespace IO
{

template <typename FS>
auto openFS(const std::filesystem::path& path)
{
  return Disk::openFile(path) | kdl::and_then([](auto file) {
           return createImageFileSystem<FS>(std::move(file));
         })
         | kdl::value();
}

std::string readTextFile(const std::filesystem::path& path);

} // namespace IO

namespace Model
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
  std::shared_ptr<Model::Game> game;
  std::unique_ptr<Model::GameConfig> gameConfig;
};
GameAndConfig loadGame(const std::string& gameName);

const Model::BrushFace* findFaceByPoints(
  const std::vector<Model::BrushFace>& faces,
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2);
void checkFaceUVCoordSystem(const Model::BrushFace& face, bool expectParallel);
void checkBrushUVCoordSystem(const Model::BrushNode* brushNode, bool expectParallel);

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

} // namespace Model

namespace View
{
class MapDocument;

struct DocumentGameConfig
{
  std::shared_ptr<MapDocument> document;
  std::shared_ptr<Model::Game> game;
  std::unique_ptr<Model::GameConfig> gameConfig;
};
DocumentGameConfig loadMapDocument(
  const std::filesystem::path& mapPath,
  const std::string& gameName,
  Model::MapFormat mapFormat);
DocumentGameConfig newMapDocument(
  const std::string& gameName, Model::MapFormat mapFormat);
} // namespace View

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
  const Assets::Texture& texture, std::size_t x, std::size_t y, Component component);
void checkColor(
  const Assets::Texture& texture,
  std::size_t x,
  std::size_t y,
  int r,
  int g,
  int b,
  int a,
  ColorMatch match = ColorMatch::Exact);

int getComponentOfPixel(
  const Assets::Material& material, std::size_t x, std::size_t y, Component component);
void checkColor(
  const Assets::Material& material,
  std::size_t x,
  std::size_t y,
  int r,
  int g,
  int b,
  int a,
  ColorMatch match = ColorMatch::Exact);

} // namespace TrenchBroom
