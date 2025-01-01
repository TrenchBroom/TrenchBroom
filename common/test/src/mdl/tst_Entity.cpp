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

#include "el/Expression.h"
#include "io/ELParser.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityProperties.h"
#include "mdl/PropertyDefinition.h"

#include "kdl/k.h"

#include "vm/bbox.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("EntityTest")
{
  SECTION("defaults")
  {
    auto entity = Entity{};

    CHECK(entity.classname() == EntityPropertyValues::NoClassname);
    CHECK(entity.pointEntity());
    CHECK(entity.origin() == vm::vec3d{0, 0, 0});
    CHECK(entity.rotation() == vm::mat4x4d::identity());
  }

  SECTION("setProperties")
  {
    SECTION("Updates cached model transformation")
    {
      auto definition = PointEntityDefinition{
        "some_name",
        Color{},
        vm::bbox3d{32.0},
        "",
        {},
        ModelDefinition{el::ExpressionNode{el::MapExpression{
          {{"scale", el::ExpressionNode{el::VariableExpression{"modelscale"}}}}}}},
        {}};

      auto entity = Entity{};
      entity.setDefinition(&definition);

      const auto defaultModelScaleExpression =
        el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}};

      REQUIRE(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{2, 2, 2}));

      entity.setProperties({{"modelscale", "1 2 3"}});

      CHECK(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{1, 2, 3}));
    }
  }

  SECTION("setDefaultProperties")
  {
    const auto propertyConfig = EntityPropertyConfig{};

    auto definition = PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      {
        std::make_shared<StringPropertyDefinition>("some_prop", "", "", !K(readOnly)),
        std::make_shared<StringPropertyDefinition>(
          "some_default_prop", "", "", !K(readOnly), "value"),
      },
      {},
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

    auto entity = Entity{initialProperties};
    setDefaultProperties(definition, entity, mode);

    CHECK_THAT(entity.properties(), Catch::Matchers::UnorderedEquals(expectedProperties));
  }

  SECTION("definitionBounds")
  {
    auto pointEntityDefinition =
      PointEntityDefinition("some_name", Color{}, vm::bbox3d{32.0}, "", {}, {}, {});
    auto entity = Entity{};

    SECTION("Returns default bounds if no definition is set")
    {
      CHECK(entity.definitionBounds() == vm::bbox3d{8.0});
    }

    SECTION("Returns definition bounds if definition is set")
    {
      entity.setDefinition(&pointEntityDefinition);
      CHECK(entity.definitionBounds() == vm::bbox3d{32.0});
    }
  }

  SECTION("setDefinition")
  {
    SECTION("Updates cached model transformation")
    {
      auto definition = PointEntityDefinition{
        "some_name",
        Color{},
        vm::bbox3d{32.0},
        "",
        {},
        ModelDefinition{el::ExpressionNode{el::MapExpression{{}}}},
        {}};

      auto entity = Entity{};

      const auto defaultModelScaleExpression =
        el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}};

      REQUIRE(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::mat4x4d::identity());

      entity.setDefinition(&definition);
      CHECK(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{2, 2, 2}));
    }
  }

  SECTION("modelSpecification")
  {
    auto modelExpression = io::ELParser::parseStrict(R"({{ 
      spawnflags == 0 -> "maps/b_shell0.bsp",
      spawnflags == 1 -> "maps/b_shell1.bsp",
                         "maps/b_shell2.bsp"
  }})");

    auto definition = PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      {},
      ModelDefinition{modelExpression},
      {}};

    auto entity = Entity{};
    entity.setDefinition(&definition);
    CHECK(entity.modelSpecification() == ModelSpecification{"maps/b_shell0.bsp", 0, 0});

    entity.addOrUpdateProperty(EntityPropertyKeys::Spawnflags, "1");
    CHECK(entity.modelSpecification() == ModelSpecification{"maps/b_shell1.bsp", 0, 0});
  }

  SECTION("decalSpecification")
  {
    auto decalExpression = io::ELParser::parseStrict(R"({ texture: texture })");

    auto definition = PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      {},
      {},
      DecalDefinition{decalExpression}};

    auto entity = Entity{};
    entity.setDefinition(&definition);
    CHECK(entity.decalSpecification() == DecalSpecification{""});

    entity.addOrUpdateProperty("texture", "decal1");
    CHECK(entity.decalSpecification() == DecalSpecification{"decal1"});
  }

  SECTION("unsetEntityDefinitionAndModel")
  {
    auto definition = PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      {},
      ModelDefinition{el::ExpressionNode{el::MapExpression{{}}}},
      {}};

    auto entity = Entity{};
    entity.setDefinition(&definition);

    const auto defaultModelScaleExpression =
      el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}};

    REQUIRE(
      entity.modelTransformation(defaultModelScaleExpression)
      == vm::scaling_matrix(vm::vec3d{2, 2, 2}));

    entity.unsetEntityDefinitionAndModel();
    CHECK(entity.definition() == nullptr);
    CHECK(
      entity.modelTransformation(defaultModelScaleExpression) == vm::mat4x4d::identity());
  }

  SECTION("addOrUpdateProperty")
  {
    // needs to be created here so that it is destroyed last
    auto definition =
      PointEntityDefinition{"some_name", Color{}, vm::bbox3d{32.0}, "", {}, {}, {}};

    auto entity = Entity{};
    REQUIRE(entity.property("test") == nullptr);

    entity.addOrUpdateProperty("test", "value");
    CHECK(*entity.property("test") == "value");

    entity.addOrUpdateProperty("test", "newValue");
    CHECK(*entity.property("test") == "newValue");

    SECTION("Setting a new property to protected by default")
    {
      entity.addOrUpdateProperty("newKey", "newValue", true);
      CHECK_THAT(
        entity.protectedProperties(),
        Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));

      entity.addOrUpdateProperty("test", "anotherValue", true);
      CHECK_THAT(
        entity.protectedProperties(),
        Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));
    }

    SECTION("Updates cached model transformation")
    {
      entity.setDefinition(&definition);
      REQUIRE(
        entity.modelTransformation(
          el::ExpressionNode{el::LiteralExpression{el::Value{1.0}}})
        == vm::scaling_matrix(vm::vec3d{1, 1, 1}));

      entity.addOrUpdateProperty("something", "else");
      CHECK(
        entity.modelTransformation(
          el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}})
        == vm::scaling_matrix(vm::vec3d{2, 2, 2}));
    }
  }

  SECTION("renameProperty")
  {
    // needs to be created here so that it is destroyed last
    auto definition = PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      {},
      ModelDefinition{el::ExpressionNode{el::MapExpression{
        {{"scale", el::ExpressionNode{el::VariableExpression{"modelscale"}}}}}}},
      {}};

    auto entity = Entity{};

    SECTION("Rename non existing property")
    {
      REQUIRE(!entity.hasProperty("originalKey"));
      entity.renameProperty("originalKey", "newKey");
      CHECK(!entity.hasProperty("originalKey"));
      CHECK(!entity.hasProperty("newKey"));
    }

    entity.addOrUpdateProperty("originalKey", "originalValue");
    REQUIRE(*entity.property("originalKey") == "originalValue");

    SECTION("Rename existing property")
    {
      entity.renameProperty("originalKey", "newKey");
      CHECK(!entity.hasProperty("originalKey"));
      CHECK(*entity.property("newKey") == "originalValue");
    }

    SECTION("Rename existing property - name conflict")
    {
      entity.addOrUpdateProperty("newKey", "newValue");

      entity.renameProperty("originalKey", "newKey");
      CHECK(!entity.hasProperty("originalKey"));
      CHECK(*entity.property("newKey") == "originalValue");
    }

    SECTION("Rename existing protected property")
    {
      entity.setProtectedProperties({"originalKey"});
      entity.renameProperty("originalKey", "newKey");
      CHECK_THAT(
        entity.protectedProperties(),
        Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));
    }

    SECTION("Updates cached model transformation")
    {
      const auto defaultModelScaleExpression =
        el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}};

      entity.setDefinition(&definition);
      entity.addOrUpdateProperty("something", "1 2 3");
      REQUIRE(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{2, 2, 2}));

      entity.renameProperty("something", "modelscale");
      CHECK(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{1, 2, 3}));

      entity.renameProperty("modelscale", "not modelscale");
      CHECK(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{2, 2, 2}));
    }
  }

  SECTION("removeProperty")
  {
    // needs to be created here so that it is destroyed last
    auto definition = PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      {},
      ModelDefinition{el::ExpressionNode{el::MapExpression{
        {{"scale", el::ExpressionNode{el::VariableExpression{"modelscale"}}}}}}},
      {}};

    auto entity = Entity{};

    SECTION("Remove non existing property")
    {
      REQUIRE(!entity.hasProperty("key"));
      entity.removeProperty("key");
      CHECK(!entity.hasProperty("key"));
    }

    SECTION("Remove existing property")
    {
      entity.addOrUpdateProperty("key", "value");
      entity.removeProperty("key");
      CHECK(!entity.hasProperty("key"));
    }

    SECTION("Remove protected property")
    {
      entity.addOrUpdateProperty("newKey", "value", true);
      REQUIRE_THAT(
        entity.protectedProperties(),
        Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));

      entity.removeProperty("newKey");
      REQUIRE(!entity.hasProperty("newKey"));
      CHECK_THAT(
        entity.protectedProperties(),
        Catch::UnorderedEquals(std::vector<std::string>{"newKey"}));
    }

    SECTION("Updates cached model transformation")
    {
      entity.setDefinition(&definition);
      entity.addOrUpdateProperty("modelscale", "1 2 3");

      const auto defaultModelScaleExpression =
        el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}};

      REQUIRE(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{1, 2, 3}));

      entity.removeProperty("modelscale");
      CHECK(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{2, 2, 2}));
    }
  }

  SECTION("hasProperty")
  {
    auto entity = Entity{};
    CHECK(!entity.hasProperty("value"));

    entity.setProperties({{"key", "value"}});
    CHECK(entity.hasProperty("key"));
  }

  SECTION("originUpdateWithSetProperties")
  {
    auto entity = Entity{};
    entity.setProperties({{"origin", "10 20 30"}});

    CHECK(entity.origin() == vm::vec3d{10, 20, 30});
  }

  SECTION("hasPropertyWithPrefix")
  {
    auto entity = Entity{};
    entity.setProperties({
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

  SECTION("hasNumberedProperty")
  {
    auto entity = Entity{};
    entity.setProperties({
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

  SECTION("property")
  {
    auto entity = Entity{};

    CHECK(entity.property("key") == nullptr);

    entity.addOrUpdateProperty("key", "value");
    CHECK(entity.property("key") != nullptr);
    CHECK(*entity.property("key") == "value");
  }

  SECTION("classname")
  {
    auto entity = Entity{};
    REQUIRE(!entity.hasProperty(EntityPropertyKeys::Classname));

    SECTION("Entities without a classname property return a default name")
    {
      CHECK(entity.classname() == EntityPropertyValues::NoClassname);
    }

    entity.addOrUpdateProperty(EntityPropertyKeys::Classname, "testclass");
    SECTION("Entities with a classname property return the value")
    {
      CHECK(*entity.property(EntityPropertyKeys::Classname) == "testclass");
      CHECK(entity.classname() == "testclass");
    }

    SECTION("addOrUpdateProperty updates cached classname property")
    {
      entity.addOrUpdateProperty(EntityPropertyKeys::Classname, "newclass");
      CHECK(*entity.property(EntityPropertyKeys::Classname) == "newclass");
      CHECK(entity.classname() == "newclass");
    }

    SECTION("setProperties updates cached classname property")
    {
      entity.setProperties({{EntityPropertyKeys::Classname, "newclass"}});
      CHECK(*entity.property(EntityPropertyKeys::Classname) == "newclass");
      CHECK(entity.classname() == "newclass");
    }
  }

  SECTION("setClassname")
  {
    auto entity = Entity{};
    REQUIRE(entity.classname() == EntityPropertyValues::NoClassname);

    entity.setClassname("testclass");
    CHECK(*entity.property(EntityPropertyKeys::Classname) == "testclass");
    CHECK(entity.classname() == "testclass");

    SECTION("Updates cached classname property")
    {
      entity.setClassname("otherclass");
      CHECK(*entity.property(EntityPropertyKeys::Classname) == "otherclass");
      CHECK(entity.classname() == "otherclass");
    }
  }

  SECTION("origin")
  {
    auto entity = Entity{};
    REQUIRE(!entity.hasProperty(EntityPropertyKeys::Origin));

    SECTION("Entities without an origin property return 0,0,0")
    {
      CHECK(entity.origin() == vm::vec3d{0, 0, 0});
    }

    SECTION("Entities with invalid origin property return 0,0,0")
    {
      entity.addOrUpdateProperty(EntityPropertyKeys::Origin, "1 2");
      CHECK(entity.origin() == vm::vec3d{0, 0, 0});

      entity.addOrUpdateProperty(EntityPropertyKeys::Origin, "asdf");
      CHECK(entity.origin() == vm::vec3d{0, 0, 0});
    }

    SECTION("Entities with nan origin property return 0,0,0")
    {
      entity.addOrUpdateProperty(EntityPropertyKeys::Origin, "1 2 nan");
      CHECK(entity.origin() == vm::vec3d{0, 0, 0});

      entity.addOrUpdateProperty(EntityPropertyKeys::Origin, "nan nan nan");
      CHECK(entity.origin() == vm::vec3d{0, 0, 0});
    }

    entity.addOrUpdateProperty(EntityPropertyKeys::Origin, "1 2 3");
    SECTION("Entities with an origin property return the value")
    {
      CHECK(*entity.property(EntityPropertyKeys::Origin) == "1 2 3");
      CHECK(entity.origin() == vm::vec3d{1, 2, 3});
    }

    SECTION("addOrUpdateProperty updates cached classname property")
    {
      entity.addOrUpdateProperty(EntityPropertyKeys::Origin, "1 2 3");
      CHECK(*entity.property(EntityPropertyKeys::Origin) == "1 2 3");
      CHECK(entity.origin() == vm::vec3d{1, 2, 3});
    }

    SECTION("setProperties updates cached classname property")
    {
      entity.setProperties({{EntityPropertyKeys::Origin, "3 4 5"}});
      CHECK(*entity.property(EntityPropertyKeys::Origin) == "3 4 5");
      CHECK(entity.origin() == vm::vec3d{3, 4, 5});
    }
  }

  SECTION("setOrigin")
  {
    // needs to be created here so that it is destroyed last
    auto definition = PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      {},
      ModelDefinition{el::ExpressionNode{el::MapExpression{
        {{"scale", el::ExpressionNode{el::VariableExpression{"modelscale"}}}}}}},
      {}};

    auto entity = Entity{};
    REQUIRE(entity.origin() == vm::vec3d{0, 0, 0});

    entity.setOrigin(vm::vec3d{1, 2, 3});
    CHECK(*entity.property(EntityPropertyKeys::Origin) == "1 2 3");
    CHECK(entity.origin() == vm::vec3d{1, 2, 3});

    SECTION("Updates cached origin property")
    {
      entity.setOrigin(vm::vec3d{3, 4, 5});
      CHECK(*entity.property(EntityPropertyKeys::Origin) == "3 4 5");
      CHECK(entity.origin() == vm::vec3d{3, 4, 5});
    }

    SECTION("Updates cached model transformation")
    {
      entity.setDefinition(&definition);

      const auto defaultModelScaleExpression =
        el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}};

      REQUIRE(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::translation_matrix(vm::vec3d{1, 2, 3})
             * vm::scaling_matrix(vm::vec3d{2, 2, 2}));

      entity.setOrigin(vm::vec3d{9, 8, 7});
      REQUIRE(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::translation_matrix(vm::vec3d{9, 8, 7})
             * vm::scaling_matrix(vm::vec3d{2, 2, 2}));
    }
  }

  SECTION("transform")
  {
    // need to be created here so that they are destroyed last
    auto definition = PointEntityDefinition(
      "some_name",
      Color{},
      vm::bbox3d{16.0}.translate(vm::vec3d{16, 16, 0}),
      "",
      {},
      {},
      {});
    auto otherDefinition =
      PointEntityDefinition{"some_class", Color{}, vm::bbox3d{32.0}, "", {}, {}, {}};

    auto entity = Entity{};
    REQUIRE(entity.rotation() == vm::mat4x4d::identity());

    SECTION("Requires classname for rotation")
    {
      const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
      entity.transform(rotation, true);

      // rotation had no effect
      CHECK(entity.rotation() == vm::mat4x4d::identity());
    }

    SECTION("Requires point entity for rotation")
    {
      entity.setClassname("some_class");
      entity.setPointEntity(false);
      REQUIRE(entity.rotation() == vm::mat4x4d::identity());

      const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
      entity.transform(rotation, true);

      // rotation had no effect
      CHECK(entity.rotation() == vm::mat4x4d::identity());
    }

    SECTION("Rotate - without offset")
    {
      entity.setClassname("some_class");
      entity.setOrigin(vm::vec3d{10, 20, 30});

      const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
      entity.transform(rotation, true);

      CHECK(entity.rotation() == rotation);
      CHECK(entity.origin() == vm::vec3d{-20, 10, 30});
    }

    SECTION("Rotate - with offset")
    {
      entity.setClassname("some_class");
      entity.setOrigin(vm::vec3d{32, 32, 0});

      entity.setDefinition(&definition);

      const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
      entity.transform(rotation, true);

      CHECK(entity.rotation() == vm::mat4x4d::identity());
      CHECK(entity.origin() == vm::vec3d{-64, 32, 0});
    }

    SECTION("Rotate - with subsequent translation")
    {
      entity.setClassname("some_class");

      const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
      entity.transform(rotation, true);
      REQUIRE(entity.rotation() == rotation);

      entity.transform(vm::translation_matrix(vm::vec3d{100, 0, 0}), true);
      CHECK(entity.rotation() == rotation);
    }

    SECTION("Updates cached model transformation")
    {
      entity.setClassname("some_class");

      const auto defaultModelScaleExpression =
        el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}};

      entity.setDefinition(&otherDefinition);
      REQUIRE(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::scaling_matrix(vm::vec3d{2, 2, 2}));

      entity.transform(vm::translation_matrix(vm::vec3d{8, 7, 6}), true);
      CHECK(
        entity.modelTransformation(defaultModelScaleExpression)
        == vm::translation_matrix(vm::vec3d{8, 7, 6})
             * vm::scaling_matrix(vm::vec3d{2, 2, 2}));
    }

    SECTION("Updates angle property")
    {
      entity.setClassname("light");
      entity.addOrUpdateProperty(EntityPropertyKeys::Angle, "0");

      const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));

      SECTION("If property update after transform is enabled")
      {
        entity.transform(rotation, true);
        CHECK(*entity.property(EntityPropertyKeys::Angle) == "90");
      }

      SECTION("If property update after transform is disabled")
      {
        entity.transform(rotation, false);
        CHECK(*entity.property(EntityPropertyKeys::Angle) == "0");
      }
    }
  }
}

} // namespace tb::mdl
