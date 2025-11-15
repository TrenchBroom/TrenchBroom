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

#pragma once

#include "Color.h"
#include "Result.h"
#include "mdl/EntityDefinition.h"

#include "kdl/reflection_decl.h"

#include <string>
#include <vector>

namespace tb::mdl
{

/**
 * Color default values can have extra values, e.g. brightness. These are not strictly
 * typed, so we just parse them as float.
 */
struct EntityColorPropertyValue
{
  Rgb color;
  std::vector<float> extraComponents;

  kdl_reflect_decl(EntityColorPropertyValue, color, extraComponents);
};

Result<EntityColorPropertyValue> parseEntityColorPropertyValue(
  const EntityDefinition* entityDefinition,
  const std::string& propertyKey,
  const std::string& propertyValue);

Result<std::string> entityColorPropertyToString(
  const EntityDefinition* entityDefinition,
  const std::string& propertyKey,
  const EntityColorPropertyValue& entityColorPropertyValue);

} // namespace tb::mdl
