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

#include "FloatType.h"
#include "Assets/EntityDefinition.h"
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

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("EntityTest.defaults") {
            Entity entity;

            CHECK(entity.classname() == PropertyValues::NoClassname);
            CHECK(entity.pointEntity());
            CHECK(entity.origin() == vm::vec3::zero());
            CHECK(entity.rotation() == vm::mat4x4::identity());
        }

        TEST_CASE("EntityTest.definitionBounds") {
            auto pointEntityDefinition = Assets::PointEntityDefinition("some_name", Color(), vm::bbox3(32.0), "", {}, {});
            Entity entity;

            SECTION("Returns default bounds if no definition is set") {
                CHECK(entity.definitionBounds() == vm::bbox3(8.0));
            }
            
            SECTION("Returns definition bounds if definition is set") {
                entity.setDefinition(&pointEntityDefinition);
                CHECK(entity.definitionBounds() == vm::bbox3(32.0));
            }
        }

        TEST_CASE("EntityTest.addOrUpdateProperty") {
            Entity entity;
            REQUIRE(entity.property("test") == nullptr);

            entity.addOrUpdateProperty("test", "value");
            CHECK(*entity.property("test") == "value");

            entity.addOrUpdateProperty("test", "newValue");
            CHECK(*entity.property("test") == "newValue");
        }

        TEST_CASE("EntityTest.renameProperty") {
            Entity entity;

            SECTION("Rename non existing attribute") {
                REQUIRE(!entity.hasProperty("originalName"));
                entity.renameProperty("originalName", "newName");
                CHECK(!entity.hasProperty("originalName"));
                CHECK(!entity.hasProperty("newName"));
            }

            entity.addOrUpdateProperty("originalName", "originalValue");
            REQUIRE(*entity.property("originalName") == "originalValue");

            SECTION("Rename existing attribute") {
                entity.renameProperty("originalName", "newName");
                CHECK(!entity.hasProperty("originalName"));
                CHECK(*entity.property("newName") == "originalValue");
            }

            SECTION("Rename existing attribute - name conflict") {
                entity.addOrUpdateProperty("newName", "newValue");

                entity.renameProperty("originalName", "newName");
                CHECK(!entity.hasProperty("originalName"));
                CHECK(*entity.property("newName") == "originalValue");
            }
        }

        TEST_CASE("EntityTest.removeProperty") {
            Entity entity;

            SECTION("Remove non existing attribute") {
                REQUIRE(!entity.hasProperty("name"));
                entity.removeProperty("name");
                CHECK(!entity.hasProperty("name"));
            }

            SECTION("Remove existing attribute") {
                entity.addOrUpdateProperty("name", "value");
                entity.removeProperty("name");
                CHECK(!entity.hasProperty("name"));
            }
        }

        TEST_CASE("EntityTest.hasProperty") {
            Entity entity;
            CHECK(!entity.hasProperty("value"));

            entity.setProperties({ EntityProperty("name", "value") });
            CHECK(entity.hasProperty("name"));
        }

        TEST_CASE("EntityTest.originUpdateWithSetAttributes") {
            Entity entity;
            entity.setProperties({ EntityProperty("origin", "10 20 30") });

            CHECK(entity.origin() == vm::vec3(10, 20, 30));
        }

        TEST_CASE("EntityTest.hasPropertyWithPrefix") {
            Entity entity;
            entity.setProperties({
                EntityProperty("somename", "somevalue"),
                EntityProperty("someothername", "someothervalue"),
            });

            CHECK(entity.hasPropertyWithPrefix("somename", "somevalue"));
            CHECK(entity.hasPropertyWithPrefix("some", "somevalue"));
            CHECK(entity.hasPropertyWithPrefix("some", "someothervalue"));
            CHECK(entity.hasPropertyWithPrefix("someother", "someothervalue"));
            CHECK(!entity.hasPropertyWithPrefix("someother", "somevalue"));
            CHECK(!entity.hasPropertyWithPrefix("sime", ""));
        }

        TEST_CASE("EntityTest.hasNumberedProperty") {
            Entity entity;
            entity.setProperties({
                EntityProperty("target", "value"),
                EntityProperty("target1", "value1"),
                EntityProperty("target2", "value2"),
            });

            CHECK(entity.hasNumberedProperty("target", "value"));
            CHECK(entity.hasNumberedProperty("target", "value1"));
            CHECK(entity.hasNumberedProperty("target", "value2"));
            CHECK(!entity.hasNumberedProperty("targe", "value"));
            CHECK(!entity.hasNumberedProperty("somename", ""));
        }

        TEST_CASE("EntityTest.attribute") {
            Entity entity;

            CHECK(entity.property("name") == nullptr);

            entity.addOrUpdateProperty("name", "value");
            CHECK(entity.property("name") != nullptr);
            CHECK(*entity.property("name") == "value");
        }

        TEST_CASE("EntityTest.classname") {
            Entity entity;
            REQUIRE(!entity.hasProperty(PropertyKeys::Classname));

            SECTION("Entities without a classname attribute return a default name") {
                CHECK(entity.classname() == PropertyValues::NoClassname);
            }

            entity.addOrUpdateProperty(PropertyKeys::Classname, "testclass");
            SECTION("Entities with a classname attribute return the value") {
                CHECK(*entity.property(PropertyKeys::Classname) == "testclass");
                CHECK(entity.classname() == "testclass");
            }

            SECTION("addOrUpdateProperty updates cached classname attribute") {
                entity.addOrUpdateProperty(PropertyKeys::Classname, "newclass");
                CHECK(*entity.property(PropertyKeys::Classname) == "newclass");
                CHECK(entity.classname() == "newclass");
            }

            SECTION("setProperties updates cached classname attribute") {
                entity.setProperties({
                    EntityProperty(PropertyKeys::Classname, "newclass")
                });
                CHECK(*entity.property(PropertyKeys::Classname) == "newclass");
                CHECK(entity.classname() == "newclass");
            }
        }

        TEST_CASE("EntityTest.setClassname") {
            Entity entity;
            REQUIRE(entity.classname() == PropertyValues::NoClassname);

            entity.setClassname("testclass");
            CHECK(*entity.property(PropertyKeys::Classname) == "testclass");
            CHECK(entity.classname() == "testclass");

            SECTION("Updates cached classname attribute") {
                entity.setClassname("otherclass");
                CHECK(*entity.property(PropertyKeys::Classname) == "otherclass");
                CHECK(entity.classname() == "otherclass");
            }
        }

        TEST_CASE("EntityTest.origin") {
            Entity entity;
            REQUIRE(!entity.hasProperty(PropertyKeys::Origin));

            SECTION("Entities without an origin attribute return 0,0,0") {
                CHECK(entity.origin() == vm::vec3::zero());
            }

            entity.addOrUpdateProperty(PropertyKeys::Origin, "1 2 3");
            SECTION("Entities with an origin attribute return the value") {
                CHECK(*entity.property(PropertyKeys::Origin) == "1 2 3");
                CHECK(entity.origin() == vm::vec3(1, 2, 3));
            }

            SECTION("addOrUpdateProperty updates cached classname attribute") {
                entity.addOrUpdateProperty(PropertyKeys::Origin, "1 2 3");
                CHECK(*entity.property(PropertyKeys::Origin) == "1 2 3");
                CHECK(entity.origin() == vm::vec3(1, 2, 3));
            }

            SECTION("setProperties updates cached classname attribute") {
                entity.setProperties({
                    EntityProperty(PropertyKeys::Origin, "3 4 5")
                });
                CHECK(*entity.property(PropertyKeys::Origin) == "3 4 5");
                CHECK(entity.origin() == vm::vec3(3, 4, 5));
            }
        }

        TEST_CASE("EntityTest.setOrigin") {
            Entity entity;
            REQUIRE(entity.origin() == vm::vec3::zero());

            entity.setOrigin(vm::vec3(1, 2, 3));
            CHECK(*entity.property(PropertyKeys::Origin) == "1 2 3");
            CHECK(entity.origin() == vm::vec3(1, 2, 3));

            SECTION("Updates cached origin attribute") {
                entity.setOrigin(vm::vec3(3, 4, 5));
                CHECK(*entity.property(PropertyKeys::Origin) == "3 4 5");
                CHECK(entity.origin() == vm::vec3(3, 4, 5));
            }
        }

        TEST_CASE("EntityTest.requiresClassnameForRotation") {
            Entity entity;
            REQUIRE(entity.rotation() == vm::mat4x4::identity());

            const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
            entity.transform(rotation);

            // rotation had no effect
            CHECK(entity.rotation() == vm::mat4x4::identity());
        }

        TEST_CASE("EntityTest.requiresPointEntityForRotation") {
            Entity entity;
            entity.setClassname("some_class");
            entity.setPointEntity(false);
            REQUIRE(entity.rotation() == vm::mat4x4::identity());

            const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
            entity.transform(rotation);

            // rotation had no effect
            CHECK(entity.rotation() == vm::mat4x4::identity());
        }

        TEST_CASE("EntityTest.rotateWithoutOffset") {
            Entity entity;
            entity.setClassname("some_class");
            entity.setOrigin(vm::vec3(10, 20, 30));

            const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
            entity.transform(rotation);

            CHECK(entity.rotation() == rotation);
            CHECK(entity.origin() == vm::vec3(-20, 10, 30));
        }

        TEST_CASE("EntityTest.rotateWithOffset") {
            auto definition = Assets::PointEntityDefinition("some_name", Color(), vm::bbox3(16.0).translate(vm::vec3(16, 16, 0)), "", {}, {});

            Entity entity;
            entity.setClassname("some_class");
            entity.setOrigin(vm::vec3(32, 32, 0));
            entity.setDefinition(&definition);

            const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
            entity.transform(rotation);

            CHECK(entity.rotation() == vm::mat4x4::identity());
            CHECK(entity.origin() == vm::vec3(-64, 32, 0));
        }

        TEST_CASE("EntityTest.translateAfterRotation") {
            Entity entity;
            entity.setClassname("some_class");

            const auto rotation = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
            entity.transform(rotation);
            REQUIRE(entity.rotation() == rotation);

            entity.transform(vm::translation_matrix(vm::vec3(100, 0, 0)));
            CHECK(entity.rotation() == rotation);
        }
    }
}
