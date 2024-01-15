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

#include "TestUtils.h"

#include "Assets/Texture.h"
#include "Ensure.h"
#include "Error.h"
#include "IO/DiskIO.h"
#include "IO/GameConfigParser.h"
#include "Model/BezierPatch.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GameImpl.h"
#include "Model/GroupNode.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "TestLogger.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/result.h>
#include <kdl/string_compare.h>

#include <vecmath/polygon.h>
#include <vecmath/scalar.h>
#include <vecmath/segment.h>

#include <sstream>
#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
bool texCoordsEqual(const vm::vec2f& tc1, const vm::vec2f& tc2)
{
  for (size_t i = 0; i < 2; ++i)
  {
    const float dist = vm::abs(tc1[i] - tc2[i]);
    const float distRemainder = dist - vm::floor(dist);

    if (!(vm::is_equal(distRemainder, 0.0f, vm::Cf::almost_zero())
          || vm::is_equal(distRemainder, 1.0f, vm::Cf::almost_zero())))
      return false;
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
bool UVListsEqual(
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
  if (!texCoordsEqual(uvs[0], transformedVertUVs[0]))
  {
    return false;
  }

  for (size_t i = 1; i < uvs.size(); ++i)
  {
    // note, just checking:
    //   texCoordsEqual(uvs[i], transformedVertUVs[i]);
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

TEST_CASE("TestUtilsTest.testTexCoordsEqual")
{
  CHECK(texCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(0.0, 0.0)));
  CHECK(texCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(1.0, 0.0)));
  CHECK(texCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(2.00001, 0.0)));
  CHECK(texCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(-10.0, 2.0)));
  CHECK(texCoordsEqual(vm::vec2f(2.0, -3.0), vm::vec2f(-10.0, 2.0)));
  CHECK(texCoordsEqual(vm::vec2f(-2.0, -3.0), vm::vec2f(-10.0, 2.0)));
  CHECK(texCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(-1.0, 1.0)));
  CHECK(texCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(-0.00001, 0.0)));
  CHECK(texCoordsEqual(vm::vec2f(0.25, 0.0), vm::vec2f(-0.75, 0.0)));

  CHECK_FALSE(texCoordsEqual(vm::vec2f(0.0, 0.0), vm::vec2f(0.1, 0.1)));
  CHECK_FALSE(texCoordsEqual(vm::vec2f(-0.25, 0.0), vm::vec2f(0.25, 0.0)));
}

TEST_CASE("TestUtilsTest.UVListsEqual")
{
  CHECK(UVListsEqual({{0, 0}, {1, 0}, {0, 1}}, {{0, 0}, {1, 0}, {0, 1}}));
  CHECK(UVListsEqual(
    {{0, 0}, {1, 0}, {0, 1}},
    {{10, 0}, {11, 0}, {10, 1}})); // translation by whole texture increments OK

  CHECK_FALSE(UVListsEqual(
    {{0, 0}, {1, 0}, {0, 1}},
    {{10.5, 0},
     {11.5, 0},
     {10.5, 1}})); // translation by partial texture increments not OK
  CHECK_FALSE(
    UVListsEqual({{0, 0}, {1, 0}, {0, 1}}, {{0, 0}, {0, 1}, {1, 0}})); // wrong order
  CHECK_FALSE(
    UVListsEqual({{0, 0}, {1, 0}, {0, 1}}, {{0, 0}, {2, 0}, {0, 2}})); // unwanted scaling
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

namespace IO
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
    .value();
}
} // namespace IO

namespace Model
{
BrushFace createParaxial(
  const vm::vec3& point0,
  const vm::vec3& point1,
  const vm::vec3& point2,
  const std::string& textureName)
{
  const BrushFaceAttributes attributes(textureName);
  return BrushFace::create(
           point0,
           point1,
           point2,
           attributes,
           std::make_unique<ParaxialTexCoordSystem>(point0, point1, point2, attributes))
    .value();
}

std::vector<vm::vec3> asVertexList(const std::vector<vm::segment3>& edges)
{
  std::vector<vm::vec3> result;
  vm::segment3::get_vertices(
    std::begin(edges), std::end(edges), std::back_inserter(result));
  return result;
}

std::vector<vm::vec3> asVertexList(const std::vector<vm::polygon3>& faces)
{
  std::vector<vm::vec3> result;
  vm::polygon3::get_vertices(
    std::begin(faces), std::end(faces), std::back_inserter(result));
  return result;
}

void assertTexture(
  const std::string& expected, const BrushNode* brushNode, const vm::vec3& faceNormal)
{
  assertTexture(expected, brushNode->brush(), faceNormal);
}

void assertTexture(
  const std::string& expected,
  const BrushNode* brushNode,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3)
{
  return assertTexture(expected, brushNode, std::vector<vm::vec3d>({v1, v2, v3}));
}

void assertTexture(
  const std::string& expected,
  const BrushNode* brushNode,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& v4)
{
  return assertTexture(expected, brushNode, std::vector<vm::vec3d>({v1, v2, v3, v4}));
}

void assertTexture(
  const std::string& expected,
  const BrushNode* brushNode,
  const std::vector<vm::vec3d>& vertices)
{
  return assertTexture(expected, brushNode, vm::polygon3d(vertices));
}

void assertTexture(
  const std::string& expected, const BrushNode* brushNode, const vm::polygon3d& vertices)
{
  assertTexture(expected, brushNode->brush(), vertices);
}

void assertTexture(
  const std::string& expected, const Brush& brush, const vm::vec3& faceNormal)
{
  const auto faceIndex = brush.findFace(faceNormal);
  REQUIRE(faceIndex);

  const BrushFace& face = brush.face(*faceIndex);
  CHECK(face.attributes().textureName() == expected);
}

void assertTexture(
  const std::string& expected,
  const Brush& brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3)
{
  return assertTexture(expected, brush, std::vector<vm::vec3d>({v1, v2, v3}));
}

void assertTexture(
  const std::string& expected,
  const Brush& brush,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& v4)
{
  return assertTexture(expected, brush, std::vector<vm::vec3d>({v1, v2, v3, v4}));
}

void assertTexture(
  const std::string& expected, const Brush& brush, const std::vector<vm::vec3d>& vertices)
{
  return assertTexture(expected, brush, vm::polygon3d(vertices));
}

void assertTexture(
  const std::string& expected, const Brush& brush, const vm::polygon3d& vertices)
{
  const auto faceIndex = brush.findFace(vertices, 0.0001);
  REQUIRE(faceIndex);

  const BrushFace& face = brush.face(*faceIndex);
  CHECK(face.attributes().textureName() == expected);
}

void transformNode(
  Node& node, const vm::mat4x4& transformation, const vm::bbox3& worldBounds)
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
      auto entity = entityNode->entity();
      entity.transform(entityNode->entityPropertyConfig(), transformation);
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

GameAndConfig loadGame(const std::string& gameName)
{
  TestLogger logger;
  const auto configPath =
    std::filesystem::current_path() / "fixture/games" / gameName / "GameConfig.cfg";
  const auto gamePath =
    std::filesystem::current_path() / "fixture/test/Model/Game" / gameName;
  const auto configStr = IO::readTextFile(configPath);
  auto configParser = IO::GameConfigParser(configStr, configPath);
  auto config = std::make_unique<Model::GameConfig>(configParser.parse());
  auto game = std::make_shared<Model::GameImpl>(*config, gamePath, logger);

  // We would ideally just return game, but GameImpl captures a raw reference
  // to the GameConfig.
  return {std::move(game), std::move(config)};
}

const Model::BrushFace* findFaceByPoints(
  const std::vector<Model::BrushFace>& faces,
  const vm::vec3& point0,
  const vm::vec3& point1,
  const vm::vec3& point2)
{
  for (const Model::BrushFace& face : faces)
  {
    if (
      face.points()[0] == point0 && face.points()[1] == point1
      && face.points()[2] == point2)
      return &face;
  }
  return nullptr;
}

void checkFaceTexCoordSystem(const Model::BrushFace& face, const bool expectParallel)
{
  auto snapshot = face.takeTexCoordSystemSnapshot();
  auto* check = dynamic_cast<Model::ParallelTexCoordSystemSnapshot*>(snapshot.get());
  const bool isParallel = (check != nullptr);
  CHECK(isParallel == expectParallel);
}

void checkBrushTexCoordSystem(
  const Model::BrushNode* brushNode, const bool expectParallel)
{
  const auto& faces = brushNode->brush().faces();
  CHECK(faces.size() == 6u);
  checkFaceTexCoordSystem(faces[0], expectParallel);
  checkFaceTexCoordSystem(faces[1], expectParallel);
  checkFaceTexCoordSystem(faces[2], expectParallel);
  checkFaceTexCoordSystem(faces[3], expectParallel);
  checkFaceTexCoordSystem(faces[4], expectParallel);
  checkFaceTexCoordSystem(faces[5], expectParallel);
}

void setLinkId(Node& node, std::string linkId)
{
  node.accept(kdl::overload(
    [&](WorldNode* worldNode) {
      auto entity = worldNode->entity();
      entity.setLinkId(std::move(linkId));
      worldNode->setEntity(std::move(entity));
    },
    [](const LayerNode*) {},
    [&](GroupNode* groupNode) {
      auto group = groupNode->group();
      group.setLinkId(std::move(linkId));
      groupNode->setGroup(std::move(group));
    },
    [&](EntityNode* entityNode) {
      auto entity = entityNode->entity();
      entity.setLinkId(std::move(linkId));
      entityNode->setEntity(std::move(entity));
    },
    [&](BrushNode* brushNode) {
      auto brush = brushNode->brush();
      brush.setLinkId(std::move(linkId));
      brushNode->setBrush(std::move(brush));
    },
    [&](PatchNode* patchNode) {
      auto patch = patchNode->patch();
      patch.setLinkId(std::move(linkId));
      patchNode->setPatch(std::move(patch));
    }));
}
} // namespace Model

namespace View
{
DocumentGameConfig loadMapDocument(
  const std::filesystem::path& mapPath,
  const std::string& gameName,
  const Model::MapFormat mapFormat)
{
  auto [document, game, gameConfig] = newMapDocument(gameName, mapFormat);

  document
    ->loadDocument(
      mapFormat,
      document->worldBounds(),
      document->game(),
      std::filesystem::current_path() / mapPath)
    .transform_error([](auto e) { throw std::runtime_error{e.msg}; });

  return {std::move(document), std::move(game), std::move(gameConfig)};
}

DocumentGameConfig newMapDocument(
  const std::string& gameName, const Model::MapFormat mapFormat)
{
  auto [game, gameConfig] = Model::loadGame(gameName);

  auto document = MapDocumentCommandFacade::newMapDocument();
  document->newDocument(mapFormat, vm::bbox3(8192.0), game).transform_error([](auto e) {
    throw std::runtime_error{e.msg};
  });

  return {std::move(document), std::move(game), std::move(gameConfig)};
}
} // namespace View

int getComponentOfPixel(
  const Assets::Texture& texture,
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

  const auto& mip0DataBuffer = texture.buffersIfUnprepared().at(0);
  assert(texture.width() * texture.height() * 4 == mip0DataBuffer.size());
  assert(x < texture.width());
  assert(y < texture.height());

  const uint8_t* mip0Data = mip0DataBuffer.data();
  return static_cast<int>(
    mip0Data[(texture.width() * 4u * y) + (x * 4u) + componentIndex]);
}

void checkColor(
  const Assets::Texture& texture,
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
} // namespace TrenchBroom
