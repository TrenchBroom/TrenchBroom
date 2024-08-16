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

#pragma once

#include "kdl/reflection_decl.h"

#include <filesystem>

namespace TrenchBroom::Assets
{

struct ModelSpecification
{
  std::filesystem::path path = {};
  size_t skinIndex = 0;
  size_t frameIndex = 0;

  kdl_reflect_decl(ModelSpecification, path, skinIndex, frameIndex);
};

} // namespace TrenchBroom::Assets


template <>
struct std::hash<TrenchBroom::Assets::ModelSpecification>
{
  std::size_t operator()(
    const TrenchBroom::Assets::ModelSpecification& spec) const noexcept;
};
