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

#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushNode.h"
#include "Model/MapFormat.h"
#include "Model/ParallelUVCoordSystem.h"
#include "Model/ParaxialUVCoordSystem.h"
#include "Model/Polyhedron.h"
#include "TestUtils.h"

#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include "vm/approx.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <memory>
#include <vector>

#include "Catch2.h"

namespace tb::Model
{
namespace
{

void getFaceVertsAndUVCoords(
  const BrushFace& face,
  std::vector<vm::vec3d>* vertPositions,
  std::vector<vm::vec2f>* vertUVCoords)
{
  for (const auto* vertex : face.vertices())
  {
    vertPositions->push_back(vertex->position());
    if (vertUVCoords)
    {
      vertUVCoords->push_back(face.uvCoords(vm::vec3d{vertex->position()}));
    }
  }
}

void resetFaceUVAlignment(BrushFace& face)
{
  auto attributes = face.attributes();
  attributes.setXOffset(0.0);
  attributes.setYOffset(0.0);
  attributes.setRotation(0.0);
  attributes.setXScale(1.0);
  attributes.setYScale(1.0);

  face.setAttributes(attributes);
  face.resetUVAxes();
}

/**
 * Assumes the UV's have been divided by the texture size.
 */
void checkUVListsEqual(
  const std::vector<vm::vec2f>& uvs,
  const std::vector<vm::vec2f>& transformedVertUVs,
  const BrushFace& face)
{
  // We require a material, so that face.textureSize() returns a correct value and not
  // 1x1, and so face.uvCoords() returns UV's that are divided by the texture size.
  // Otherwise, the UV comparisons below could spuriously pass.
  REQUIRE(face.material() != nullptr);

  CHECK(uvListsEqual(uvs, transformedVertUVs));
}

/**
 * Incomplete test for transforming a face with alignment lock off.
 *
 * It only tests that alignment lock off works when the face's alignment is reset before
 * applying the transform.
 */
void checkAlignmentLockOffWithTransform(
  const vm::mat4x4d& transform, const BrushFace& origFace)
{

  // reset alignment, transform the face (alignment lock off)
  auto face = origFace;
  resetFaceUVAlignment(face);
  REQUIRE(face.transform(transform, false).is_success());
  face.resetUVCoordSystemCache();

  // reset alignment, transform the face (alignment lock off), then reset the alignment
  // again
  auto resetFace = origFace;
  resetFaceUVAlignment(resetFace);
  REQUIRE(resetFace.transform(transform, false).is_success());
  resetFaceUVAlignment(resetFace);

  // UVs of the verts of `face` and `resetFace` should be the same now

  auto verts = std::vector<vm::vec3d>{};
  getFaceVertsAndUVCoords(origFace, &verts, nullptr);

  // transform the verts
  auto transformedVerts = std::vector<vm::vec3d>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    transformedVerts.push_back(transform * verts[i]);
  }

  // get UV of each transformed vert using `face` and `resetFace`
  auto face_UVs = std::vector<vm::vec2f>{};
  auto resetFace_UVs = std::vector<vm::vec2f>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    face_UVs.push_back(face.uvCoords(transformedVerts[i]));
    resetFace_UVs.push_back(resetFace.uvCoords(transformedVerts[i]));
  }

  checkUVListsEqual(face_UVs, resetFace_UVs, face);
}

void checkFaceUVsEqual(const BrushFace& face, const BrushFace& other)
{
  auto verts = std::vector<vm::vec3d>{};
  auto faceUVs = std::vector<vm::vec2f>{};
  auto otherFaceUVs = std::vector<vm::vec2f>{};

  for (const auto* vertex : face.vertices())
  {
    verts.push_back(vertex->position());

    const auto position = vm::vec3d{vertex->position()};
    faceUVs.push_back(face.uvCoords(position));
    otherFaceUVs.push_back(other.uvCoords(position));
  }

  checkUVListsEqual(faceUVs, otherFaceUVs, face);
}

void checkBrushUVsEqual(const Brush& brush, const Brush& other)
{
  assert(brush.faceCount() == other.faceCount());

  for (size_t i = 0; i < brush.faceCount(); ++i)
  {
    checkFaceUVsEqual(brush.face(i), other.face(i));
  }
}

/**
 * Applies the given transform to a copy of origFace.
 *
 * Checks that the UV coordinates of the verts
 * are equivelant to the UV coordinates of the non-transformed verts,
 * i.e. checks that alignment lock worked.
 */
void checkAlignmentLockOnWithTransform(
  const vm::mat4x4d& transform, const BrushFace& origFace)
{
  auto verts = std::vector<vm::vec3d>{};
  auto uvs = std::vector<vm::vec2f>{};
  getFaceVertsAndUVCoords(origFace, &verts, &uvs);
  CHECK(verts.size() >= 3u);

  // transform the face
  auto face = origFace;
  REQUIRE(face.transform(transform, true).is_success());
  face.resetUVCoordSystemCache();

  // transform the verts
  auto transformedVerts = std::vector<vm::vec3d>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    transformedVerts.push_back(transform * verts[i]);
  }

  // ask the transformed face for the UVs at the transformed verts
  auto transformedVertUVs = std::vector<vm::vec2f>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    transformedVertUVs.push_back(face.uvCoords(transformedVerts[i]));
  }

  checkUVListsEqual(uvs, transformedVertUVs, face);
}

/**
 * Given a face and three reference verts and their UVs,
 * generates many different transformations and checks that the UVs are
 * stable after these transformations.
 */
template <class L>
void doWithTranslationAnd90DegreeRotations(L&& lambda)
{
  for (int i = 0; i < (1 << 7); i++)
  {
    auto xform = vm::mat4x4d{};

    const auto translate = (i & (1 << 0)) != 0;

    const auto rollMinus180 = (i & (1 << 1)) != 0;
    const auto pitchMinus180 = (i & (1 << 2)) != 0;
    const auto yawMinus180 = (i & (1 << 3)) != 0;

    const auto rollPlus90 = (i & (1 << 4)) != 0;
    const auto pitchPlus90 = (i & (1 << 5)) != 0;
    const auto yawPlus90 = (i & (1 << 6)) != 0;

    // translations

    if (translate)
    {
      xform = vm::translation_matrix(vm::vec3d{100, 100, 100}) * xform;
    }

    // -180 / -90 / 90 degree rotations

    if (rollMinus180)
    {
      xform = vm::rotation_matrix(vm::to_radians(-180.0), 0.0, 0.0) * xform;
    }
    if (pitchMinus180)
    {
      xform = vm::rotation_matrix(0.0, vm::to_radians(-180.0), 0.0) * xform;
    }
    if (yawMinus180)
    {
      xform = vm::rotation_matrix(0.0, 0.0, vm::to_radians(-180.0)) * xform;
    }

    if (rollPlus90)
    {
      xform = vm::rotation_matrix(vm::to_radians(90.0), 0.0, 0.0) * xform;
    }
    if (pitchPlus90)
    {
      xform = vm::rotation_matrix(0.0, vm::to_radians(90.0), 0.0) * xform;
    }
    if (yawPlus90)
    {
      xform = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0)) * xform;
    }

    lambda(xform);
  }
}

/**
 * Generates transforms for testing alignment lock, etc., by rotating by the given
 * amount, in each axis alone, as well as in all combinations of axes.
 */
template <class L>
void doMultiAxisRotations(const double degrees, L&& lambda)
{
  const auto rotateRadians = vm::to_radians(degrees);

  for (int i = 0; i < (1 << 3); i++)
  {
    auto xform = vm::mat4x4d{};

    const auto testRoll = (i & (1 << 0)) != 0;
    const auto testPitch = (i & (1 << 1)) != 0;
    const auto testYaw = (i & (1 << 2)) != 0;

    if (testRoll)
    {
      xform = vm::rotation_matrix(rotateRadians, 0.0, 0.0) * xform;
    }
    if (testPitch)
    {
      xform = vm::rotation_matrix(0.0, rotateRadians, 0.0) * xform;
    }
    if (testYaw)
    {
      xform = vm::rotation_matrix(0.0, 0.0, rotateRadians) * xform;
    }

    lambda(xform);
  }
}

/**
 * Runs the given lambda of type `const vm::mat4x4d& -> void` with
 * rotations of the given angle in degrees in +/- pitch, yaw, and roll.
 */
template <class L>
void doWithSingleAxisRotations(const double degrees, L&& lambda)
{
  const double rotateRadians = vm::to_radians(degrees);

  for (int i = 0; i < 6; i++)
  {
    auto xform = vm::mat4x4d{};

    switch (i)
    {
    case 0:
      xform = vm::rotation_matrix(rotateRadians, 0.0, 0.0) * xform;
      break;
    case 1:
      xform = vm::rotation_matrix(-rotateRadians, 0.0, 0.0) * xform;
      break;
    case 2:
      xform = vm::rotation_matrix(0.0, rotateRadians, 0.0) * xform;
      break;
    case 3:
      xform = vm::rotation_matrix(0.0, -rotateRadians, 0.0) * xform;
      break;
    case 4:
      xform = vm::rotation_matrix(0.0, 0.0, rotateRadians) * xform;
      break;
    case 5:
      xform = vm::rotation_matrix(0.0, 0.0, -rotateRadians) * xform;
      break;
    }

    lambda(xform);
  }
}

void checkAlignmentLockOffWithTranslation(const BrushFace& origFace)
{
  const auto xform = vm::translation_matrix(vm::vec3d(100.0, 100.0, 100.0));
  checkAlignmentLockOffWithTransform(xform, origFace);
}

template <class L>
void doWithScale(const vm::vec3d& scaleFactors, L&& lambda)
{
  const auto xform = vm::scaling_matrix(scaleFactors);
  lambda(xform);
}

template <class L>
void doWithShear(L&& lambda)
{
  // shear the x axis towards the y axis
  const auto xform = vm::shear_matrix(1.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  lambda(xform);
}

template <class L>
void doWithAlignmentLockTestTransforms(const bool doParallelTests, L&& lambda)
{
  doWithTranslationAnd90DegreeRotations(lambda);
  doWithSingleAxisRotations(30, lambda);
  doWithSingleAxisRotations(45, lambda);

  // rotation on multiple axes simultaneously is only expected to work on
  // ParallelUVCoordSystem
  if (doParallelTests)
  {
    doMultiAxisRotations(30.0, lambda);
    doMultiAxisRotations(45.0, lambda);

    doWithShear(lambda);
  }

  doWithScale(vm::vec3d{2, 2, 1}, lambda);
  doWithScale(vm::vec3d{2, 2, -1}, lambda);
}

void checkAlignmentLockForFace(const BrushFace& origFace, const bool doParallelTests)
{
  doWithAlignmentLockTestTransforms(doParallelTests, [&](const vm::mat4x4d& xform) {
    checkAlignmentLockOnWithTransform(xform, origFace);
  });

  checkAlignmentLockOffWithTranslation(origFace);
}

/**
 * For the sides of a cube, a horizontal or vertical flip should have no effect on
 * texturing when alignment lock is off.
 */
void checkAlignmentLockOffWithVerticalFlip(const Brush& cube)
{
  const auto transform = vm::mirror_matrix<double>(vm::axis::z);
  const auto origFaceIndex = cube.findFace(vm::vec3d{1, 0, 0});
  REQUIRE(origFaceIndex);
  const auto& origFace = cube.face(*origFaceIndex);

  // transform the face (alignment lock off)
  auto face = origFace;
  REQUIRE(face.transform(transform, false).is_success());
  face.resetUVCoordSystemCache();

  // UVs of the verts of `face` and `origFace` should be the same now

  // get UV of each vert using `face` and `resetFace`
  auto face_UVs = std::vector<vm::vec2f>{};
  auto origFace_UVs = std::vector<vm::vec2f>{};
  for (const auto vert : origFace.vertices())
  {
    face_UVs.push_back(face.uvCoords(vert->position()));
    origFace_UVs.push_back(origFace.uvCoords(vert->position()));
  }

  checkUVListsEqual(face_UVs, origFace_UVs, face);
}

void checkAlignmentLockOffWithScale(const Brush& cube)
{
  const vm::vec3d mins(cube.bounds().min);

  // translate the cube mins to the origin, scale by 2 in the X axis, then translate
  // back
  const vm::mat4x4d transform = vm::translation_matrix(mins)
                                * vm::scaling_matrix(vm::vec3d{2, 1, 1})
                                * vm::translation_matrix(-1.0 * mins);
  const auto origFaceIndex = cube.findFace(vm::vec3d{0, -1, 0});
  REQUIRE(origFaceIndex);
  const auto& origFace = cube.face(*origFaceIndex);

  // transform the face (alignment lock off)
  auto face = origFace;
  REQUIRE(face.transform(transform, false).is_success());
  face.resetUVCoordSystemCache();

  // get UV at mins; should be equal
  const auto left_origTC = origFace.uvCoords(mins);
  const auto left_transformedTC = face.uvCoords(mins);
  CHECK(uvCoordsEqual(left_origTC, left_transformedTC));

  // get UVs at mins, plus the X size of the cube
  const auto right_origTC =
    origFace.uvCoords(mins + vm::vec3d{cube.bounds().size().x(), 0, 0});
  const auto right_transformedTC =
    face.uvCoords(mins + vm::vec3d{2.0 * cube.bounds().size().x(), 0, 0});

  // this assumes that the U axis of the material was scaled (i.e. the material is
  // oriented upright)
  const auto orig_U_width = right_origTC - left_origTC;
  const auto transformed_U_width = right_transformedTC - left_transformedTC;

  CHECK(transformed_U_width.x() == vm::approx{orig_U_width.x() * 2.0f});
  CHECK(transformed_U_width.y() == vm::approx{orig_U_width.y()});
}

} // namespace

TEST_CASE("BrushFaceTest.constructWithValidPoints")
{
  const auto p0 = vm::vec3d{0, 0, 4};
  const auto p1 = vm::vec3d{1, 0, 4};
  const auto p2 = vm::vec3d{0, -1, 4};

  const auto attribs = BrushFaceAttributes{""};
  auto face =
    BrushFace::create(
      p0, p1, p2, attribs, std::make_unique<ParaxialUVCoordSystem>(p0, p1, p2, attribs))
    | kdl::value();
  CHECK(face.points()[0] == vm::approx{p0});
  CHECK(face.points()[1] == vm::approx{p1});
  CHECK(face.points()[2] == vm::approx{p2});
  CHECK(face.boundary().normal == vm::approx{vm::vec3d{0, 0, 1}});
  CHECK(face.boundary().distance == 4.0);
}

TEST_CASE("BrushFaceTest.constructWithColinearPoints")
{
  const auto p0 = vm::vec3d{0, 0, 4};
  const auto p1 = vm::vec3d{1, 0, 4};
  const auto p2 = vm::vec3d{2, 0, 4};

  const auto attribs = BrushFaceAttributes{""};
  CHECK_FALSE(
    BrushFace::create(
      p0, p1, p2, attribs, std::make_unique<ParaxialUVCoordSystem>(p0, p1, p2, attribs))
      .is_success());
}

TEST_CASE("BrushFaceTest.materialUsageCount")
{
  const auto p0 = vm::vec3d{0, 0, 4};
  const auto p1 = vm::vec3d{1, 0, 4};
  const auto p2 = vm::vec3d{0, -1, 4};
  auto material =
    Assets::Material{"testMaterial", createTextureResource(Assets::Texture{64, 64})};
  auto material2 =
    Assets::Material{"testMaterial2", createTextureResource(Assets::Texture{64, 64})};

  CHECK(material.usageCount() == 0u);
  CHECK(material2.usageCount() == 0u);

  auto attribs = BrushFaceAttributes{""};
  {
    // test constructor
    auto face =
      BrushFace::create(
        p0, p1, p2, attribs, std::make_unique<ParaxialUVCoordSystem>(p0, p1, p2, attribs))
      | kdl::value();
    CHECK(material.usageCount() == 0u);

    // test setMaterial
    face.setMaterial(&material);
    CHECK(material.usageCount() == 1u);
    CHECK(material2.usageCount() == 0u);

    {
      // test copy constructor
      const auto clone = face;
      CHECK(material.usageCount() == 2u);
    }

    // test destructor
    CHECK(material.usageCount() == 1u);

    // test setMaterial with different material
    face.setMaterial(&material2);
    CHECK(material.usageCount() == 0u);
    CHECK(material2.usageCount() == 1u);

    // test setMaterial with the same material
    face.setMaterial(&material2);
    CHECK(material2.usageCount() == 1u);
  }

  CHECK(material.usageCount() == 0u);
  CHECK(material2.usageCount() == 0u);
}

TEST_CASE("BrushFaceTest.projectedArea")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  auto brush = builder.createCuboid(
                 vm::bbox3d(vm::vec3d{-64, -64, -64}, vm::vec3d{64, 64, 64}), "material")
               | kdl::value();
  REQUIRE(
    brush
      .transform(worldBounds, vm::rotation_matrix(0.0, 0.0, vm::to_radians(45.0)), false)
      .is_success());

  const auto& face = brush.faces().front();
  REQUIRE(face.boundary().normal.z() == vm::approx{0.0});
  REQUIRE(face.area() == vm::approx{128.0 * 128.0});

  const auto expectedSize = std::cos(vm::to_radians(45.0)) * 128.0 * 128.0;
  CHECK(face.projectedArea(vm::axis::x) == vm::approx{expectedSize});
  CHECK(face.projectedArea(vm::axis::y) == vm::approx{expectedSize});
  CHECK(face.projectedArea(vm::axis::z) == vm::approx{0.0});
}

TEST_CASE("BrushFaceTest.testSetRotation_Paraxial")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  Assets::Material material(
    "testMaterial", createTextureResource(Assets::Texture{64, 64}));

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto cube = builder.createCube(128.0, "") | kdl::value();
  auto& face = cube.faces().front();

  // This face's UV normal is in the same direction as the face normal
  const auto uvNormal = vm::normalize(vm::cross(face.uAxis(), face.vAxis()));

  const auto rot45 = vm::quatd{uvNormal, vm::to_radians(45.0)};
  const auto newXAxis = vm::vec3d{rot45 * face.uAxis()};
  const auto newYAxis = vm::vec3d{rot45 * face.vAxis()};

  auto attributes = face.attributes();
  attributes.setRotation(-45.0f);
  face.setAttributes(attributes);

  CHECK(face.uAxis() == vm::approx{newXAxis});
  CHECK(face.vAxis() == vm::approx{newYAxis});
}

TEST_CASE("BrushFaceTest.testAlignmentLock_Paraxial")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  auto material =
    Assets::Material{"testMaterial", createTextureResource(Assets::Texture{64, 64})};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto cube = builder.createCube(128.0, "") | kdl::value();

  for (auto& face : cube.faces())
  {
    face.setMaterial(&material);
    checkAlignmentLockForFace(face, false);
  }

  checkAlignmentLockOffWithVerticalFlip(cube);
  checkAlignmentLockOffWithScale(cube);
}

TEST_CASE("BrushFaceTest.testAlignmentLock_Parallel")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  auto material =
    Assets::Material{"testMaterial", createTextureResource(Assets::Texture{64, 64})};

  auto builder = BrushBuilder{MapFormat::Valve, worldBounds};
  auto cube = builder.createCube(128.0, "") | kdl::value();

  for (auto& face : cube.faces())
  {
    face.setMaterial(&material);
    checkAlignmentLockForFace(face, true);
  }

  checkAlignmentLockOffWithVerticalFlip(cube);
  checkAlignmentLockOffWithScale(cube);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2001
TEST_CASE("BrushFaceTest.testValveRotation")
{
  const auto data = R"(
{
  "classname" "worldspawn"
  {
    ( 24 8 48 ) ( 32 16 -16 ) ( 24 -8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1
    ( 8 -8 48 ) ( -0 -16 -16 ) ( 8 8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1
    ( 8 8 48 ) ( -0 16 -16 ) ( 24 8 48 ) tlight11 [ 1 0 0 -0 ] [ 0 0 -1 56 ] -0 1 1
    ( 24 -8 48 ) ( 32 -16 -16 ) ( 8 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 0 -1 56 ] -0 1 1
    ( 8 -8 48 ) ( 8 8 48 ) ( 24 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 -1 0 48 ] -0 1 1
    ( -0 16 -16 ) ( -0 -16 -16 ) ( 32 16 -16 ) tlight11 [ -1 0 0 -0 ] [ 0 -1 0 48 ] -0 1 1
  }
}
)";

  const auto worldBounds = vm::bbox3d{4096.0};

  auto status = IO::TestParserStatus{};
  auto nodes = IO::NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status);
  auto* pyramidLight = dynamic_cast<BrushNode*>(nodes.at(0)->children().at(0));
  REQUIRE(pyramidLight != nullptr);

  auto brush = pyramidLight->brush();

  // find the faces
  BrushFace* negXFace = nullptr;
  for (auto& face : brush.faces())
  {
    if (vm::get_abs_max_component_axis(face.boundary().normal) == vm::vec3d{-1, 0, 0})
    {
      REQUIRE(negXFace == nullptr);
      negXFace = &face;
    }
  }
  REQUIRE(negXFace != nullptr);

  CHECK(negXFace->uAxis() == vm::vec3d{0, 1, 0});
  CHECK(negXFace->vAxis() == vm::vec3d{0, 0, -1});

  // This face's UV normal is in the same direction as the face normal
  const auto uvNormal = vm::normalize(vm::cross(negXFace->uAxis(), negXFace->vAxis()));
  CHECK(vm::dot(uvNormal, vm::vec3d{negXFace->boundary().normal}) > 0.0);

  const auto rot45 = vm::quatd{uvNormal, vm::to_radians(45.0)};
  const auto newXAxis = vm::vec3d{rot45 * negXFace->uAxis()};
  const auto newYAxis = vm::vec3d{rot45 * negXFace->vAxis()};

  // Rotate by 45 degrees CCW
  CHECK(negXFace->attributes().rotation() == vm::approx{0.0f});
  negXFace->rotateUV(45.0);
  CHECK(negXFace->attributes().rotation() == vm::approx{45.0f});

  CHECK(negXFace->uAxis() == vm::approx{newXAxis});
  CHECK(negXFace->vAxis() == vm::approx{newYAxis});

  kdl::vec_clear_and_delete(nodes);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/1995
TEST_CASE("BrushFaceTest.testCopyUVCoordSystem")
{
  const auto data = R"(
{
  "classname" "worldspawn"
  {
    ( 24 8 48 ) ( 32 16 -16 ) ( 24 -8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1
    ( 8 -8 48 ) ( -0 -16 -16 ) ( 8 8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1
    ( 8 8 48 ) ( -0 16 -16 ) ( 24 8 48 ) tlight11 [ 1 0 0 -0 ] [ 0 0 -1 56 ] -0 1 1
    ( 24 -8 48 ) ( 32 -16 -16 ) ( 8 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 0 -1 56 ] -0 1 1
    ( 8 -8 48 ) ( 8 8 48 ) ( 24 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 -1 0 48 ] -0 1 1
    ( -0 16 -16 ) ( -0 -16 -16 ) ( 32 16 -16 ) tlight11 [ -1 0 0 -0 ] [ 0 -1 0 48 ] -0 1 1
  }
}
)";

  const auto worldBounds = vm::bbox3d{4096.0};

  auto status = IO::TestParserStatus{};

  auto nodes = IO::NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status);
  auto* pyramidLight = dynamic_cast<BrushNode*>(nodes.at(0)->children().at(0));
  REQUIRE(pyramidLight != nullptr);

  auto brush = pyramidLight->brush();

  // find the faces
  BrushFace* negYFace = nullptr;
  BrushFace* posXFace = nullptr;
  for (auto& face : brush.faces())
  {
    if (vm::get_abs_max_component_axis(face.boundary().normal) == vm::vec3d{0, -1, 0})
    {
      REQUIRE(negYFace == nullptr);
      negYFace = &face;
    }
    else if (vm::get_abs_max_component_axis(face.boundary().normal) == vm::vec3d{1, 0, 0})
    {
      REQUIRE(posXFace == nullptr);
      posXFace = &face;
    }
  }
  REQUIRE(negYFace != nullptr);
  REQUIRE(posXFace != nullptr);

  CHECK(negYFace->uAxis() == vm::vec3d{1, 0, 0});
  CHECK(negYFace->vAxis() == vm::vec3d{0, 0, -1});

  auto snapshot = negYFace->takeUVCoordSystemSnapshot();

  // copy texturing from the negYFace to posXFace using the rotation method
  posXFace->copyUVCoordSystemFromFace(
    *snapshot, negYFace->attributes(), negYFace->boundary(), WrapStyle::Rotation);
  CHECK(
    posXFace->uAxis()
    == vm::approx{
      vm::vec3d{0.030303030303030123, 0.96969696969696961, -0.24242424242424243}});
  CHECK(
    posXFace->vAxis()
    == vm::approx{
      vm::vec3d{-0.0037296037296037088, -0.24242424242424243, -0.97016317016317011}});

  // copy texturing from the negYFace to posXFace using the projection method
  posXFace->copyUVCoordSystemFromFace(
    *snapshot, negYFace->attributes(), negYFace->boundary(), WrapStyle::Projection);
  CHECK(posXFace->uAxis() == vm::approx{vm::vec3d{0, -1, 0}});
  CHECK(posXFace->vAxis() == vm::approx{vm::vec3d{0, 0, -1}});

  kdl::vec_clear_and_delete(nodes);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2315
TEST_CASE("BrushFaceTest.move45DegreeFace")
{
  const auto data = R"(
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 32 -64 16 ) ( 48 -48 16 ) ( 48 -48 144 ) __TB_empty [ -0.707107 -0.707107 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)";

  const auto worldBounds = vm::bbox3d{4096.0};

  auto status = IO::TestParserStatus{};

  auto nodes = IO::NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status);
  auto* brushNode = dynamic_cast<BrushNode*>(nodes.at(0)->children().at(0));
  CHECK(brushNode != nullptr);

  auto brush = brushNode->brush();

  // find the face
  const auto angledFaceIndex =
    brush.findFace(vm::vec3d{-0.70710678118654746, 0.70710678118654746, 0});
  REQUIRE(angledFaceIndex);

  CHECK(brush
          .moveBoundary(
            worldBounds,
            *angledFaceIndex,
            vm::vec3d{-7.9999999999999973, 7.9999999999999973, 0},
            true)
          .is_success());

  kdl::vec_clear_and_delete(nodes);
}

TEST_CASE("BrushFaceTest.formatConversion")
{
  const auto worldBounds = vm::bbox3d{4096.0};

  auto standardBuilder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto valveBuilder = BrushBuilder{MapFormat::Valve, worldBounds};

  auto material =
    Assets::Material{"testMaterial", createTextureResource(Assets::Texture{64, 64})};

  const auto startingCube = standardBuilder.createCube(128.0, "")
                            | kdl::transform([&](auto&& brush) {
                                for (size_t i = 0; i < brush.faceCount(); ++i)
                                {
                                  auto& face = brush.face(i);
                                  face.setMaterial(&material);
                                }
                                return std::forward<decltype(brush)>(brush);
                              })
                            | kdl::value();

  auto testTransform = [&](const auto& transform) {
    auto standardCube = startingCube;
    REQUIRE(standardCube.transform(worldBounds, transform, true).is_success());
    CHECK(
      dynamic_cast<const ParaxialUVCoordSystem*>(&standardCube.face(0).uvCoordSystem()));

    const auto valveCube = standardCube.convertToParallel();
    CHECK(dynamic_cast<const ParallelUVCoordSystem*>(&valveCube.face(0).uvCoordSystem()));
    checkBrushUVsEqual(standardCube, valveCube);

    const auto standardCubeRoundTrip = valveCube.convertToParaxial();
    CHECK(dynamic_cast<const ParaxialUVCoordSystem*>(
      &standardCubeRoundTrip.face(0).uvCoordSystem()));
    checkBrushUVsEqual(standardCube, standardCubeRoundTrip);
  };

  // NOTE: intentionally include the shear/multi-axis rotations which won't work
  // properly on Standard. We're not testing alignment lock, just generating interesting
  // brushes to test Standard -> Valve -> Standard round trip, so it doesn't matter if
  // alignment lock works.
  doWithAlignmentLockTestTransforms(true, testTransform);
}

TEST_CASE("BrushFaceTest.flipUV")
{
  const auto data = R"(
// entity 0
{
"mapversion" "220"
"classname" "worldspawn"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) skip [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) skip [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) skip [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) hint [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) skip [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) skip [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)";

  const auto worldBounds = vm::bbox3d{4096.0};

  auto status = IO::TestParserStatus{};

  auto nodes = IO::NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status);
  auto* brushNode = dynamic_cast<BrushNode*>(nodes.at(0)->children().at(0));
  REQUIRE(brushNode != nullptr);

  auto brush = brushNode->brush();
  auto& face = brush.face(*brush.findFace(vm::vec3d{0, 0, 1}));
  CHECK(face.attributes().scale() == vm::vec2f{1, 1});

  SECTION("Default camera angle")
  {
    const auto cameraUp = vm::vec3d{0.284427, 0.455084, 0.843801};
    const auto cameraRight = vm::vec3d{0.847998, -0.529999, 0};

    SECTION("Left flip")
    {
      face.flipUV(cameraUp, cameraRight, vm::direction::left);
      CHECK(face.attributes().scale() == vm::vec2f{-1, 1});
    }

    SECTION("Up flip")
    {
      face.flipUV(cameraUp, cameraRight, vm::direction::up);
      CHECK(face.attributes().scale() == vm::vec2f{1, -1});
    }
  }

  SECTION("Camera is aimed at +x")
  {
    const auto cameraUp = vm::vec3d{0.419431, -0.087374, 0.903585};
    const auto cameraRight = vm::vec3d{-0.203938, -0.978984, 0};

    SECTION("left arrow (does vertical flip)")
    {
      face.flipUV(cameraUp, cameraRight, vm::direction::left);
      CHECK(face.attributes().scale() == vm::vec2f{1, -1});
    }

    SECTION("up arrow (does horizontal flip)")
    {
      face.flipUV(cameraUp, cameraRight, vm::direction::up);
      CHECK(face.attributes().scale() == vm::vec2f{-1, 1});
    }
  }
}

} // namespace tb::Model
