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

#include "mdl/ModelSpecification.h"

#include "kd/hash_utils.h"
#include "kd/path_hash.h"
#include "kd/reflection_impl.h"

namespace tb::mdl
{

kdl_reflect_impl(ModelSpecification);

} // namespace tb::mdl

std::size_t std::hash<tb::mdl::ModelSpecification>::operator()(
  const tb::mdl::ModelSpecification& spec) const noexcept
{
  return kdl::combine_hash(
    kdl::path_hash{}(spec.path), kdl::hash(spec.skinIndex, spec.frameIndex));
}
