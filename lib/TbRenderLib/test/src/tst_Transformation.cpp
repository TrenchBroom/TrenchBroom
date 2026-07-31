/*
 Copyright (C) 2026 Kristian Duske

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

#include "gl/TestGl.h"
#include "render/Transformation.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

TEST_CASE("Transformation")
{
  auto testGl = gl::TestGl{};

  const auto projection = vm::translation_matrix(vm::vec3f{1, 0, 0});
  const auto view = vm::translation_matrix(vm::vec3f{0, 1, 0});
  const auto model = vm::translation_matrix(vm::vec3f{0, 0, 1});

  SECTION("constructor")
  {
    SECTION("stores the given matrices")
    {
      auto transformation = Transformation{testGl, projection, view, model};
      CHECK(transformation.projectionMatrix() == projection);
      CHECK(transformation.viewMatrix() == view);
      CHECK(transformation.modelMatrix() == model);
    }

    SECTION("defaults the model matrix to identity")
    {
      auto transformation = Transformation{testGl, projection, view};
      CHECK(transformation.modelMatrix() == vm::mat4x4f::identity());
    }
  }

  SECTION("pushTransformation and popTransformation")
  {
    auto transformation = Transformation{testGl, projection, view, model};

    const auto newProjection = vm::translation_matrix(vm::vec3f{2, 0, 0});
    const auto newView = vm::translation_matrix(vm::vec3f{0, 2, 0});
    const auto newModel = vm::translation_matrix(vm::vec3f{0, 0, 2});

    transformation.pushTransformation(newProjection, newView, newModel);
    CHECK(transformation.projectionMatrix() == newProjection);
    CHECK(transformation.viewMatrix() == newView);
    CHECK(transformation.modelMatrix() == newModel);

    transformation.popTransformation();
    CHECK(transformation.projectionMatrix() == projection);
    CHECK(transformation.viewMatrix() == view);
    CHECK(transformation.modelMatrix() == model);
  }

  SECTION("pushModelMatrix and popModelMatrix")
  {
    auto transformation = Transformation{testGl, projection, view, model};

    const auto delta = vm::translation_matrix(vm::vec3f{1, 1, 1});
    transformation.pushModelMatrix(delta);
    CHECK(transformation.modelMatrix() == model * delta);

    transformation.popModelMatrix();
    CHECK(transformation.modelMatrix() == model);
  }

  SECTION("replaceAndPushModelMatrix and popModelMatrix")
  {
    auto transformation = Transformation{testGl, projection, view, model};

    const auto replacement = vm::translation_matrix(vm::vec3f{5, 5, 5});
    transformation.replaceAndPushModelMatrix(replacement);
    CHECK(transformation.modelMatrix() == replacement);

    transformation.popModelMatrix();
    CHECK(transformation.modelMatrix() == model);
  }

  SECTION("slice")
  {
    auto transformation = Transformation{testGl, projection, view, model};
    transformation.pushModelMatrix(vm::translation_matrix(vm::vec3f{1, 1, 1}));

    auto sliced = transformation.slice();
    CHECK(sliced.projectionMatrix() == transformation.projectionMatrix());
    CHECK(sliced.viewMatrix() == transformation.viewMatrix());
    CHECK(sliced.modelMatrix() == transformation.modelMatrix());
  }

  SECTION("destructor restores the base matrices when the stacks were not unwound")
  {
    // exercises the destructor's restore path (stack size > 1 on destruction), as
    // opposed to the common case where every push has a matching pop
    auto transformation = Transformation{testGl, projection, view, model};
    transformation.pushTransformation(
      vm::translation_matrix(vm::vec3f{2, 0, 0}),
      vm::translation_matrix(vm::vec3f{0, 2, 0}));
    transformation.pushModelMatrix(vm::translation_matrix(vm::vec3f{0, 0, 2}));
    // transformation goes out of scope here without popping back down to the base
  }
}

TEST_CASE("ReplaceTransformation")
{
  SECTION("constructor and destructor push and pop a transformation")
  {
    auto testGl = gl::TestGl{};
    const auto projection = vm::translation_matrix(vm::vec3f{1, 0, 0});
    const auto view = vm::translation_matrix(vm::vec3f{0, 1, 0});
    const auto model = vm::translation_matrix(vm::vec3f{0, 0, 1});

    auto transformation = Transformation{testGl, projection, view, model};

    const auto newProjection = vm::translation_matrix(vm::vec3f{9, 0, 0});
    const auto newView = vm::translation_matrix(vm::vec3f{0, 9, 0});
    const auto newModel = vm::translation_matrix(vm::vec3f{0, 0, 9});

    {
      const auto replace =
        ReplaceTransformation{transformation, newProjection, newView, newModel};
      CHECK(transformation.projectionMatrix() == newProjection);
      CHECK(transformation.viewMatrix() == newView);
      CHECK(transformation.modelMatrix() == newModel);
    }

    CHECK(transformation.projectionMatrix() == projection);
    CHECK(transformation.viewMatrix() == view);
    CHECK(transformation.modelMatrix() == model);
  }
}

TEST_CASE("MultiplyModelMatrix")
{
  SECTION("constructor and destructor push and pop a model matrix multiplication")
  {
    auto testGl = gl::TestGl{};
    const auto projection = vm::mat4x4f::identity();
    const auto view = vm::mat4x4f::identity();
    const auto model = vm::translation_matrix(vm::vec3f{0, 0, 1});

    auto transformation = Transformation{testGl, projection, view, model};
    const auto delta = vm::translation_matrix(vm::vec3f{1, 1, 1});

    {
      const auto multiply = MultiplyModelMatrix{transformation, delta};
      CHECK(transformation.modelMatrix() == model * delta);
    }

    CHECK(transformation.modelMatrix() == model);
  }
}

TEST_CASE("ReplaceModelMatrix")
{
  SECTION("constructor and destructor push and pop a model matrix replacement")
  {
    auto testGl = gl::TestGl{};
    const auto projection = vm::mat4x4f::identity();
    const auto view = vm::mat4x4f::identity();
    const auto model = vm::translation_matrix(vm::vec3f{0, 0, 1});

    auto transformation = Transformation{testGl, projection, view, model};
    const auto replacement = vm::translation_matrix(vm::vec3f{5, 5, 5});

    {
      const auto replace = ReplaceModelMatrix{transformation, replacement};
      CHECK(transformation.modelMatrix() == replacement);
    }

    CHECK(transformation.modelMatrix() == model);
  }
}

} // namespace tb::render
