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

#include "TestParserStatus.h"
#include "base/Color.h"
#include "gl/Material.h"
#include "gl/MiniGl.h"
#include "gl/Texture.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushGeometry.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/MapFormat.h"
#include "mdl/NodeReader.h"
#include "mdl/ParallelUvCoordSystem.h"
#include "mdl/ParaxialUvCoordSystem.h"
#include "mdl/SurfaceAttributes.h"
#include "mdl/TestUtils.h"
#include "mdl/UvAttributes.h"

#include "kd/ranges/contains.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/task_manager.h"

#include "vm/approx.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/polygon.h"
#include "vm/ray.h"
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <algorithm>
#include <limits>
#include <optional>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
namespace
{

void getFaceVertsAndUvCoords(
  const BrushFace& face,
  std::vector<vm::vec3d>* vertPositions,
  std::vector<vm::vec2f>* vertUvCoords)
{
  for (const auto* vertex : face.vertices())
  {
    vertPositions->push_back(vertex->position());
    if (vertUvCoords)
    {
      vertUvCoords->push_back(face.uvCoords(vm::vec3d{vertex->position()}));
    }
  }
}

void resetFaceUvAlignment(BrushFace& face)
{
  REQUIRE(face.setUvAttributes({}).is_success());
  face.resetUvAxes();
}

/**
 * Assumes the UV's have been divided by the texture size.
 */
void checkUvListsEqual(
  const std::vector<vm::vec2f>& uvs,
  const std::vector<vm::vec2f>& transformedVertUvs,
  const BrushFace& face)
{
  // We require a material, so that face.textureSize() returns a correct value and not
  // 1x1, and so face.uvCoords() returns UV's that are divided by the texture size.
  // Otherwise, the UV comparisons below could spuriously pass.
  REQUIRE(face.material() != nullptr);

  CHECK(uvListsEqual(uvs, transformedVertUvs));
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
  resetFaceUvAlignment(face);
  REQUIRE(face.transform(transform, false));
  face.resetUvCoordSystemCache();

  // reset alignment, transform the face (alignment lock off), then reset the alignment
  // again
  auto resetFace = origFace;
  resetFaceUvAlignment(resetFace);
  REQUIRE(resetFace.transform(transform, false));
  resetFaceUvAlignment(resetFace);

  // UVs of the verts of `face` and `resetFace` should be the same now

  auto verts = std::vector<vm::vec3d>{};
  getFaceVertsAndUvCoords(origFace, &verts, nullptr);

  // transform the verts
  auto transformedVerts = std::vector<vm::vec3d>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    transformedVerts.push_back(transform * verts[i]);
  }

  // get UV of each transformed vert using `face` and `resetFace`
  auto face_Uvs = std::vector<vm::vec2f>{};
  auto resetFace_Uvs = std::vector<vm::vec2f>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    face_Uvs.push_back(face.uvCoords(transformedVerts[i]));
    resetFace_Uvs.push_back(resetFace.uvCoords(transformedVerts[i]));
  }

  checkUvListsEqual(face_Uvs, resetFace_Uvs, face);
}

void checkFaceUvsEqual(const BrushFace& face, const BrushFace& other)
{
  auto verts = std::vector<vm::vec3d>{};
  auto faceUvs = std::vector<vm::vec2f>{};
  auto otherFaceUvs = std::vector<vm::vec2f>{};

  for (const auto* vertex : face.vertices())
  {
    verts.push_back(vertex->position());

    const auto position = vm::vec3d{vertex->position()};
    faceUvs.push_back(face.uvCoords(position));
    otherFaceUvs.push_back(other.uvCoords(position));
  }

  checkUvListsEqual(faceUvs, otherFaceUvs, face);
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
  getFaceVertsAndUvCoords(origFace, &verts, &uvs);
  CHECK(verts.size() >= 3u);

  // transform the face
  auto face = origFace;
  REQUIRE(face.transform(transform, true));
  face.resetUvCoordSystemCache();

  // transform the verts
  auto transformedVerts = std::vector<vm::vec3d>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    transformedVerts.push_back(transform * verts[i]);
  }

  // ask the transformed face for the UVs at the transformed verts
  auto transformedVertUvs = std::vector<vm::vec2f>{};
  for (size_t i = 0; i < verts.size(); i++)
  {
    transformedVertUvs.push_back(face.uvCoords(transformedVerts[i]));
  }

  checkUvListsEqual(uvs, transformedVertUvs, face);
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
  // ParallelUvCoordSystem
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
  REQUIRE(face.transform(transform, false));
  face.resetUvCoordSystemCache();

  // UVs of the verts of `face` and `origFace` should be the same now

  // get UV of each vert using `face` and `resetFace`
  auto face_Uvs = std::vector<vm::vec2f>{};
  auto origFace_Uvs = std::vector<vm::vec2f>{};
  for (const auto vert : origFace.vertices())
  {
    face_Uvs.push_back(face.uvCoords(vert->position()));
    origFace_Uvs.push_back(origFace.uvCoords(vert->position()));
  }

  checkUvListsEqual(face_Uvs, origFace_Uvs, face);
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
  REQUIRE(face.transform(transform, false));
  face.resetUvCoordSystemCache();

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

/**
 * Runs the given lambda of type `const BrushFace& -> void` with each face of a Standard
 * format cube transformed by each of the alignment lock test transforms.
 */
template <class L>
void doWithTransformedStandardCubeFaces(L&& lambda)
{
  const auto worldBounds = vm::bbox3d{4096.0};
  auto material =
    gl::Material{"testMaterial", gl::createTextureResource(gl::Texture{64, 64})};

  auto startingCube =
    BrushBuilder{MapFormat::Standard, worldBounds}.createCube(128.0, "") | kdl::value();
  for (auto& face : startingCube.faces())
  {
    face.setMaterial(&material);
  }

  // NOTE: intentionally include the shear/multi-axis rotations which won't work properly
  // on Standard. We're not testing alignment lock, just generating interesting faces, so
  // it doesn't matter if alignment lock works.
  doWithAlignmentLockTestTransforms(true, [&](const vm::mat4x4d& transform) {
    auto cube = startingCube;
    REQUIRE(cube.transform(worldBounds, transform, true));

    for (const auto& face : cube.faces())
    {
      lambda(face);
    }
  });
}

/**
 * Creates a face in the plane z = 4 with normal +Z.
 */
BrushFace createHorizontalFace(
  const MapFormat mapFormat, const UvAttributes& uvAttributes = UvAttributes{})
{
  return BrushFace::create(
           {0, 0, 4},
           {1, 0, 4},
           {0, -1, 4},
           "",
           uvAttributes,
           SurfaceAttributes{},
           mapFormat)
         | kdl::value();
}

/**
 * Creates a face in the plane z = 64 + x / 2, whose normal is closest to +Z.
 */
BrushFace createSlantedFace(
  const MapFormat mapFormat, const UvAttributes& uvAttributes = UvAttributes{})
{
  return BrushFace::create(
           {0, 0, 64},
           {0, 1, 64},
           {1, 0, 64.5},
           "",
           uvAttributes,
           SurfaceAttributes{},
           mapFormat)
         | kdl::value();
}

gl::Material createMaterial(const gl::EmbeddedDefaults& embeddedDefaults)
{
  return gl::Material{
    "material",
    gl::createTextureResource(gl::Texture{
      64, 64, RgbaF{}, GL_RGBA, embeddedDefaults, std::vector<gl::TextureBuffer>{}})};
}

} // namespace

TEST_CASE("BrushFace")
{
  auto taskManager = kdl::task_manager{};

  SECTION("copy constructor")
  {
    auto material =
      gl::Material{"testMaterial", gl::createTextureResource(gl::Texture{64, 64})};

    auto face = BrushFace::create(
                  {0, 0, 4},
                  {1, 0, 4},
                  {0, -1, 4},
                  "",
                  UvAttributes{},
                  SurfaceAttributes{},
                  MapFormat::Standard)
                | kdl::value();
    face.setMaterial(&material);
    REQUIRE(material.usageCount() == 1u);

    const auto clone = face;
    CHECK(material.usageCount() == 2u);
  }

  SECTION("destructor")
  {
    auto material =
      gl::Material{"testMaterial", gl::createTextureResource(gl::Texture{64, 64})};

    {
      auto face = BrushFace::create(
                    {0, 0, 4},
                    {1, 0, 4},
                    {0, -1, 4},
                    "",
                    UvAttributes{},
                    SurfaceAttributes{},
                    MapFormat::Standard)
                  | kdl::value();
      face.setMaterial(&material);
      REQUIRE(material.usageCount() == 1u);
    }

    CHECK(material.usageCount() == 0u);
  }

  SECTION("create")
  {
    SECTION("with valid points")
    {
      const auto p0 = vm::vec3d{0, 0, 4};
      const auto p1 = vm::vec3d{1, 0, 4};
      const auto p2 = vm::vec3d{0, -1, 4};

      auto face = BrushFace::create(
                    p0,
                    p1,
                    p2,
                    "",
                    UvCoordSystem{
                      ParaxialUvCoordSystem::createFromPoints(p0, p1, p2, UvAttributes{})
                      | kdl::value()},
                    SurfaceAttributes{})
                  | kdl::value();
      CHECK(face.points()[0] == vm::approx{p0});
      CHECK(face.points()[1] == vm::approx{p1});
      CHECK(face.points()[2] == vm::approx{p2});
      CHECK(face.boundary().normal == vm::approx{vm::vec3d{0, 0, 1}});
      CHECK(face.boundary().distance == 4.0);
    }

    SECTION("with colinear points")
    {
      const auto p0 = vm::vec3d{0, 0, 4};
      const auto p1 = vm::vec3d{1, 0, 4};
      const auto p2 = vm::vec3d{2, 0, 4};

      // the UV coordinate system's own axes are independent of the (colinear) face
      // points here -- this exercises BrushFace::create's own plane-degeneracy check
      CHECK(!BrushFace::create(
        p0,
        p1,
        p2,
        "",
        UvCoordSystem{
          ParaxialUvCoordSystem::createFromNormal(vm::vec3d{0, 0, 1}, UvAttributes{})
          | kdl::value()},
        SurfaceAttributes{}));
    }

    SECTION("with an invalid UV attribute")
    {
      const auto p0 = vm::vec3d{0, 0, 4};
      const auto p1 = vm::vec3d{1, 0, 4};
      const auto p2 = vm::vec3d{0, -1, 4};
      const auto nan = std::numeric_limits<float>::quiet_NaN();
      const auto invalidUvAttributes = UvAttributes{{0, 0}, {1, 1}, nan};

      // Standard format goes through ParaxialUvCoordSystem
      CHECK(!BrushFace::create(
        p0, p1, p2, "", invalidUvAttributes, SurfaceAttributes{}, MapFormat::Standard));

      // Valve format goes through ParallelUvCoordSystem
      CHECK(!BrushFace::create(
        p0, p1, p2, "", invalidUvAttributes, SurfaceAttributes{}, MapFormat::Valve));
    }
  }

  SECTION("createFromStandard")
  {
    const auto uvAttributes =
      UvAttributes{.offset = {1, 2}, .scale = {3, 4}, .rotation = 30.0f};
    const auto surfaceAttributes = SurfaceAttributes{.flags = 1};

    SECTION("for a Standard format map")
    {
      const auto p0 = vm::vec3d{0, 0, 4};
      const auto p1 = vm::vec3d{1, 0, 4};
      const auto p2 = vm::vec3d{0, -1, 4};

      const auto face =
        BrushFace::createFromStandard(
          p0, p1, p2, "material", uvAttributes, surfaceAttributes, MapFormat::Standard)
        | kdl::value();

      CHECK(face.materialName() == "material");
      CHECK(face.uvCoordSystem().is<ParaxialUvCoordSystem>());
      CHECK(face.uvAttributes() == uvAttributes);
      CHECK(face.surfaceAttributes() == surfaceAttributes);
    }

    SECTION("for a Valve format map")
    {
      const auto p0 = vm::vec3d{0, 0, 4};
      const auto p1 = vm::vec3d{1, 0, 4};
      const auto p2 = vm::vec3d{0, -1, 4};

      const auto face =
        BrushFace::createFromStandard(
          p0, p1, p2, "material", uvAttributes, surfaceAttributes, MapFormat::Valve)
        | kdl::value();

      CHECK(face.materialName() == "material");
      CHECK(face.uvCoordSystem().is<ParallelUvCoordSystem>());
      CHECK(face.uvAttributes() == uvAttributes);
      CHECK(face.surfaceAttributes() == surfaceAttributes);
    }

    SECTION("with an extreme scale")
    {
      const auto p0 = vm::vec3d{0, 0, 4};
      const auto p1 = vm::vec3d{1, 0, 4};
      const auto p2 = vm::vec3d{0, -1, 4};

      const auto extremeUvAttributes = UvAttributes{{0, 0}, {1e30f, 1e30f}, 0.0f};

      // converting from Paraxial to Parallel preserves the (here, extreme) UV attributes
      CHECK(!BrushFace::createFromStandard(
        p0, p1, p2, "", extremeUvAttributes, SurfaceAttributes{}, MapFormat::Valve));
    }
  }

  SECTION("createFromValve")
  {
    const auto p0 = vm::vec3d{0, 0, 4};
    const auto p1 = vm::vec3d{1, 0, 4};
    const auto p2 = vm::vec3d{0, -1, 4};
    const auto uvAttributes = UvAttributes{.offset = {1, 2}, .scale = {3, 4}};

    SECTION("for a Valve format map")
    {
      const auto uAxis = vm::vec3d{0.6, 0.8, 0};
      const auto vAxis = vm::vec3d{0.8, -0.6, 0};

      const auto face = BrushFace::createFromValve(
                          p0,
                          p1,
                          p2,
                          "material",
                          uvAttributes,
                          SurfaceAttributes{},
                          uAxis,
                          vAxis,
                          MapFormat::Valve)
                        | kdl::value();

      CHECK(face.materialName() == "material");
      CHECK(face.uvCoordSystem().is<ParallelUvCoordSystem>());
      CHECK(face.uvAttributes() == uvAttributes);
      CHECK(face.uAxis() == uAxis);
      CHECK(face.vAxis() == vAxis);
    }

    SECTION("for a Standard format map")
    {
      const auto uAxis = vm::vec3d{0.6, 0.8, 0};
      const auto vAxis = vm::vec3d{0.8, -0.6, 0};

      const auto valveFace = BrushFace::createFromValve(
                               p0,
                               p1,
                               p2,
                               "material",
                               uvAttributes,
                               SurfaceAttributes{},
                               uAxis,
                               vAxis,
                               MapFormat::Valve)
                             | kdl::value();
      const auto face = BrushFace::createFromValve(
                          p0,
                          p1,
                          p2,
                          "material",
                          uvAttributes,
                          SurfaceAttributes{},
                          uAxis,
                          vAxis,
                          MapFormat::Standard)
                        | kdl::value();

      CHECK(face.materialName() == "material");
      CHECK(face.uvCoordSystem().is<ParaxialUvCoordSystem>());

      // the UV axes are converted to paraxial UV attributes that preserve the UVs
      for (const auto& point : {p0, p1, p2, vm::vec3d{10, 20, 4}})
      {
        CAPTURE(point);
        CHECK(face.uvCoords(point) == vm::approx{valveFace.uvCoords(point), 0.001f});
      }
    }

    SECTION("with degenerate axes")
    {
      const auto uAxis = vm::vec3d{1, 0, 0};

      // Valve format passes the axes through to ParallelUvCoordSystem unchanged
      CHECK(!BrushFace::createFromValve(
        p0,
        p1,
        p2,
        "",
        UvAttributes{},
        SurfaceAttributes{},
        uAxis,
        uAxis,
        MapFormat::Valve));
    }
  }

  SECTION("setMaterialName")
  {
    auto face = createHorizontalFace(MapFormat::Standard);
    REQUIRE(face.materialName() == "");

    SECTION("returns true if the material name changed")
    {
      CHECK(face.setMaterialName("material"));
      CHECK(face.materialName() == "material");
    }

    SECTION("returns false if the material name did not change")
    {
      CHECK(!face.setMaterialName(""));
      CHECK(face.materialName() == "");
    }
  }

  SECTION("setMaterial")
  {
    auto material =
      gl::Material{"testMaterial", gl::createTextureResource(gl::Texture{64, 64})};
    auto material2 =
      gl::Material{"testMaterial2", gl::createTextureResource(gl::Texture{64, 64})};

    auto face = BrushFace::create(
                  {0, 0, 4},
                  {1, 0, 4},
                  {0, -1, 4},
                  "",
                  UvAttributes{},
                  SurfaceAttributes{},
                  MapFormat::Standard)
                | kdl::value();
    CHECK(material.usageCount() == 0u);
    CHECK(material2.usageCount() == 0u);

    face.setMaterial(&material);
    CHECK(material.usageCount() == 1u);
    CHECK(material2.usageCount() == 0u);

    // set a different material
    face.setMaterial(&material2);
    CHECK(material.usageCount() == 0u);
    CHECK(material2.usageCount() == 1u);

    // set the same material
    face.setMaterial(&material2);
    CHECK(material2.usageCount() == 1u);
  }

  SECTION("resolvedSurfaceContents")
  {
    auto material =
      createMaterial(gl::Q2EmbeddedDefaults{.flags = 1, .contents = 2, .value = 3});
    auto face = createHorizontalFace(MapFormat::Quake2);

    SECTION("returns the face's contents if it has any")
    {
      face.setMaterial(&material);
      face.setSurfaceAttributes({.contents = 4});
      CHECK(face.resolvedSurfaceContents() == 4);
    }

    SECTION("returns the material's default contents if the face has none")
    {
      face.setMaterial(&material);
      CHECK(face.resolvedSurfaceContents() == 2);
    }

    SECTION("returns 0 if the face has no contents and no material")
    {
      CHECK(face.resolvedSurfaceContents() == 0);
    }
  }

  SECTION("resolvedSurfaceFlags")
  {
    auto material =
      createMaterial(gl::Q2EmbeddedDefaults{.flags = 1, .contents = 2, .value = 3});
    auto face = createHorizontalFace(MapFormat::Quake2);

    SECTION("returns the face's flags if it has any")
    {
      face.setMaterial(&material);
      face.setSurfaceAttributes({.flags = 4});
      CHECK(face.resolvedSurfaceFlags() == 4);
    }

    SECTION("returns the material's default flags if the face has none")
    {
      face.setMaterial(&material);
      CHECK(face.resolvedSurfaceFlags() == 1);
    }

    SECTION("returns 0 if the face has no flags and no material")
    {
      CHECK(face.resolvedSurfaceFlags() == 0);
    }
  }

  SECTION("resolvedSurfaceValue")
  {
    auto material =
      createMaterial(gl::Q2EmbeddedDefaults{.flags = 1, .contents = 2, .value = 3});
    auto face = createHorizontalFace(MapFormat::Quake2);

    SECTION("returns the face's value if it has one")
    {
      face.setMaterial(&material);
      face.setSurfaceAttributes({.value = 4.0f});
      CHECK(face.resolvedSurfaceValue() == 4.0f);
    }

    SECTION("returns the material's default value if the face has none")
    {
      face.setMaterial(&material);
      CHECK(face.resolvedSurfaceValue() == 3.0f);
    }

    SECTION("returns 0 if the face has no value and no material")
    {
      CHECK(face.resolvedSurfaceValue() == 0.0f);
    }
  }

  SECTION("projectedArea")
  {
    const auto worldBounds = vm::bbox3d{8192.0};
    const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    auto brush =
      builder.createCuboid(
        vm::bbox3d(vm::vec3d{-64, -64, -64}, vm::vec3d{64, 64, 64}), "material")
      | kdl::value();
    REQUIRE(brush.transform(
      worldBounds, vm::rotation_matrix(0.0, 0.0, vm::to_radians(45.0)), false));

    const auto& face = brush.faces().front();
    REQUIRE(face.boundary().normal.z() == vm::approx{0.0});
    REQUIRE(face.area() == vm::approx{128.0 * 128.0});

    const auto expectedSize = std::cos(vm::to_radians(45.0)) * 128.0 * 128.0;
    CHECK(face.projectedArea(vm::axis::x) == vm::approx{expectedSize});
    CHECK(face.projectedArea(vm::axis::y) == vm::approx{expectedSize});
    CHECK(face.projectedArea(vm::axis::z) == vm::approx{0.0});
  }

  SECTION("bounds")
  {
    const auto worldBounds = vm::bbox3d{8192.0};
    const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    auto brush =
      builder.createCuboid(
        vm::bbox3d{vm::vec3d{-32, -64, -16}, vm::vec3d{32, 64, 16}}, "material")
      | kdl::value();

    const auto faceIndex = brush.findFace(vm::vec3d{1, 0, 0});
    REQUIRE(faceIndex);
    const auto& face = brush.face(*faceIndex);

    CHECK(face.bounds() == vm::bbox3d{vm::vec3d{32, -64, -16}, vm::vec3d{32, 64, 16}});
  }

  SECTION("boundsCenter")
  {
    const auto worldBounds = vm::bbox3d{8192.0};
    const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    SECTION("axis aligned face")
    {
      auto brush =
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{-32, -64, -16}, vm::vec3d{32, 64, 16}}, "material")
        | kdl::value();

      const auto faceIndex = brush.findFace(vm::vec3d{1, 0, 0});
      REQUIRE(faceIndex);
      const auto& face = brush.face(*faceIndex);

      CHECK(face.boundsCenter() == vm::approx{vm::vec3d{32, 0, 0}});
    }

    SECTION("face on a rotated brush")
    {
      auto brush =
        builder.createCuboid(
          vm::bbox3d{vm::vec3d{-32, -64, -16}, vm::vec3d{32, 64, 16}}, "material")
        | kdl::value();
      REQUIRE(brush.transform(
        worldBounds, vm::rotation_matrix(0.0, 0.0, vm::to_radians(45.0)), false));

      const auto& face = brush.faces().front();
      const auto vertexPositions = face.vertexPositions();

      auto expectedCenter = vm::vec3d{0, 0, 0};
      for (const auto& position : vertexPositions)
      {
        expectedCenter = expectedCenter + position;
      }
      expectedCenter = expectedCenter / double(vertexPositions.size());

      CHECK(face.boundsCenter() == vm::approx{expectedCenter});
    }
  }

  SECTION("coplanarWith")
  {
    const auto cube =
      BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}}.createCube(128.0, "")
      | kdl::value();
    const auto faceIndex = cube.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);
    const auto& face = cube.face(*faceIndex);

    CHECK(face.coplanarWith(face.boundary()));

    // a parallel plane
    CHECK(!face.coplanarWith(vm::plane3d{65.0, vm::vec3d{0, 0, 1}}));

    // a plane through the face's center, but with a different normal
    CHECK(!face.coplanarWith(
      vm::plane3d{face.center(), vm::normalize(vm::vec3d{0.1, 0, 1})}));

    // the inverted plane
    CHECK(!face.coplanarWith(face.boundary().flip()));
  }

  SECTION("setUvAttributes")
  {
    SECTION("rotates the UV axes (paraxial)")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      const gl::Material material(
        "testMaterial", gl::createTextureResource(gl::Texture{64, 64}));

      auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
      auto cube = builder.createCube(128.0, "") | kdl::value();
      auto& face = cube.faces().front();

      // This face's UV normal is in the same direction as the face normal
      const auto uvNormal = vm::normalize(vm::cross(face.uAxis(), face.vAxis()));

      const auto rot45 = vm::quatd{uvNormal, vm::to_radians(45.0)};
      const auto newXAxis = vm::vec3d{rot45 * face.uAxis()};
      const auto newYAxis = vm::vec3d{rot45 * face.vAxis()};

      REQUIRE(face.setUvAttributes({.rotation = -45.0f}).is_success());

      CHECK(face.uAxis() == vm::approx{newXAxis});
      CHECK(face.vAxis() == vm::approx{newYAxis});
    }
  }

  SECTION("copyAttributes")
  {
    const auto mapFormat = GENERATE(MapFormat::Standard, MapFormat::Valve);
    CAPTURE(mapFormat);

    const auto source =
      BrushFace::create(
        {0, 0, 64},
        {0, 1, 64},
        {1, 0, 64},
        "source",
        UvAttributes{.offset = {7, 3}, .scale = {0.5f, 2.0f}, .rotation = 30.0f},
        SurfaceAttributes{.flags = 4},
        mapFormat)
      | kdl::value();

    const auto p0 = vm::vec3d{48, 0, 0};
    const auto p1 = vm::vec3d{48, 64, 0};
    const auto p2 = vm::vec3d{0, 0, 48};

    auto face =
      BrushFace::create(p0, p1, p2, "", UvAttributes{}, SurfaceAttributes{}, mapFormat)
      | kdl::value();
    REQUIRE(face.copyAttributes(source).is_success());

    SECTION("copies the material and the UV and surface attributes")
    {
      CHECK(face.materialName() == "source");
      CHECK(face.uvAttributes() == source.uvAttributes());
      CHECK(face.surfaceAttributes() == source.surfaceAttributes());
    }

    // https://github.com/TrenchBroom/TrenchBroom/issues/5583
    SECTION("rotates the UV axes to match the copied rotation")
    {
      const auto expected =
        BrushFace::create(
          p0, p1, p2, "", source.uvAttributes(), SurfaceAttributes{}, mapFormat)
        | kdl::value();

      CHECK(face.uAxis() == vm::approx{expected.uAxis()});
      CHECK(face.vAxis() == vm::approx{expected.vAxis()});
    }
  }

  SECTION("transform")
  {
    SECTION("paraxial UV coordinate system")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      auto material =
        gl::Material{"testMaterial", gl::createTextureResource(gl::Texture{64, 64})};

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

    SECTION("parallel UV coordinate system")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      auto material =
        gl::Material{"testMaterial", gl::createTextureResource(gl::Texture{64, 64})};

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

    // https://github.com/TrenchBroom/TrenchBroom/issues/5582
    SECTION("parallel UV coordinate system keeps the rotation when translated")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      auto builder = BrushBuilder{MapFormat::Valve, worldBounds};
      auto cube = builder.createCube(128.0, "") | kdl::value();

      for (auto& face : cube.faces())
      {
        REQUIRE(face.setUvAttributes({.rotation = 30.0f}).is_success());
        REQUIRE(face.transform(vm::translation_matrix(vm::vec3d{16, 32, 8}), true));
        CHECK(face.uvAttributes().rotation == vm::approx{30.0f});
      }
    }

    SECTION("parallel UV coordinate system rotation follows the texture when rotated")
    {
      // The UV normals of the -X, -Z and +Y faces point in the same direction as their
      // face normals, the UV normals of the other faces point in the opposite direction
      const auto data = R"(
{
"classname" "worldspawn"
{
( 16 0 0 ) ( 16 1 0 ) ( 16 0 1 ) material [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 16 0 0 ) ( 16 0 1 ) ( 17 0 0 ) material [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 16 0 0 ) ( 17 0 0 ) ( 16 1 0 ) material [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 80 64 64 ) ( 80 65 64 ) ( 81 64 64 ) material [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 80 64 64 ) ( 81 64 64 ) ( 80 64 65 ) material [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 80 64 64 ) ( 80 64 65 ) ( 80 65 64 ) material [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)";

      const auto worldBounds = vm::bbox3d{8192.0};

      auto status = TestParserStatus{};
      auto nodes =
        NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status, taskManager);
      REQUIRE(nodes);

      auto* brushNode = dynamic_cast<BrushNode*>(nodes.value().at(0)->children()[0]);
      REQUIRE(brushNode != nullptr);

      auto brush = brushNode->brush();
      for (auto& face : brush.faces())
      {
        REQUIRE(face.setUvAttributes({.rotation = 30.0f}).is_success());
        const auto uAxis = face.uAxis();
        const auto vAxis = face.vAxis();

        const auto rotation =
          vm::rotation_matrix(face.boundary().normal, vm::to_radians(15.0));
        REQUIRE(face.transform(rotation, true));

        // setting the rotation back to its original value undoes the rotation
        REQUIRE(face.setUvAttributes({.rotation = 30.0f}).is_success());
        CHECK(face.uAxis() == vm::approx{uAxis});
        CHECK(face.vAxis() == vm::approx{vAxis});
      }
    }
  }

  SECTION("restoreUvCoordSystemSnapshot")
  {
    auto face = createHorizontalFace(MapFormat::Valve);
    const auto uAxis = face.uAxis();
    const auto vAxis = face.vAxis();

    const auto snapshot = face.takeUvCoordSystemSnapshot();
    REQUIRE(snapshot);

    REQUIRE(face.rotateUv(30.0f).is_success());
    REQUIRE(face.uAxis() != uAxis);

    face.restoreUvCoordSystemSnapshot(*snapshot);
    CHECK(face.uAxis() == uAxis);
    CHECK(face.vAxis() == vAxis);
  }

  SECTION("resetUvAxesToParaxial")
  {
    SECTION("resets parallel UV axes to the paraxial UV axes")
    {
      auto face = createHorizontalFace(MapFormat::Valve);
      REQUIRE(face.rotateUv(30.0f).is_success());

      face.resetUvAxesToParaxial();
      CHECK(face.uAxis() == vm::approx{vm::vec3d{1, 0, 0}});
      CHECK(face.vAxis() == vm::approx{vm::vec3d{0, -1, 0}});
    }

    SECTION("does not change paraxial UV axes")
    {
      auto face = createHorizontalFace(MapFormat::Standard);
      REQUIRE(face.rotateUv(30.0f).is_success());
      const auto uAxis = face.uAxis();
      const auto vAxis = face.vAxis();

      face.resetUvAxesToParaxial();
      CHECK(face.uAxis() == uAxis);
      CHECK(face.vAxis() == vAxis);
    }
  }

  SECTION("translateUv")
  {
    auto face = createHorizontalFace(MapFormat::Standard);
    REQUIRE(face.uAxis() == vm::approx{vm::vec3d{1, 0, 0}});
    REQUIRE(face.vAxis() == vm::approx{vm::vec3d{0, -1, 0}});

    // The U axis points along the camera's right axis, and the V axis points against
    // the camera's up axis.
    face.translateUv(vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, vm::vec2f{2, 3});
    CHECK(face.uvAttributes().offset == vm::vec2f{-2, 3});
  }

  SECTION("rotateUv")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/2001
    SECTION("rotates the UV axes of a slanted face (Valve format)")
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

      auto status = TestParserStatus{};
      auto nodes =
        NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status, taskManager);
      REQUIRE(nodes);

      auto* pyramidLight = dynamic_cast<BrushNode*>(nodes.value().at(0)->children()[0]);
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
      const auto uvNormal =
        vm::normalize(vm::cross(negXFace->uAxis(), negXFace->vAxis()));
      CHECK(vm::dot(uvNormal, vm::vec3d{negXFace->boundary().normal}) > 0.0);

      const auto rot45 = vm::quatd{uvNormal, vm::to_radians(45.0)};
      const auto newXAxis = vm::vec3d{rot45 * negXFace->uAxis()};
      const auto newYAxis = vm::vec3d{rot45 * negXFace->vAxis()};

      // Rotate by 45 degrees CCW
      CHECK(negXFace->uvAttributes().rotation == vm::approx{0.0f});
      REQUIRE(negXFace->rotateUv(45.0).is_success());
      CHECK(negXFace->uvAttributes().rotation == vm::approx{45.0f});

      CHECK(negXFace->uAxis() == vm::approx{newXAxis});
      CHECK(negXFace->vAxis() == vm::approx{newYAxis});
    }
  }

  SECTION("shearUv")
  {
    SECTION("shears parallel UV axes")
    {
      auto face = createHorizontalFace(MapFormat::Valve);
      REQUIRE(face.uAxis() == vm::approx{vm::vec3d{1, 0, 0}});
      REQUIRE(face.vAxis() == vm::approx{vm::vec3d{0, -1, 0}});

      face.shearUv(vm::vec2f{0.5f, 0.25f});
      CHECK(face.uAxis() == vm::approx{vm::vec3d{1, -0.25, 0}});
      CHECK(face.vAxis() == vm::approx{vm::vec3d{0.5, -1, 0}});
    }

    SECTION("does not change paraxial UV axes")
    {
      auto face = createHorizontalFace(MapFormat::Standard);
      const auto uAxis = face.uAxis();
      const auto vAxis = face.vAxis();

      face.shearUv(vm::vec2f{0.5f, 0.25f});
      CHECK(face.uAxis() == uAxis);
      CHECK(face.vAxis() == vAxis);
    }
  }

  SECTION("copyUvCoordSystemFromFace")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1995
    SECTION("copies the UV axes from an angled face")
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

      auto status = TestParserStatus{};

      auto nodes =
        NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status, taskManager);
      REQUIRE(nodes);

      auto* pyramidLight = dynamic_cast<BrushNode*>(nodes.value().at(0)->children()[0]);
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
        else if (
          vm::get_abs_max_component_axis(face.boundary().normal) == vm::vec3d{1, 0, 0})
        {
          REQUIRE(posXFace == nullptr);
          posXFace = &face;
        }
      }
      REQUIRE(negYFace != nullptr);
      REQUIRE(posXFace != nullptr);

      CHECK(negYFace->uAxis() == vm::vec3d{1, 0, 0});
      CHECK(negYFace->vAxis() == vm::vec3d{0, 0, -1});

      auto snapshot = negYFace->takeUvCoordSystemSnapshot();

      // copy texturing from the negYFace to posXFace using the rotation method
      posXFace->copyUvCoordSystemFromFace(
        *snapshot, negYFace->uvAttributes(), negYFace->boundary(), WrapStyle::Rotation);
      CHECK(
        posXFace->uAxis()
        == vm::approx{
          vm::vec3d{0.030303030303030123, 0.96969696969696961, -0.24242424242424243}});
      CHECK(
        posXFace->vAxis()
        == vm::approx{
          vm::vec3d{-0.0037296037296037088, -0.24242424242424243, -0.97016317016317011}});

      // copy texturing from the negYFace to posXFace using the projection method
      posXFace->copyUvCoordSystemFromFace(
        *snapshot, negYFace->uvAttributes(), negYFace->boundary(), WrapStyle::Projection);
      CHECK(posXFace->uAxis() == vm::approx{vm::vec3d{0, -1, 0}});
      CHECK(posXFace->vAxis() == vm::approx{vm::vec3d{0, 0, -1}});
    }
  }

  SECTION("convertToParaxial")
  {
    SECTION("preserves UVs of a face converted to parallel")
    {
      doWithTransformedStandardCubeFaces([](const BrushFace& standardFace) {
        auto valveFace = standardFace;
        REQUIRE(valveFace.convertToParallel());

        auto standardFaceRoundTrip = valveFace;
        REQUIRE(standardFaceRoundTrip.convertToParaxial());
        CHECK(standardFaceRoundTrip.uvCoordSystem().is<ParaxialUvCoordSystem>());
        checkFaceUvsEqual(standardFace, standardFaceRoundTrip);
      });
    }
  }

  SECTION("convertToParallel")
  {
    SECTION("preserves UVs")
    {
      doWithTransformedStandardCubeFaces([](const BrushFace& standardFace) {
        REQUIRE(standardFace.uvCoordSystem().is<ParaxialUvCoordSystem>());

        auto valveFace = standardFace;
        REQUIRE(valveFace.convertToParallel());
        CHECK(valveFace.uvCoordSystem().is<ParallelUvCoordSystem>());
        checkFaceUvsEqual(standardFace, valveFace);
      });
    }
  }

  SECTION("flipUv")
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

    auto status = TestParserStatus{};

    auto nodes =
      NodeReader::read(data, MapFormat::Valve, worldBounds, {}, status, taskManager);
    REQUIRE(nodes);

    auto* brushNode = dynamic_cast<BrushNode*>(nodes.value().at(0)->children()[0]);
    REQUIRE(brushNode != nullptr);

    auto brush = brushNode->brush();
    auto& face = brush.face(*brush.findFace(vm::vec3d{0, 0, 1}));
    CHECK(face.uvAttributes().scale == vm::vec2f{1, 1});

    SECTION("Default camera angle")
    {
      const auto cameraUp = vm::vec3d{0.284427, 0.455084, 0.843801};
      const auto cameraRight = vm::vec3d{0.847998, -0.529999, 0};

      SECTION("Left flip")
      {
        face.flipUv(cameraUp, cameraRight, vm::direction::left);
        CHECK(face.uvAttributes().scale == vm::vec2f{-1, 1});
      }

      SECTION("Right flip")
      {
        face.flipUv(cameraUp, cameraRight, vm::direction::right);
        CHECK(face.uvAttributes().scale == vm::vec2f{-1, 1});
      }

      SECTION("Up flip")
      {
        face.flipUv(cameraUp, cameraRight, vm::direction::up);
        CHECK(face.uvAttributes().scale == vm::vec2f{1, -1});
      }
    }

    SECTION("Camera is aimed at +x")
    {
      const auto cameraUp = vm::vec3d{0.419431, -0.087374, 0.903585};
      const auto cameraRight = vm::vec3d{-0.203938, -0.978984, 0};

      SECTION("left arrow (does vertical flip)")
      {
        face.flipUv(cameraUp, cameraRight, vm::direction::left);
        CHECK(face.uvAttributes().scale == vm::vec2f{1, -1});
      }

      SECTION("up arrow (does horizontal flip)")
      {
        face.flipUv(cameraUp, cameraRight, vm::direction::up);
        CHECK(face.uvAttributes().scale == vm::vec2f{-1, 1});
      }
    }
  }

  SECTION("invert")
  {
    auto face = createHorizontalFace(MapFormat::Standard);
    const auto points = face.points();
    const auto boundary = face.boundary();

    face.invert();
    CHECK(face.boundary() == boundary.flip());
    CHECK(face.points() == BrushFace::Points{points[0], points[2], points[1]});
  }

  SECTION("updatePointsFromVertices")
  {
    auto cube =
      BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}}.createCube(128.0, "")
      | kdl::value();
    const auto faceIndex = cube.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);
    auto& face = cube.face(*faceIndex);

    const auto boundary = face.boundary();
    const auto vertexPositions = face.vertexPositions();
    REQUIRE(!kdl::ranges::contains(vertexPositions, face.points()[1]));

    REQUIRE(face.updatePointsFromVertices().is_success());
    CHECK(face.boundary() == boundary);
    for (const auto& point : face.points())
    {
      CHECK(kdl::ranges::contains(vertexPositions, point));
    }
  }

  SECTION("projectToBoundaryMatrix")
  {
    SECTION("projects along the paraxial UV normal")
    {
      const auto face = createSlantedFace(MapFormat::Standard);
      CHECK(
        face.projectToBoundaryMatrix() * vm::vec3d{10, 20, 100}
        == vm::approx{vm::vec3d{10, 20, 69}});
    }

    SECTION("projects along the parallel UV normal")
    {
      // the parallel UV normal is the inverted face normal
      const auto face = createSlantedFace(MapFormat::Valve);
      CHECK(
        face.projectToBoundaryMatrix() * vm::vec3d{10, 20, 100}
        == vm::approx{vm::vec3d{22.4, 20, 75.2}});
    }
  }

  SECTION("toUvCoordSystemMatrix")
  {
    const auto uvAttributes =
      UvAttributes{.offset = {7, 3}, .scale = {0.5f, 2.0f}, .rotation = 30.0f};
    const auto face = createSlantedFace(MapFormat::Standard, uvAttributes);
    const auto point = vm::vec3d{10, 20, 69};

    const auto uvPoint =
      face.toUvCoordSystemMatrix(uvAttributes.offset, uvAttributes.scale) * point;
    CHECK(vm::vec2f{uvPoint.xy()} == vm::approx{face.uvCoords(point)});
    CHECK(uvPoint.z() == 0.0);
  }

  SECTION("fromUvCoordSystemMatrix")
  {
    const auto uvAttributes =
      UvAttributes{.offset = {7, 3}, .scale = {0.5f, 2.0f}, .rotation = 30.0f};
    const auto face = createSlantedFace(MapFormat::Standard, uvAttributes);
    const auto toUv = face.toUvCoordSystemMatrix(uvAttributes.offset, uvAttributes.scale);
    const auto fromUv =
      face.fromUvCoordSystemMatrix(uvAttributes.offset, uvAttributes.scale);

    // maps UV coordinates back onto the face plane
    const auto point = vm::vec3d{10, 20, 69};
    CHECK(fromUv * (toUv * point) == vm::approx{point});
  }

  SECTION("vertexCount")
  {
    const auto cube =
      BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}}.createCube(128.0, "")
      | kdl::value();
    const auto faceIndex = cube.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);
    const auto& face = cube.face(*faceIndex);

    CHECK(face.vertexCount() == 4u);
  }

  SECTION("edges")
  {
    const auto cube =
      BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}}.createCube(128.0, "")
      | kdl::value();
    const auto faceIndex = cube.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);
    const auto& face = cube.face(*faceIndex);

    const auto edges = face.edges() | kdl::ranges::to<std::vector>();
    const auto hasEdge = [&](const vm::vec3d& position1, const vm::vec3d& position2) {
      return std::ranges::any_of(edges, [&](const auto* edge) {
        return edge->hasPositions(position1, position2);
      });
    };

    CHECK(edges.size() == 4u);
    CHECK(hasEdge({-64, -64, 64}, {64, -64, 64}));
    CHECK(hasEdge({64, -64, 64}, {64, 64, 64}));
    CHECK(hasEdge({64, 64, 64}, {-64, 64, 64}));
    CHECK(hasEdge({-64, 64, 64}, {-64, -64, 64}));
  }

  SECTION("hasVertices")
  {
    const auto cube =
      BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}}.createCube(128.0, "")
      | kdl::value();
    const auto faceIndex = cube.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);
    const auto& face = cube.face(*faceIndex);

    const auto polygon = face.polygon();
    CHECK(face.hasVertices(polygon));

    // the vertices must be in the same order
    auto reversedVertices = polygon.vertices();
    std::ranges::reverse(reversedVertices);
    CHECK(!face.hasVertices(vm::polygon3d{reversedVertices}));

    // the vertices must be equal up to the given epsilon
    CHECK(!face.hasVertices(polygon.translate(vm::vec3d{0, 0, 0.01})));
    CHECK(face.hasVertices(polygon.translate(vm::vec3d{0, 0, 0.01}), 0.1));
  }

  SECTION("polygon")
  {
    const auto cube =
      BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}}.createCube(128.0, "")
      | kdl::value();
    const auto faceIndex = cube.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);
    const auto& face = cube.face(*faceIndex);

    CHECK_THAT(
      face.polygon().vertices(),
      Catch::Matchers::UnorderedEquals(std::vector<vm::vec3d>{
        {-64, -64, 64},
        {64, -64, 64},
        {64, 64, 64},
        {-64, 64, 64},
      }));
  }

  SECTION("intersectWithRay")
  {
    const auto cube =
      BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}}.createCube(128.0, "")
      | kdl::value();
    const auto faceIndex = cube.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);
    const auto& face = cube.face(*faceIndex);

    // a ray hitting the front of the face
    CHECK(
      face.intersectWithRay(vm::ray3d{vm::vec3d{0, 0, 128}, vm::vec3d{0, 0, -1}})
      == 64.0);

    // a ray hitting the back of the face
    CHECK(
      face.intersectWithRay(vm::ray3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}})
      == std::nullopt);

    // a ray missing the face
    CHECK(
      face.intersectWithRay(vm::ray3d{vm::vec3d{100, 0, 128}, vm::vec3d{0, 0, -1}})
      == std::nullopt);
  }
}

} // namespace tb::mdl
