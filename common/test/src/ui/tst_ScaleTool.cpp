/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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

#include "ui/ScaleTool.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE("ScaleToolTest.moveBBoxFace_NonProportional")
{
  const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

  const auto exp1 = vm::bbox3d{{-100, -100, -100}, {125, 100, 100}};

  CHECK(
    moveBBoxSide(
      input1,
      BBoxSide{{1, 0, 0}},
      vm::vec3d{25, 0, 0},
      ProportionalAxes::None(),
      AnchorPos::Opposite)
    == exp1);

  // attempting to collapse the bbox returns an empty box
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-200, 0, 0},
          ProportionalAxes::None(),
          AnchorPos::Opposite)
          .is_empty());
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-225, 0, 0},
          ProportionalAxes::None(),
          AnchorPos::Opposite)
          .is_empty());

  // test with center anchor
  const auto exp2 = vm::bbox3d{{-125, -100, -100}, {125, 100, 100}};

  CHECK(
    moveBBoxSide(
      input1,
      BBoxSide{{1, 0, 0}},
      vm::vec3d{25, 0, 0},
      ProportionalAxes::None(),
      AnchorPos::Center)
    == exp2);
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-100, 0, 0},
          ProportionalAxes::None(),
          AnchorPos::Center)
          .is_empty());
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-125, 0, 0},
          ProportionalAxes::None(),
          AnchorPos::Center)
          .is_empty());
}

TEST_CASE("ScaleToolTest.moveBBoxFace_Proportional")
{
  const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

  const auto exp1 = vm::bbox3d{{-100, -112.5, -112.5}, {125, 112.5, 112.5}};

  CHECK(exp1.size() == vm::vec3d{225, 225, 225});
  CHECK(
    moveBBoxSide(
      input1,
      BBoxSide{{1, 0, 0}},
      vm::vec3d{25, 0, 0},
      ProportionalAxes::All(),
      AnchorPos::Opposite)
    == exp1);

  // attempting to collapse the bbox returns an empty box
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-200, 0, 0},
          ProportionalAxes::All(),
          AnchorPos::Opposite)
          .is_empty());
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-225, 0, 0},
          ProportionalAxes::All(),
          AnchorPos::Opposite)
          .is_empty());

  // test with center anchor
  const auto exp2 = vm::bbox3d{{-125, -125, -125}, {125, 125, 125}};

  CHECK(
    moveBBoxSide(
      input1,
      BBoxSide{{1, 0, 0}},
      vm::vec3d{25, 0, 0},
      ProportionalAxes::All(),
      AnchorPos::Center)
    == exp2);
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-100, 0, 0},
          ProportionalAxes::All(),
          AnchorPos::Center)
          .is_empty());
  CHECK(moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{-125, 0, 0},
          ProportionalAxes::All(),
          AnchorPos::Center)
          .is_empty());
}

TEST_CASE("ScaleToolTest.moveBBoxCorner")
{
  const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

  const auto exp1 = vm::bbox3d{{-100, -100, -100}, {125, 125, 125}};

  CHECK(
    moveBBoxCorner(
      input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{25, 25, 25}, AnchorPos::Opposite)
    == exp1);

  // attempting to collapse the bbox returns an empty box
  CHECK(moveBBoxCorner(
          input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-200, 0, 0}, AnchorPos::Opposite)
          .is_empty());
  CHECK(moveBBoxCorner(
          input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-225, 0, 0}, AnchorPos::Opposite)
          .is_empty());

  // test with center anchor
  const auto exp2 = vm::bbox3d{{-125, -125, -125}, {125, 125, 125}};

  CHECK(
    moveBBoxCorner(
      input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{25, 25, 25}, AnchorPos::Center)
    == exp2);
  CHECK(moveBBoxCorner(
          input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-100, 0, 0}, AnchorPos::Center)
          .is_empty());
  CHECK(moveBBoxCorner(
          input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-125, 0, 0}, AnchorPos::Center)
          .is_empty());
}

TEST_CASE("ScaleToolTest.moveBBoxEdge_NonProportional")
{
  const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

  const auto exp1 = vm::bbox3d{{-100, -100, -100}, {125, 125, 100}};

  // move the (+X, +Y, +/-Z) edge by X=25, Y=25
  CHECK(
    moveBBoxEdge(
      input1,
      BBoxEdge{{1, 1, -1}, {1, 1, 1}},
      vm::vec3d{25, 25, 0},
      ProportionalAxes::None(),
      AnchorPos::Opposite)
    == exp1);

  // attempting to collapse the bbox returns an empty box
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-200, -200, 0},
          ProportionalAxes::None(),
          AnchorPos::Opposite)
          .is_empty());
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-225, -225, 0},
          ProportionalAxes::None(),
          AnchorPos::Opposite)
          .is_empty());

  // test with center anchor
  const auto exp2 = vm::bbox3d{{-125, -125, -100}, {125, 125, 100}};

  // move the (+X, +Y, +/-Z) edge by X=25, Y=25
  CHECK(
    moveBBoxEdge(
      input1,
      BBoxEdge{{1, 1, -1}, {1, 1, 1}},
      vm::vec3d{25, 25, 0},
      ProportionalAxes::None(),
      AnchorPos::Center)
    == exp2);
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-100, -200, 0},
          ProportionalAxes::None(),
          AnchorPos::Center)
          .is_empty());
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-125, -225, 0},
          ProportionalAxes::None(),
          AnchorPos::Center)
          .is_empty());
}

TEST_CASE("ScaleToolTest.moveBBoxEdge_NonProportional_NegY")
{
  const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

  const auto exp1 = vm::bbox3d{{-100, -125, -100}, {100, 100, 125}};

  // move the (+Z, -Y, +/-X) edge by Z=25, Y=-25
  CHECK(
    moveBBoxEdge(
      input1,
      BBoxEdge{{1, -1, 1}, {-1, -1, 1}},
      vm::vec3d{0, -25, 25},
      ProportionalAxes::None(),
      AnchorPos::Opposite)
    == exp1);

  // test with center anchor
  const auto exp2 = vm::bbox3d{{-100, -125, -125}, {100, 125, 125}};

  CHECK(
    moveBBoxEdge(
      input1,
      BBoxEdge{{1, -1, 1}, {-1, -1, 1}},
      vm::vec3d{0, -25, 25},
      ProportionalAxes::None(),
      AnchorPos::Center)
    == exp2);
}

TEST_CASE("ScaleToolTest.moveBBoxEdge_Proportional")
{
  const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

  const auto exp1 = vm::bbox3d{{-100, -100, -112.5}, {125, 125, 112.5}};

  // move the (+X, +Y, +/-Z) edge by X=25, Y=25
  CHECK(
    moveBBoxEdge(
      input1,
      BBoxEdge{{1, 1, -1}, {1, 1, 1}},
      vm::vec3d{25, 25, 0},
      ProportionalAxes::All(),
      AnchorPos::Opposite)
    == exp1);

  // attempting to collapse the bbox returns an empty box
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-200, -200, 0},
          ProportionalAxes::All(),
          AnchorPos::Opposite)
          .is_empty());
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-225, -225, 0},
          ProportionalAxes::All(),
          AnchorPos::Opposite)
          .is_empty());

  // test with center anchor
  const auto exp2 = vm::bbox3d{{-125, -125, -125}, {125, 125, 125}};

  CHECK(
    moveBBoxEdge(
      input1,
      BBoxEdge{{1, 1, -1}, {1, 1, 1}},
      vm::vec3d{25, 25, 0},
      ProportionalAxes::All(),
      AnchorPos::Center)
    == exp2);
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-100, -100, 0},
          ProportionalAxes::All(),
          AnchorPos::Center)
          .is_empty());
  CHECK(moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{-125, -125, 0},
          ProportionalAxes::All(),
          AnchorPos::Center)
          .is_empty());
}

TEST_CASE("ScaleToolTest.moveBBoxEdge")
{
  const auto input1 = vm::bbox3d{{-64, -64, -16}, {64, 64, 16}};

  const auto exp1 = vm::bbox3d{{-64, -64, -16}, {128, 64, 48}};

  // NOTE: the Y=64 part of the delta is ignored because the edge we are moving points
  // along the Y axis
  const auto delta = vm::vec3d{64, 64, 32};

  CHECK(
    moveBBoxEdge(
      input1,
      BBoxEdge{{1, 1, 1}, {1, -1, 1}},
      delta,
      ProportionalAxes(true, false, true),
      AnchorPos::Opposite)
    == exp1);
}
} // namespace tb::ui
