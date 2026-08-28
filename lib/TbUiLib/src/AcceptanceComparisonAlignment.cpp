/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceComparisonAlignment.h"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <ranges>
#include <variant>

namespace tb::ui
{
namespace
{

AcceptanceAlignmentError error(
  const AcceptanceAlignmentErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

bool finite(const double value)
{
  return std::isfinite(value);
}

bool validAffineMatrix(const std::array<double, 16u>& matrix)
{
  constexpr auto epsilon = 0.000001;
  return std::ranges::all_of(matrix, [](const auto value) { return finite(value); })
         && std::abs(matrix[12u]) < epsilon && std::abs(matrix[13u]) < epsilon
         && std::abs(matrix[14u]) < epsilon && std::abs(matrix[15u] - 1.0) < epsilon;
}

double determinant(const std::array<double, 16u>& matrix)
{
  return matrix[0u] * (matrix[5u] * matrix[10u] - matrix[6u] * matrix[9u])
         - matrix[1u] * (matrix[4u] * matrix[10u] - matrix[6u] * matrix[8u])
         + matrix[2u] * (matrix[4u] * matrix[9u] - matrix[5u] * matrix[8u]);
}

Result<void, AcceptanceAlignmentError> validateMatrix(
  const std::array<double, 16u>& matrix)
{
  if (!validAffineMatrix(matrix))
  {
    return error(
      AcceptanceAlignmentErrorCode::InvalidMatrix,
      "Alignment matrix must be finite row-major affine matrix");
  }
  if (std::abs(determinant(matrix)) < 0.000000001)
  {
    return error(
      AcceptanceAlignmentErrorCode::NonInvertibleMatrix,
      "Alignment matrix must have an invertible linear component");
  }
  return {};
}

vm::vec3d transformPoint(const std::array<double, 16u>& matrix, const vm::vec3d& point)
{
  return {
    matrix[0u] * point.x() + matrix[1u] * point.y() + matrix[2u] * point.z() + matrix[3u],
    matrix[4u] * point.x() + matrix[5u] * point.y() + matrix[6u] * point.z() + matrix[7u],
    matrix[8u] * point.x() + matrix[9u] * point.y() + matrix[10u] * point.z()
      + matrix[11u],
  };
}

vm::vec3d transformVector(const std::array<double, 16u>& matrix, const vm::vec3d& vector)
{
  return {
    matrix[0u] * vector.x() + matrix[1u] * vector.y() + matrix[2u] * vector.z(),
    matrix[4u] * vector.x() + matrix[5u] * vector.y() + matrix[6u] * vector.z(),
    matrix[8u] * vector.x() + matrix[9u] * vector.y() + matrix[10u] * vector.z(),
  };
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const AcceptanceAlignmentError& rhs)
{
  return lhs << rhs.message;
}

AcceptanceAlignedCameraResult alignAcceptanceTargetCamera(
  const AcceptanceAlignment& alignment,
  const AcceptanceCamera& referenceCamera,
  const AcceptanceCamera& authoredTargetCamera)
{
  switch (alignment.type)
  {
  case AcceptanceAlignmentType::Identity:
    return referenceCamera;
  case AcceptanceAlignmentType::Independent:
    return authoredTargetCamera;
  case AcceptanceAlignmentType::Landmarks:
    return error(
      AcceptanceAlignmentErrorCode::UnsupportedAlignment,
      "Landmark alignment is not implemented by the paired-capture foundation");
  case AcceptanceAlignmentType::Matrix:
    break;
  }

  const auto matrixValidation = validateMatrix(alignment.matrix);
  if (matrixValidation.is_error())
    return std::get<AcceptanceAlignmentError>(matrixValidation.error());

  auto targetCamera = referenceCamera;
  targetCamera.position = transformPoint(alignment.matrix, referenceCamera.position);
  const auto direction = transformVector(alignment.matrix, referenceCamera.direction);
  const auto up = transformVector(alignment.matrix, referenceCamera.up);
  if (vm::is_zero(direction, 0.000001) || vm::is_zero(up, 0.000001))
  {
    return error(
      AcceptanceAlignmentErrorCode::InvalidMatrix,
      "Alignment matrix collapses a camera orientation vector");
  }
  targetCamera.direction = vm::normalize(direction);
  targetCamera.up = vm::normalize(up);
  return targetCamera;
}

AcceptanceAlignedPointResult alignAcceptanceTargetPoint(
  const AcceptanceAlignment& alignment, const vm::vec3d& referencePoint)
{
  switch (alignment.type)
  {
  case AcceptanceAlignmentType::Identity:
    return referencePoint;
  case AcceptanceAlignmentType::Independent:
    return error(
      AcceptanceAlignmentErrorCode::UnsupportedAlignment,
      "Independent alignment does not define a reference-to-candidate point transform");
  case AcceptanceAlignmentType::Landmarks:
    return error(
      AcceptanceAlignmentErrorCode::UnsupportedAlignment,
      "Landmark alignment does not define a reference-to-candidate point transform");
  case AcceptanceAlignmentType::Matrix:
    break;
  }

  const auto matrixValidation = validateMatrix(alignment.matrix);
  if (matrixValidation.is_error())
    return std::get<AcceptanceAlignmentError>(matrixValidation.error());
  return transformPoint(alignment.matrix, referencePoint);
}

} // namespace tb::ui
