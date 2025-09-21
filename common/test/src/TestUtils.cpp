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

#include "TestUtils.h"

#include "Ensure.h"
#include "TestLogger.h"
#include "io/DiskIO.h"
#include "io/GameConfigParser.h"
#include "io/ReaderException.h"
#include "mdl/BezierPatch.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GameImpl.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Material.h"
#include "mdl/ParallelUVCoordSystem.h"
#include "mdl/ParaxialUVCoordSystem.h"
#include "mdl/PatchNode.h"
#include "mdl/Resource.h"
#include "mdl/Texture.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "kdl/result.h"

#include "vm/polygon.h"
#include "vm/scalar.h"
#include "vm/segment.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb
{

bool uvCoordsEqual(const vm::vec2f& tc1, const vm::vec2f& tc2)
{
  for (size_t i = 0; i < 2; ++i)
  {
    const float dist = vm::abs(tc1[i] - tc2[i]);
    const float distRemainder = dist - vm::floor(dist);

    if (!(vm::is_equal(distRemainder, 0.0f, vm::Cf::almost_zero())
          || vm::is_equal(distRemainder, 1.0f, vm::Cf::almost_zero())))
    {
      return false;
    }
  }
  return true;
}

bool pointExactlyIntegral(const vm::vec3d& point)
{
  for (size_t i = 0; i < 3; i++)
  {
    const double value = point[i];
    if (static_cast<double>(static_cast<int>(value)) != value)
    {
      return false;
    }
  }
  return true;
}

/**
 * Assumes the UV's have been divided by the texture size.
 */
bool uvListsEqual(
  const std::vector<vm::vec2f>& uvs, const std::vector<vm::vec2f>& transformedVertUVs)
{
  if (uvs.size() != transformedVertUVs.size())
  {
    return false;
  }
  if (uvs.size() < 3U)
  {
    return false;
  }
  if (!uvCoordsEqual(uvs[0], transformedVertUVs[0]))
  {
    return false;
  }

  for (size_t i = 1; i < uvs.size(); ++i)
  {
    // note, just checking:
    //   uvCoordsEqual(uvs[i], transformedVertUVs[i]);
    // would be too lenient.
    const vm::vec2f expected = uvs[i] - uvs[0];
    const vm::vec2f actual = transformedVertUVs[i] - transformedVertUVs[0];
    if (!vm::is_equal(expected, actual, vm::Cf::almost_zero()))
    {
      return false;
    }
  }
  return true;
}

TEST_CASE("TestUtilsTest.testUVCoordsEqual")
{
  CHECK(uvCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(0.0, 0.0)));
  CHECK(uvCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(1.0, 0.0)));
  CHECK(uvCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(2.00001, 0.0)));
  CHECK(uvCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(-10.0, 2.0)));
  CHECK(uvCoordsEqual(vm::vec2f(2.0, -3.0), vm::vec2f(-10.0, 2.0)));
  CHECK(uvCoordsEqual(vm::vec2f(-2.0, -3.0), vm::vec2f(-10.0, 2.0)));
  CHECK(uvCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(-1.0, 1.0)));
  CHECK(uvCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(-0.00001, 0.0)));
  CHECK(uvCoordsEqual(vm::vec2f(0.25, 0.0), vm::vec2f(-0.75, 0.0)));

  CHECK_FALSE(uvCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(0.1, 0.1)));
  CHECK_FALSE(uvCoordsEqual(vm::vec2f(-0.25, 0.0), vm::vec2f(0.25, 0.0)));
}

TEST_CASE("TestUtilsTest.uvListsEqual")
{
  CHECK(uvListsEqual({{0, 0}, {1, 0}, {0, 1}}, {{0, 0}, {1, 0}, {0, 1}}));
  CHECK(uvListsEqual(
    {{0, 0}, {1, 0}, {0, 1}},
    {{10, 0}, {11, 0}, {10, 1}})); // translation by whole UV increments OK

  CHECK_FALSE(uvListsEqual(
    {{0, 0}, {1, 0}, {0, 1}},
    {{10.5, 0}, {11.5, 0}, {10.5, 1}})); // translation by partial UV increments not OK
  CHECK_FALSE(
    uvListsEqual({{0, 0}, {1, 0}, {0, 1}}, {{0, 0}, {0, 1}, {1, 0}})); // wrong order
  CHECK_FALSE(
    uvListsEqual({{0, 0}, {1, 0}, {0, 1}}, {{0, 0}, {2, 0}, {0, 2}})); // unwanted scaling
}

TEST_CASE("TestUtilsTest.pointExactlyIntegral")
{
  CHECK(pointExactlyIntegral(vm::vec3d(0.0, 0.0, 0.0)));
  CHECK(pointExactlyIntegral(vm::vec3d(1024.0, 1204.0, 1024.0)));
  CHECK(pointExactlyIntegral(vm::vec3d(-10000.0, -10000.0, -10000.0)));

  const double near1024 = vm::nextgreater(1024.0);
  CHECK_FALSE(pointExactlyIntegral(vm::vec3d(1024.0, near1024, 1024.0)));
  CHECK_FALSE(pointExactlyIntegral(vm::vec3d(1024.5, 1024.5, 1024.5)));
}

std::unique_ptr<kdl::task_manager> createTestTaskManager()
{
  return std::make_unique<kdl::task_manager>(1);
}

namespace io
{
std::string readTextFile(const std::filesystem::path& path)
{
  const auto fixedPath = Disk::fixPath(path);
  return Disk::withInputStream(
           fixedPath,
           [](auto& stream) {
             return std::string{
               (std::istreambuf_iterator<char>(stream)),
               std::istreambuf_iterator<char>()};
           })
         | kdl::value();
}

Result<std::string> readTextFile(const FileSystem& fs, const std::filesystem::path& path)
{
  try
  {
    return fs.openFile(path) | kdl::transform([](const auto file) {
             return file->reader().readString(file->size());
           });
  }
  catch (const ReaderException& e)
  {
    return Error{fmt::format("Failed to read file {}: {}", path, e.what())};
  }
}

} // namespace io

namespace mdl
{
BrushFace createParaxial(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const std::string& materialName)
{
  const BrushFaceAttributes attributes(materialName);
  return BrushFace::create(
           point0,
           point1,
           point2,
           attributes,
           std::make_unique<ParaxialUVCoordSystem>(point0, point1, point2, attributes))
         | kdl::value();
}

std::vector<vm::vec3d> asVertexList(const std::vector<vm::segment3d>& edges)
{
  std::vector<vm::vec3d> result;
  vm::segment3d::get_vertices(
    std::begin(edges), std::end(edges), std::back_inserter(result));
  return result;
}

std::vector<vm::vec3d> asVertexList(const std::vector<vm::polygon3d>& faces)
{
  std::vector<vm::vec3d> result;
  vm::polygon3d::get_vertices(
    std::begin(faces), std::end(faces), std::back_inserter(result));
  return result;
}

void assertMaterial(
  const std::string& expected, const BrushNode* brushNode, const vm::vec3d& faceNormal)
{
  assertMaterial(expected, brushNode->brush(), faceNormal);
}

void assertMaterial(
  const std::string& expected,
  const BrushNode* brushNode,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3)
{
  return assertMaterial(expected, brushNode, std::vector<vm::vec3d>({v1, v2, v3}));
}

void assertMaterial(
  const std::string& expected,
  const BrushNode* brushNode,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& v4)
{
  return assertMaterial(expected, brushNode, std::vector<vm::vec3d>({v1, v2, v3, v4}));
}

void assertMaterial(
  const std::string& expected,
  const BrushNode* brushNode,
  const std::vector<vm::vec3d>& vertices)
{
  return assertMaterial(expected, brushNode, vm::polygon3d(vertices));
}

void assertMaterial(
  const std::string& expected, const BrushNode* brushNode, const vm::polygon3d& vertices)
{
  assertMaterial(expected, brushNode->brush(), vertices);
}

void assertMaterial(
  const std::string& expected, const Brush& brush, const vm::vec3d& faceNormal)
{
  const auto faceIndex = brush.findFace(faceNormal);
  REQUIRE(faceIndex);

  const BrushFace& face = brush.face(*faceIndex);
  CHECK(face.attributes().materialName() == expected);
}

void assertMaterial(
  const std::string& expected,
  const Brush& brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3)
{
  return assertMaterial(expected, brush, std::vector<vm::vec3d>({v1, v2, v3}));
}

void assertMaterial(
  const std::string& expected,
  const Brush& brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& v4)
{
  return assertMaterial(expected, brush, std::vector<vm::vec3d>({v1, v2, v3, v4}));
}

void assertMaterial(
  const std::string& expected, const Brush& brush, const std::vector<vm::vec3d>& vertices)
{
  return assertMaterial(expected, brush, vm::polygon3d(vertices));
}

void assertMaterial(
  const std::string& expected, const Brush& brush, const vm::polygon3d& vertices)
{
  const auto faceIndex = brush.findFace(vertices, 0.0001);
  REQUIRE(faceIndex);

  const BrushFace& face = brush.face(*faceIndex);
  CHECK(face.attributes().materialName() == expected);
}

void transformNode(
  Node& node, const vm::mat4x4d& transformation, const vm::bbox3d& worldBounds)
{
  node.accept(kdl::overload(
    [](const WorldNode*) {},
    [](const LayerNode*) {},
    [&](auto&& thisLambda, GroupNode* groupNode) {
      auto group = groupNode->group();
      group.transform(transformation);
      groupNode->setGroup(std::move(group));

      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode* entityNode) {
      const auto updateAngleProperty =
        entityNode->entityPropertyConfig().updateAnglePropertyAfterTransform;

      auto entity = entityNode->entity();
      entity.transform(transformation, updateAngleProperty);
      entityNode->setEntity(std::move(entity));

      entityNode->visitChildren(thisLambda);
    },
    [&](BrushNode* brushNode) {
      auto brush = brushNode->brush();
      REQUIRE(brush.transform(worldBounds, transformation, false).is_success());
      brushNode->setBrush(std::move(brush));
    },
    [&](PatchNode* patchNode) {
      auto patch = patchNode->patch();
      patch.transform(transformation);
      patchNode->setPatch(std::move(patch));
    }));
}

std::unique_ptr<Game> loadGame(const std::string& gameName)
{
  TestLogger logger;
  const auto configPath =
    std::filesystem::current_path() / "fixture/games" / gameName / "GameConfig.cfg";
  const auto gamePath =
    std::filesystem::current_path() / "fixture/test/mdl/Game" / gameName;
  const auto configStr = io::readTextFile(configPath);
  auto configParser = io::GameConfigParser(configStr, configPath);
  auto config = configParser.parse().value();
  auto game = std::make_unique<mdl::GameImpl>(std::move(config), gamePath, logger);

  return game;
}

const mdl::BrushFace* findFaceByPoints(
  const std::vector<mdl::BrushFace>& faces,
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2)
{
  for (const mdl::BrushFace& face : faces)
  {
    if (
      face.points()[0] == point0 && face.points()[1] == point1
      && face.points()[2] == point2)
    {
      return &face;
    }
  }
  return nullptr;
}

void checkFaceUVCoordSystem(const mdl::BrushFace& face, const bool expectParallel)
{
  auto snapshot = face.takeUVCoordSystemSnapshot();
  auto* check = dynamic_cast<mdl::ParallelUVCoordSystemSnapshot*>(snapshot.get());
  const bool isParallel = (check != nullptr);
  CHECK(isParallel == expectParallel);
}

void checkBrushUVCoordSystem(const mdl::BrushNode* brushNode, const bool expectParallel)
{
  const auto& faces = brushNode->brush().faces();
  CHECK(faces.size() == 6u);
  checkFaceUVCoordSystem(faces[0], expectParallel);
  checkFaceUVCoordSystem(faces[1], expectParallel);
  checkFaceUVCoordSystem(faces[2], expectParallel);
  checkFaceUVCoordSystem(faces[3], expectParallel);
  checkFaceUVCoordSystem(faces[4], expectParallel);
  checkFaceUVCoordSystem(faces[5], expectParallel);
}

void setLinkId(Node& node, std::string linkId)
{
  node.accept(kdl::overload(
    [](const WorldNode*) {},
    [](const LayerNode*) {},
    [&](Object* object) { object->setLinkId(std::move(linkId)); }));
}

Selection makeSelection(const std::vector<Node*>& nodes)
{
  auto selection = mdl::Selection{};

  mdl::Node::visitAll(
    nodes,
    kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [&](mdl::GroupNode* group) {
        selection.nodes.push_back(group);
        selection.groups.push_back(group);
      },
      [&](mdl::EntityNode* entity) {
        selection.nodes.push_back(entity);
        selection.entities.push_back(entity);
      },
      [&](mdl::BrushNode* brush) {
        selection.nodes.push_back(brush);
        selection.brushes.push_back(brush);
      },
      [&](mdl::PatchNode* patch) {
        selection.nodes.push_back(patch);
        selection.patches.push_back(patch);
      }));

  return selection;
}

Selection makeSelection(const std::vector<BrushFaceHandle>& brushFaces)
{
  auto selection = Selection{};
  selection.brushFaces = brushFaces;
  return selection;
}

} // namespace mdl

namespace ui
{
DocumentGameConfig loadMapDocument(
  const std::filesystem::path& mapPath,
  const std::string& gameName,
  const mdl::MapFormat mapFormat)
{
  auto taskManager = createTestTaskManager();
  auto document = std::make_shared<MapDocument>(*taskManager);
  auto& map = document->map();

  auto game = mdl::loadGame(gameName);
  map.load(
    mapFormat,
    vm::bbox3d{8192.0},
    std::move(game),
    std::filesystem::current_path() / mapPath)
    | kdl::transform_error([](auto e) { throw std::runtime_error{e.msg}; });

  map.processResourcesSync(mdl::ProcessContext{false, [](auto, auto) {}});

  return {document, std::move(taskManager)};
}

DocumentGameConfig newMapDocument(
  const std::string& gameName, const mdl::MapFormat mapFormat)
{
  auto taskManager = createTestTaskManager();
  auto document = std::make_shared<MapDocument>(*taskManager);
  auto& map = document->map();

  auto game = mdl::loadGame(gameName);
  map.create(mapFormat, vm::bbox3d{8192.0}, std::move(game))
    | kdl::transform_error([](auto e) { throw std::runtime_error{e.msg}; });

  return {document, std::move(taskManager)};
}
} // namespace ui

int getComponentOfPixel(
  const mdl::Texture& texture,
  const std::size_t x,
  const std::size_t y,
  const Component component)
{
  const auto format = texture.format();

  ensure(GL_BGRA == format || GL_RGBA == format, "expected GL_BGRA or GL_RGBA");

  std::size_t componentIndex = 0;
  if (format == GL_RGBA)
  {
    switch (component)
    {
    case Component::R:
      componentIndex = 0u;
      break;
    case Component::G:
      componentIndex = 1u;
      break;
    case Component::B:
      componentIndex = 2u;
      break;
    case Component::A:
      componentIndex = 3u;
      break;
    }
  }
  else
  {
    switch (component)
    {
    case Component::R:
      componentIndex = 2u;
      break;
    case Component::G:
      componentIndex = 1u;
      break;
    case Component::B:
      componentIndex = 0u;
      break;
    case Component::A:
      componentIndex = 3u;
      break;
    }
  }

  const auto& mip0DataBuffer = texture.buffersIfLoaded().at(0);
  assert(texture.width() * texture.height() * 4 == mip0DataBuffer.size());
  assert(x < texture.width());
  assert(y < texture.height());

  const uint8_t* mip0Data = mip0DataBuffer.data();
  return static_cast<int>(
    mip0Data[(texture.width() * 4u * y) + (x * 4u) + componentIndex]);
}

void checkColor(
  const mdl::Texture& texture,
  const std::size_t x,
  const std::size_t y,
  const int r,
  const int g,
  const int b,
  const int a,
  const ColorMatch match)
{

  const auto actualR = getComponentOfPixel(texture, x, y, Component::R);
  const auto actualG = getComponentOfPixel(texture, x, y, Component::G);
  const auto actualB = getComponentOfPixel(texture, x, y, Component::B);
  const auto actualA = getComponentOfPixel(texture, x, y, Component::A);

  if (match == ColorMatch::Approximate)
  {
    // allow some error for lossy formats, e.g. JPG
    CHECK(std::abs(r - actualR) <= 5);
    CHECK(std::abs(g - actualG) <= 5);
    CHECK(std::abs(b - actualB) <= 5);
    CHECK(a == actualA);
  }
  else
  {
    CHECK(r == actualR);
    CHECK(g == actualG);
    CHECK(b == actualB);
    CHECK(a == actualA);
  }
}

int getComponentOfPixel(
  const mdl::Material& material, std::size_t x, std::size_t y, Component component)
{
  ensure(material.texture(), "expected material to have a texture");
  return getComponentOfPixel(*material.texture(), x, y, component);
}

void checkColor(
  const mdl::Material& material,
  const std::size_t x,
  const std::size_t y,
  const int r,
  const int g,
  const int b,
  const int a,
  const ColorMatch match)
{
  ensure(material.texture(), "expected material to have a texture");
  return checkColor(*material.texture(), x, y, r, g, b, a, match);
}
} // namespace tb
