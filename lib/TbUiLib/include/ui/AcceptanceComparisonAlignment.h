/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "ui/AcceptanceView.h"

#include <iosfwd>
#include <string>

namespace tb::ui
{

enum class AcceptanceAlignmentErrorCode
{
  InvalidMatrix,
  NonInvertibleMatrix,
  UnsupportedAlignment,
};

struct AcceptanceAlignmentError
{
  AcceptanceAlignmentErrorCode code = AcceptanceAlignmentErrorCode::InvalidMatrix;
  std::string message;
};

std::ostream& operator<<(std::ostream& lhs, const AcceptanceAlignmentError& rhs);

using AcceptanceAlignedCameraResult = Result<AcceptanceCamera, AcceptanceAlignmentError>;

/**
 * Produces the target camera for a comparison. Matrix values are row-major affine
 * transforms mapping reference world coordinates to target world coordinates.
 * Landmark solving is intentionally deferred to a later AV3 subpackage.
 */
AcceptanceAlignedCameraResult alignAcceptanceTargetCamera(
  const AcceptanceAlignment& alignment,
  const AcceptanceCamera& referenceCamera,
  const AcceptanceCamera& authoredTargetCamera);

} // namespace tb::ui
