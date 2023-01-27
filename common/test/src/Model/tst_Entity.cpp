/*
 Copyright (C) 2020 Kristian Duske

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
#include "Assets/PropertyDefinition.h"
#include "EL/Expressions.h"
#include "FloatType.h"
#include "IO/ELParser.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"

#include <vecmath/approx.h>
#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
TEST_CASE("EntityTest.defaults")
{
  auto entity = Entity{};

  CHECK(entity.classname() == EntityPropertyValues::NoClassname);
  CHECK(entity.pointEntity());
  CHECK(entity.origin() == vm::vec3::zero());
  CHECK(entity.rotation() == vm::mat4x4::identity());
}

TEST_CASE("EntityTest.setProperties")
{
  SECTION("Updates cached model transformation")
  {
    auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};
    auto definition = Assets::PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3{32.0},
      "",
      {},
      Assets::ModelDefinition{
        {EL::MapExpression{{{"scale", {EL::VariableExpression{"modelscale"}, 0, 0}}}},
         0,
         0}}};

    auto entity = Entity{};
    entity.setDefinition(config, &definition);
    REQUIRE(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));

    entity.setProperties(config, {{"modelscale", "1 2 3"}});

    CHECK(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{1, 2, 3}));
  }
}

TEST_CASE("EntityTest.setDefaultProperties")
{
  const auto propertyConfig = EntityPropertyConfig{};

  auto definition = Assets::PointEntityDefinition{
    "some_name",
    Color{},
    vm::bbox3{32.0},
    "",
    {
      std::make_shared<Assets::StringPropertyDefinition>(
        "some_prop", "", "", !true(readOnly)),
      std::make_shared<Assets::StringPropertyDefinition>(
        "some_default_prop", "", "", !true(readOnly), "value"),
    },
    {}};

  using T = std::tuple<
    std::vector<EntityProperty>,
    SetDefaultPropertyMode,
    std::vector<EntityProperty>>;

  const auto [initialProperties, mode, expectedProperties] = GENERATE(values<T>({
    {{}, SetDefaultPropertyMode::SetExisting, std::vector<EntityProperty>{}},
    {{}, SetDefaultPropertyMode::SetMissing, {{"some_default_prop", "value"}}},
    {{}, SetDefaultPropertyMode::SetAll, {{"some_default_prop", "value"}}},
    {{{"some_default_prop", "other_value"}},
     SetDefaultPropertyMode::SetExisting,
     {{"some_default_prop", "value"}}},
    {{{"some_default_prop", "other_value"}},
     SetDefaultPropertyMode::SetMissing,
     {{"some_default_prop", "other_value"}}},
    {{{"some_default_prop", "other_value"}},
     SetDefaultPropertyMode::SetAll,
     {{"some_default_prop", "value"}}},
  }));

  auto entity = Entity{propertyConfig, initialProperties};
  setDefaultProperties(propertyConfig, definition, entity, mode);

  CHECK_THAT(entity.properties(), Catch::Matchers::UnorderedEquals(expectedProperties));
}

TEST_CASE("EntityTest.definitionBounds")
{
  auto pointEntityDefinition =
    Assets::PointEntityDefinition("some_name", Color(), vm::bbox3(32.0), "", {}, {});
  Entity entity;

  SECTION("Returns default bounds if no definition is set")
  {
    CHECK(entity.definitionBounds() == vm::bbox3(8.0));
  }

  SECTION("Returns definition bounds if definition is set")
  {
    entity.setDefinition({}, &pointEntityDefinition);
    CHECK(entity.definitionBounds() == vm::bbox3(32.0));
  }
}

TEST_CASE("EntityTest.setDefinition")
{
  SECTION("Updates cached model transformation")
  {
    auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};
    auto definition = Assets::PointEntityDefinition{
      "some_name",
      Color(),
      vm::bbox3(32.0),
      "",
      {},
      Assets::ModelDefinition{{EL::MapExpression{{}}, 0, 0}}};

    auto entity = Entity{};
    REQUIRE(entity.modelTransformation() == vm::mat4x4::identity());

    entity.setDefinition(config, &definition);
    CHECK(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));
  }
}

TEST_CASE("EntityTest.modelSpecification")
{
  auto modelExpression = IO::ELParser::parseStrict(R"({{ 
      spawnflags == 0 -> "maps/b_shell0.bsp",
      spawnflags == 1 -> "maps/b_shell1.bsp",
                         "maps/b_shell2.bsp"
  }})");

  auto definition = Assets::PointEntityDefinition{
    "some_name",
    Color(),
    vm::bbox3(32.0),
    "",
    {},
    Assets::ModelDefinition{modelExpression}};

  auto entity = Entity{};
  entity.setDefinition({}, &definition);
  CHECK(
    entity.modelSpecification()
    == Assets::ModelSpecification{IO::Path{"maps/b_shell0.bsp"}, 0, 0});

  entity.addOrUpdateProperty({}, EntityPropertyKeys::Spawnflags, "1");
  CHECK(
    entity.modelSpecification()
    == Assets::ModelSpecification{IO::Path{"maps/b_shell1.bsp"}, 0, 0});
}

TEST_CASE("EntityTest.unsetEntityDefinitionAndModel")
{
  auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};
  auto definition = Assets::PointEntityDefinition{
    "some_name",
    Color(),
    vm::bbox3(32.0),
    "",
    {},
    Assets::ModelDefinition{{EL::MapExpression{{}}, 0, 0}}};

  auto entity = Entity{};
  entity.setDefinition(config, &definition);
  REQUIRE(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));

  entity.unsetEntityDefinitionAndModel();
  CHECK(entity.definition() == nullptr);
  CHECK(entity.modelTransformation() == vm::mat4x4::identity());
}

TEST_CASE("EntityTest.addOrUpdateProperty")
{
  // needs to be created here so that it is destroyed last
  auto definition =
    Assets::PointEntityDefinition{"some_name", Color{}, vm::bbox3{32.0}, "", {}, {}};

  Entity entity;
  REQUIRE(entity.property("test") == nullptr);

  entity.addOrUpdateProperty({}, "test", "value");
  CHECK(*entity.property("test") == "value");

  entity.addOrUpdateProperty({}, "test", "newValue");
  CHECK(*entity.property("test") == "newValue");

  SECTION("Setting a new property to protected by default")
  {
    entity.addOrUpdateProperty({}, "newKey", "newValue", true);
    CHECK_THAT(
      entity.protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));

    entity.addOrUpdateProperty({}, "test", "anotherValue", true);
    CHECK_THAT(
      entity.protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));
  }

  SECTION("Updates cached model transformation")
  {
    auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};

    entity.setDefinition({}, &definition);
    REQUIRE(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{1, 1, 1}));

    entity.addOrUpdateProperty(config, "something", "else");
    CHECK(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));
  }
}

TEST_CASE("EntityTest.renameProperty")
{
  // needs to be created here so that it is destroyed last
  auto definition = Assets::PointEntityDefinition{
    "some_name",
    Color{},
    vm::bbox3{32.0},
    "",
    {},
    Assets::ModelDefinition{
      {EL::MapExpression{{{"scale", {EL::VariableExpression{"modelscale"}, 0, 0}}}},
       0,
       0}}};

  Entity entity;

  SECTION("Rename non existing property")
  {
    REQUIRE(!entity.hasProperty("originalKey"));
    entity.renameProperty({}, "originalKey", "newKey");
    CHECK(!entity.hasProperty("originalKey"));
    CHECK(!entity.hasProperty("newKey"));
  }

  entity.addOrUpdateProperty({}, "originalKey", "originalValue");
  REQUIRE(*entity.property("originalKey") == "originalValue");

  SECTION("Rename existing property")
  {
    entity.renameProperty({}, "originalKey", "newKey");
    CHECK(!entity.hasProperty("originalKey"));
    CHECK(*entity.property("newKey") == "originalValue");
  }

  SECTION("Rename existing property - name conflict")
  {
    entity.addOrUpdateProperty({}, "newKey", "newValue");

    entity.renameProperty({}, "originalKey", "newKey");
    CHECK(!entity.hasProperty("originalKey"));
    CHECK(*entity.property("newKey") == "originalValue");
  }

  SECTION("Rename existing protected property")
  {
    entity.setProtectedProperties({"originalKey"});
    entity.renameProperty({}, "originalKey", "newKey");
    CHECK_THAT(
      entity.protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));
  }

  SECTION("Updates cached model transformation")
  {
    auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};

    entity.setDefinition(config, &definition);
    entity.addOrUpdateProperty(config, "something", "1 2 3");
    REQUIRE(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));

    entity.renameProperty(config, "something", "modelscale");
    CHECK(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{1, 2, 3}));

    entity.renameProperty(config, "modelscale", "not modelscale");
    CHECK(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));
  }
}

TEST_CASE("EntityTest.removeProperty")
{
  // needs to be created here so that it is destroyed last
  auto definition = Assets::PointEntityDefinition{
    "some_name",
    Color{},
    vm::bbox3{32.0},
    "",
    {},
    Assets::ModelDefinition{
      {EL::MapExpression{{{"scale", {EL::VariableExpression{"modelscale"}, 0, 0}}}},
       0,
       0}}};

  Entity entity;

  SECTION("Remove non existing property")
  {
    REQUIRE(!entity.hasProperty("key"));
    entity.removeProperty({}, "key");
    CHECK(!entity.hasProperty("key"));
  }

  SECTION("Remove existing property")
  {
    entity.addOrUpdateProperty({}, "key", "value");
    entity.removeProperty({}, "key");
    CHECK(!entity.hasProperty("key"));
  }

  SECTION("Remove protected property")
  {
    entity.addOrUpdateProperty({}, "newKey", "value", true);
    REQUIRE_THAT(
      entity.protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));

    entity.removeProperty({}, "newKey");
    REQUIRE(!entity.hasProperty("newKey"));
    CHECK_THAT(
      entity.protectedProperties(),
      Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));
  }

  SECTION("Updates cached model transformation")
  {
    auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};

    entity.setDefinition(config, &definition);
    entity.addOrUpdateProperty(config, "modelscale", "1 2 3");
    REQUIRE(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{1, 2, 3}));

    entity.removeProperty(config, "modelscale");
    CHECK(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));
  }
}

TEST_CASE("EntityTest.hasProperty")
{
  Entity entity;
  CHECK(!entity.hasProperty("value"));

  entity.setProperties({}, {{"key", "value"}});
  CHECK(entity.hasProperty("key"));
}

TEST_CASE("EntityTest.originUpdateWithSetProperties")
{
  Entity entity;
  entity.setProperties({}, {{"origin", "10 20 30"}});

  CHECK(entity.origin() == vm::vec3(10, 20, 30));
}

TEST_CASE("EntityTest.hasPropertyWithPrefix")
{
  Entity entity;
  entity.setProperties(
    {},
    {
      {"somename", "somevalue"},
      {"someothername", "someothervalue"},
    });

  CHECK(entity.hasPropertyWithPrefix("somename", "somevalue"));
  CHECK(entity.hasPropertyWithPrefix("some", "somevalue"));
  CHECK(entity.hasPropertyWithPrefix("some", "someothervalue"));
  CHECK(entity.hasPropertyWithPrefix("someother", "someothervalue"));
  CHECK(!entity.hasPropertyWithPrefix("someother", "somevalue"));
  CHECK(!entity.hasPropertyWithPrefix("sime", ""));
}

TEST_CASE("EntityTest.hasNumberedProperty")
{
  Entity entity;
  entity.setProperties(
    {},
    {
      {"target", "value"},
      {"target1", "value1"},
      {"target2", "value2"},
    });

  CHECK(entity.hasNumberedProperty("target", "value"));
  CHECK(entity.hasNumberedProperty("target", "value1"));
  CHECK(entity.hasNumberedProperty("target", "value2"));
  CHECK(!entity.hasNumberedProperty("targe", "value"));
  CHECK(!entity.hasNumberedProperty("somename", ""));
}

TEST_CASE("EntityTest.property")
{
  Entity entity;

  CHECK(entity.property("key") == nullptr);

  entity.addOrUpdateProperty({}, "key", "value");
  CHECK(entity.property("key") != nullptr);
  CHECK(*entity.property("key") == "value");
}

TEST_CASE("EntityTest.classname")
{
  Entity entity;
  REQUIRE(!entity.hasProperty(EntityPropertyKeys::Classname));

  SECTION("Entities without a classname property return a default name")
  {
    CHECK(entity.classname() == EntityPropertyValues::NoClassname);
  }

  entity.addOrUpdateProperty({}, EntityPropertyKeys::Classname, "testclass");
  SECTION("Entities with a classname property return the value")
  {
    CHECK(*entity.property(EntityPropertyKeys::Classname) == "testclass");
    CHECK(entity.classname() == "testclass");
  }

  SECTION("addOrUpdateProperty updates cached classname property")
  {
    entity.addOrUpdateProperty({}, EntityPropertyKeys::Classname, "newclass");
    CHECK(*entity.property(EntityPropertyKeys::Classname) == "newclass");
    CHECK(entity.classname() == "newclass");
  }

  SECTION("setProperties updates cached classname property")
  {
    entity.setProperties({}, {{EntityPropertyKeys::Classname, "newclass"}});
    CHECK(*entity.property(EntityPropertyKeys::Classname) == "newclass");
    CHECK(entity.classname() == "newclass");
  }
}

TEST_CASE("EntityTest.setClassname")
{
  Entity entity;
  REQUIRE(entity.classname() == EntityPropertyValues::NoClassname);

  entity.setClassname({}, "testclass");
  CHECK(*entity.property(EntityPropertyKeys::Classname) == "testclass");
  CHECK(entity.classname() == "testclass");

  SECTION("Updates cached classname property")
  {
    entity.setClassname({}, "otherclass");
    CHECK(*entity.property(EntityPropertyKeys::Classname) == "otherclass");
    CHECK(entity.classname() == "otherclass");
  }
}

TEST_CASE("EntityTest.origin")
{
  Entity entity;
  REQUIRE(!entity.hasProperty(EntityPropertyKeys::Origin));

  SECTION("Entities without an origin property return 0,0,0")
  {
    CHECK(entity.origin() == vm::vec3::zero());
  }

  entity.addOrUpdateProperty({}, EntityPropertyKeys::Origin, "1 2 3");
  SECTION("Entities with an origin property return the value")
  {
    CHECK(*entity.property(EntityPropertyKeys::Origin) == "1 2 3");
    CHECK(entity.origin() == vm::vec3(1, 2, 3));
  }

  SECTION("addOrUpdateProperty updates cached classname property")
  {
    entity.addOrUpdateProperty({}, EntityPropertyKeys::Origin, "1 2 3");
    CHECK(*entity.property(EntityPropertyKeys::Origin) == "1 2 3");
    CHECK(entity.origin() == vm::vec3(1, 2, 3));
  }

  SECTION("setProperties updates cached classname property")
  {
    entity.setProperties({}, {{EntityPropertyKeys::Origin, "3 4 5"}});
    CHECK(*entity.property(EntityPropertyKeys::Origin) == "3 4 5");
    CHECK(entity.origin() == vm::vec3(3, 4, 5));
  }
}

TEST_CASE("EntityTest.setOrigin")
{
  // needs to be created here so that it is destroyed last
  auto definition = Assets::PointEntityDefinition{
    "some_name",
    Color{},
    vm::bbox3{32.0},
    "",
    {},
    Assets::ModelDefinition{
      {EL::MapExpression{{{"scale", {EL::VariableExpression{"modelscale"}, 0, 0}}}},
       0,
       0}}};

  Entity entity;
  REQUIRE(entity.origin() == vm::vec3::zero());

  entity.setOrigin({}, vm::vec3(1, 2, 3));
  CHECK(*entity.property(EntityPropertyKeys::Origin) == "1 2 3");
  CHECK(entity.origin() == vm::vec3(1, 2, 3));

  SECTION("Updates cached origin property")
  {
    entity.setOrigin({}, vm::vec3(3, 4, 5));
    CHECK(*entity.property(EntityPropertyKeys::Origin) == "3 4 5");
    CHECK(entity.origin() == vm::vec3(3, 4, 5));
  }

  SECTION("Updates cached model transformation")
  {
    auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};

    entity.setDefinition(config, &definition);
    REQUIRE(
      entity.modelTransformation()
      == vm::translation_matrix(vm::vec3{1, 2, 3})
           * vm::scaling_matrix(vm::vec3{2, 2, 2}));

    entity.setOrigin(config, vm::vec3{9, 8, 7});
    REQUIRE(
      entity.modelTransformation()
      == vm::translation_matrix(vm::vec3{9, 8, 7})
           * vm::scaling_matrix(vm::vec3{2, 2, 2}));
  }
}

TEST_CASE("EntityTest.transform")
{
  // need to be created here so that they are destroyed last
  auto definition = Assets::PointEntityDefinition(
    "some_name", Color(), vm::bbox3(16.0).translate(vm::vec3(16, 16, 0)), "", {}, {});
  auto otherDefinition =
    Assets::PointEntityDefinition{"some_class", Color{}, vm::bbox3{32.0}, "", {}, {}};

  Entity entity;
  REQUIRE(entity.rotation() == vm::mat4x4::identity());

  SECTION("Requires classname for rotation")
  {
    const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
    entity.transform({}, rotation);

    // rotation had no effect
    CHECK(entity.rotation() == vm::mat4x4::identity());
  }

  SECTION("Requires point entity for rotation")
  {
    entity.setClassname({}, "some_class");
    entity.setPointEntity({}, false);
    REQUIRE(entity.rotation() == vm::mat4x4::identity());

    const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
    entity.transform({}, rotation);

    // rotation had no effect
    CHECK(entity.rotation() == vm::mat4x4::identity());
  }

  SECTION("Rotate - without offset")
  {
    entity.setClassname({}, "some_class");
    entity.setOrigin({}, vm::vec3(10, 20, 30));

    const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
    entity.transform({}, rotation);

    CHECK(entity.rotation() == rotation);
    CHECK(entity.origin() == vm::vec3(-20, 10, 30));
  }

  SECTION("Rotate - with offset")
  {
    entity.setClassname({}, "some_class");
    entity.setOrigin({}, vm::vec3(32, 32, 0));

    entity.setDefinition({}, &definition);

    const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
    entity.transform({}, rotation);

    CHECK(entity.rotation() == vm::mat4x4::identity());
    CHECK(entity.origin() == vm::vec3(-64, 32, 0));
  }

  SECTION("Rotate - with subsequent translation")
  {
    entity.setClassname({}, "some_class");

    const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
    entity.transform({}, rotation);
    REQUIRE(entity.rotation() == rotation);

    entity.transform({}, vm::translation_matrix(vm::vec3(100, 0, 0)));
    CHECK(entity.rotation() == rotation);
  }

  SECTION("Updates cached model transformation")
  {
    entity.setClassname({}, "some_class");

    auto config = EntityPropertyConfig{{{EL::LiteralExpression{EL::Value{2.0}}, 0, 0}}};

    entity.setDefinition(config, &otherDefinition);
    REQUIRE(entity.modelTransformation() == vm::scaling_matrix(vm::vec3{2, 2, 2}));

    entity.transform(config, vm::translation_matrix(vm::vec3{8, 7, 6}));
    CHECK(
      entity.modelTransformation()
      == vm::translation_matrix(vm::vec3{8, 7, 6})
           * vm::scaling_matrix(vm::vec3{2, 2, 2}));
  }

  SECTION("Updates angle property")
  {
    auto entityPropertyConfig = EntityPropertyConfig{};

    entity.setClassname(entityPropertyConfig, "light");
    entity.addOrUpdateProperty(entityPropertyConfig, EntityPropertyKeys::Angle, "0");

    const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));

    SECTION("If property update after transform is enabled")
    {
      entity.transform(entityPropertyConfig, rotation);
      CHECK(*entity.property(EntityPropertyKeys::Angle) == "90");
    }

    SECTION("If property update after transform is disabled")
    {
      entityPropertyConfig.updateAnglePropertyAfterTransform = false;

      entity.transform(entityPropertyConfig, rotation);
      CHECK(*entity.property(EntityPropertyKeys::Angle) == "0");
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
