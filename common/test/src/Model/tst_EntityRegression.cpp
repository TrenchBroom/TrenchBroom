/*
 Copyright (C) 2021 Kristian Duske

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

#include "Assets/EntityDefinition.h"
#include "Color.h"
#include "EL/ELExceptions.h"
#include "EL/Expression.h"
#include "EL/Expressions.h"
#include "FloatType.h"
#include "IO/ELParser.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"

#include "vm/bbox.h"
#include "vm/bbox_io.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
TEST_CASE("EntityTest.modelScaleExpressionThrows")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/3914

  const auto modelExpression = IO::ELParser{IO::ELParser::Mode::Strict, R"(
{{
    spawnflags & 2 ->   ":maps/b_bh100.bsp",
    spawnflags & 1 ->   ":maps/b_bh10.bsp",
                        ":maps/b_bh25.bsp"
}})"}
                                 .parse();

  auto definition = Assets::PointEntityDefinition{
    "some_name",
    Color{},
    vm::bbox3{32.0},
    "",
    {},
    Assets::ModelDefinition{modelExpression},
    {}};
  const auto propertyConfig = EntityPropertyConfig{};

  auto entity = Entity{};
  entity.setDefinition(propertyConfig, &definition);

  // throws because 'a & 2' cannot be evaluated -- we must catch the exception in
  // Entity::updateCachedProperties
  CHECK_NOTHROW(entity.addOrUpdateProperty(propertyConfig, "spawnflags", "a"));
}
} // namespace Model
} // namespace TrenchBroom
