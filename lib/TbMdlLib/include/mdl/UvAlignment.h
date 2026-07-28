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

#pragma once

#include "mdl/UpdateBrushFaceAttributes.h"

#include "vm/vec.h"

#include <iosfwd>

namespace tb::mdl
{

class BrushFace;

enum class UvPolicy
{
  best,
  next,
  prev,
};

std::ostream& operator<<(std::ostream& lhs, UvPolicy rhs);

enum class UvFitMode
{
  fitToFace,
  trimSheet,
};

std::ostream& operator<<(std::ostream& lhs, UvFitMode rhs);

enum class UvAxis
{
  u,
  v,
};

std::ostream& operator<<(std::ostream& lhs, UvAxis rhs);

enum class UvSign
{
  plus,
  minus,
};

std::ostream& operator<<(std::ostream& lhs, UvSign rhs);

/**
 * Return a vertex of the given brush face to use as an anchor.
 *
 * 1. If the texture is currently justifed along the given axis and sign, then a vertex is
 *    selected such that its dot product with the UV axis vector with sign is maximal.
 *
 * 2. Otherwise, we try the opposite sign and return the corresponding vertex if the
 *    texture is justified.
 *
 * 3. If the texture isn't justified according to the given axis and either sign, we
 *    return the same vertex as in case 1.
 */
vm::vec3d anchorVertex(const BrushFace& brushFace, UvAxis uvAxis, UvSign preferredSign);

bool isAligned(const BrushFace& brushFace);
bool isJustified(const BrushFace& brushFace, UvAxis uvAxis, UvSign uvSign);
bool isFitted(const BrushFace& brushFace, UvAxis uvAxis);

UpdateBrushFaceAttributes align(const BrushFace& brushFace, UvPolicy uvPolicy);
UpdateBrushFaceAttributes justify(
  const BrushFace& brushFace, UvAxis uvAxis, UvSign uvSign, UvPolicy uvPolicy);
UpdateBrushFaceAttributes fit(
  const BrushFace& brushFace, UvAxis uvAxis, UvPolicy uvPolicy, UvFitMode uvFitMode);

} // namespace tb::mdl
