/*
 Copyright (C) 2025 Kristian Duske

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

#include "Result.h"

#include <string>

namespace tb::gl
{
class Texture;
}

namespace tb::fs
{
class Reader;
}

namespace tb::mdl
{

/**
 * Loads a UE1 (Unreal/UT99) Texture object named `textureName` out of a
 * package. Direct port of `uetex.py` (the validated reference implementation)
 * for the P8-indexed decode path: tagged property list, TArray<FMipmap> mip
 * chain, and TArray<FColor> palette.
 *
 * `reader` must cover the entire package file (.utx), not just one object's
 * serial data: the Texture's `Palette` property is an object reference that
 * is resolved by name against the same package's export table, matching the
 * self-contained .utx files Phase 1 targets (Crypt.utx, NBSpecialT.utx).
 *
 * Phase 1 scope: FileVersion 61 (Unreal) and 68 (UT99).
 */
Result<gl::Texture> loadUnrealTexture(
  fs::Reader& reader, const std::string& packageName, const std::string& textureName);

} // namespace tb::mdl
