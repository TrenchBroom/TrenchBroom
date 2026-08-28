/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceComparisonAlignment.h"
#include "ui/CatchConfig.h"

#include <variant>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceAlignment matrixAlignment()
{
  auto alignment = AcceptanceAlignment{};
  alignment.type = AcceptanceAlignmentType::Matrix;
  alignment.matrix = {
    2.0,
    0.0,
    0.0,
    10.0,
    0.0,
    3.0,
    0.0,
    20.0,
    0.0,
    0.0,
    4.0,
    30.0,
    0.0,
    0.0,
    0.0,
    1.0,
  };
  return alignment;
}

AcceptanceAlignmentErrorCode errorCode(const AcceptanceAlignedPointResult& result)
{
  return std::get<AcceptanceAlignmentError>(result.error()).code;
}

} // namespace

TEST_CASE("AcceptanceComparisonAlignment")
{
  SECTION("maps identity reference points unchanged")
  {
    const auto result =
      alignAcceptanceTargetPoint(AcceptanceAlignment{}, vm::vec3d{1.0, 2.0, 3.0});

    REQUIRE(result.is_success());
    const auto expected = vm::vec3d{1.0, 2.0, 3.0};
    CHECK(result.value() == expected);
  }

  SECTION("maps reference points with a row-major affine matrix")
  {
    const auto result =
      alignAcceptanceTargetPoint(matrixAlignment(), vm::vec3d{1.0, 2.0, 3.0});

    REQUIRE(result.is_success());
    const auto expected = vm::vec3d{12.0, 26.0, 42.0};
    CHECK(result.value() == expected);
  }

  SECTION("rejects point transforms without a defined mapping")
  {
    auto independent = AcceptanceAlignment{};
    independent.type = AcceptanceAlignmentType::Independent;
    CHECK(
      errorCode(alignAcceptanceTargetPoint(independent, {}))
      == AcceptanceAlignmentErrorCode::UnsupportedAlignment);

    auto landmarks = AcceptanceAlignment{};
    landmarks.type = AcceptanceAlignmentType::Landmarks;
    CHECK(
      errorCode(alignAcceptanceTargetPoint(landmarks, {}))
      == AcceptanceAlignmentErrorCode::UnsupportedAlignment);
  }

  SECTION("uses the camera transform's matrix validation")
  {
    auto invalid = matrixAlignment();
    invalid.matrix[15u] = 2.0;
    CHECK(
      errorCode(alignAcceptanceTargetPoint(invalid, {}))
      == AcceptanceAlignmentErrorCode::InvalidMatrix);

    auto singular = matrixAlignment();
    singular.matrix[0u] = 0.0;
    CHECK(
      errorCode(alignAcceptanceTargetPoint(singular, {}))
      == AcceptanceAlignmentErrorCode::NonInvertibleMatrix);
  }
}

} // namespace tb::ui
