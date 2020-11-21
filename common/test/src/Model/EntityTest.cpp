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
#include "Model/EntityAttributes.h"

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

            CHECK(entity.classname() == AttributeValues::NoClassname);
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

        TEST_CASE("EntityTest.addOrUpdateAttribute") {
            Entity entity;
            REQUIRE(entity.attribute("test") == nullptr);

            entity.addOrUpdateAttribute("test", "value");
            CHECK(*entity.attribute("test") == "value");

            entity.addOrUpdateAttribute("test", "newValue");
            CHECK(*entity.attribute("test") == "newValue");
        }

        TEST_CASE("EntityTest.renameAttribute") {
            Entity entity;

            SECTION("Rename non existing attribute") {
                REQUIRE(!entity.hasAttribute("originalName"));
                entity.renameAttribute("originalName", "newName");
                CHECK(!entity.hasAttribute("originalName"));
                CHECK(!entity.hasAttribute("newName"));
            }

            entity.addOrUpdateAttribute("originalName", "originalValue");
            REQUIRE(*entity.attribute("originalName") == "originalValue");

            SECTION("Rename existing attribute") {
                entity.renameAttribute("originalName", "newName");
                CHECK(!entity.hasAttribute("originalName"));
                CHECK(*entity.attribute("newName") == "originalValue");
            }

            SECTION("Rename existing attribute - name conflict") {
                entity.addOrUpdateAttribute("newName", "newValue");

                entity.renameAttribute("originalName", "newName");
                CHECK(!entity.hasAttribute("originalName"));
                CHECK(*entity.attribute("newName") == "originalValue");
            }
        }

        TEST_CASE("EntityTest.removeAttribute") {
            Entity entity;

            SECTION("Remove non existing attribute") {
                REQUIRE(!entity.hasAttribute("name"));
                entity.removeAttribute("name");
                CHECK(!entity.hasAttribute("name"));
            }

            SECTION("Remove existing attribute") {
                entity.addOrUpdateAttribute("name", "value");
                entity.removeAttribute("name");
                CHECK(!entity.hasAttribute("name"));
            }
        }

        TEST_CASE("EntityTest.hasAttribute") {
            Entity entity;
            CHECK(!entity.hasAttribute("value"));
            
            entity.setAttributes({EntityAttribute("name", "value")});
            CHECK(entity.hasAttribute("name"));
        }

        TEST_CASE("EntityTest.originUpdateWithSetAttributes") {
            Entity entity;
            entity.setAttributes({EntityAttribute("origin", "10 20 30")});

            CHECK(entity.origin() == vm::vec3(10, 20, 30));
        }

        TEST_CASE("EntityTest.hasAttributeWithPrefix") {
            Entity entity;
            entity.setAttributes({
                EntityAttribute("somename", "somevalue"),
                EntityAttribute("someothername", "someothervalue"),
            });

            CHECK(entity.hasAttributeWithPrefix("somename", "somevalue"));
            CHECK(entity.hasAttributeWithPrefix("some", "somevalue"));
            CHECK(entity.hasAttributeWithPrefix("some", "someothervalue"));
            CHECK(entity.hasAttributeWithPrefix("someother", "someothervalue"));
            CHECK(!entity.hasAttributeWithPrefix("someother", "somevalue"));
            CHECK(!entity.hasAttributeWithPrefix("sime", ""));
        }

        TEST_CASE("EntityTest.hasNumberedAttribute") {
            Entity entity;
            entity.setAttributes({
                EntityAttribute("target", "value"),
                EntityAttribute("target1", "value1"),
                EntityAttribute("target2", "value2"),
            });

            CHECK(entity.hasNumberedAttribute("target", "value"));
            CHECK(entity.hasNumberedAttribute("target", "value1"));
            CHECK(entity.hasNumberedAttribute("target", "value2"));
            CHECK(!entity.hasNumberedAttribute("targe", "value"));
            CHECK(!entity.hasNumberedAttribute("somename", ""));
        }

        TEST_CASE("EntityTest.attribute") {
            Entity entity;

            CHECK(entity.attribute("name") == nullptr);
            
            entity.addOrUpdateAttribute("name", "value");
            CHECK(entity.attribute("name") != nullptr);
            CHECK(*entity.attribute("name") == "value");
        }

        TEST_CASE("EntityTest.classname") {
            Entity entity;
            REQUIRE(!entity.hasAttribute(AttributeNames::Classname));

            SECTION("Entities without a classname attribute return a default name") {
                CHECK(entity.classname() == AttributeValues::NoClassname);
            }

            entity.addOrUpdateAttribute(AttributeNames::Classname, "testclass");
            SECTION("Entities with a classname attribute return the value") {
                CHECK(*entity.attribute(AttributeNames::Classname) == "testclass");
                CHECK(entity.classname() == "testclass");
            }

            SECTION("addOrUpdateAttribute updates cached classname attribute") {
                entity.addOrUpdateAttribute(AttributeNames::Classname, "newclass");
                CHECK(*entity.attribute(AttributeNames::Classname) == "newclass");
                CHECK(entity.classname() == "newclass");
            }

            SECTION("setAttributes updates cached classname attribute") {
                entity.setAttributes({
                    EntityAttribute(AttributeNames::Classname, "newclass")
                });
                CHECK(*entity.attribute(AttributeNames::Classname) == "newclass");
                CHECK(entity.classname() == "newclass");
            }
        }

        TEST_CASE("EntityTest.setClassname") {
            Entity entity;
            REQUIRE(entity.classname() == AttributeValues::NoClassname);

            entity.setClassname("testclass");
            CHECK(*entity.attribute(AttributeNames::Classname) == "testclass");
            CHECK(entity.classname() == "testclass");

            SECTION("Updates cached classname attribute") {
                entity.setClassname("otherclass");
                CHECK(*entity.attribute(AttributeNames::Classname) == "otherclass");
                CHECK(entity.classname() == "otherclass");
            }
        }

        TEST_CASE("EntityTest.origin") {
            Entity entity;
            REQUIRE(!entity.hasAttribute(AttributeNames::Origin));

            SECTION("Entities without an origin attribute return 0,0,0") {
                CHECK(entity.origin() == vm::vec3::zero());
            }

            entity.addOrUpdateAttribute(AttributeNames::Origin, "1 2 3");
            SECTION("Entities with an origin attribute return the value") {
                CHECK(*entity.attribute(AttributeNames::Origin) == "1 2 3");
                CHECK(entity.origin() == vm::vec3(1, 2, 3));
            }

            SECTION("addOrUpdateAttribute updates cached classname attribute") {
                entity.addOrUpdateAttribute(AttributeNames::Origin, "1 2 3");
                CHECK(*entity.attribute(AttributeNames::Origin) == "1 2 3");
                CHECK(entity.origin() == vm::vec3(1, 2, 3));
            }

            SECTION("setAttributes updates cached classname attribute") {
                entity.setAttributes({
                    EntityAttribute(AttributeNames::Origin, "3 4 5")
                });
                CHECK(*entity.attribute(AttributeNames::Origin) == "3 4 5");
                CHECK(entity.origin() == vm::vec3(3, 4, 5));
            }
        }

        TEST_CASE("EntityTest.setOrigin") {
            Entity entity;
            REQUIRE(entity.origin() == vm::vec3::zero());

            entity.setOrigin(vm::vec3(1, 2, 3));
            CHECK(*entity.attribute(AttributeNames::Origin) == "1 2 3");
            CHECK(entity.origin() == vm::vec3(1, 2, 3));

            SECTION("Updates cached origin attribute") {
                entity.setOrigin(vm::vec3(3, 4, 5));
                CHECK(*entity.attribute(AttributeNames::Origin) == "3 4 5");
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
