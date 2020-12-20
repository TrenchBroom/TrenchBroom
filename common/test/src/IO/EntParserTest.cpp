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

#include "Exceptions.h"
#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "IO/DiskIO.h"
#include "IO/EntParser.h"
#include "IO/FileMatcher.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"

#include <kdl/vector_utils.h>

#include <algorithm>
#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
        static void assertPropertyDefinition(const std::string& key, const Assets::PropertyDefinitionType expectedType, const Assets::EntityDefinition* entityDefinition) {
            const auto* propDefinition = entityDefinition->propertyDefinition(key);
            UNSCOPED_INFO("Missing property definition for '" + key + "' key");
            ASSERT_NE(nullptr, propDefinition);

            UNSCOPED_INFO("Expected '" + key + "' property definition to be of expected type");
            ASSERT_EQ(expectedType, propDefinition->type());
        }

        TEST_CASE("EntParserTest.parseIncludedEntFiles", "[EntParserTest]") {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
            const std::vector<Path> cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("ent"));

            for (const Path& path : cfgFiles) {
                auto file = Disk::openFile(path);
                auto reader = file->reader().buffer();

                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                EntParser parser(reader.stringView(), defaultColor);

                TestParserStatus status;
                UNSCOPED_INFO("Parsing ENT file " << path.asString() << " failed");
                CHECK_NOTHROW(parser.parseDefinitions(status));

                /* Disabled because our files are full of previously undetected problems
                if (status.countStatus(LogLevel::Warn) > 0u) {
                    UNSCOPED_INFO("Parsing ENT file " << path.asString() << " produced warnings");
                    for (const auto& message : status.messages(LogLevel::Warn)) {
                        UNSCOPED_INFO(message);
                    }
                    CHECK(status.countStatus(LogLevel::Warn) == 0u);
                }

                if (status.countStatus(LogLevel::Error) > 0u) {
                    UNSCOPED_INFO("Parsing ENT file " << path.asString() << " produced errors");
                    for (const auto& message : status.messages(LogLevel::Error)) {
                        UNSCOPED_INFO(message);
                    }
                    CHECK(status.countStatus(LogLevel::Error) == 0u);
                }
                */
            }
        }

        TEST_CASE("EntParserTest.parseEmptyFile", "[EntParserTest]") {
            const std::string file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("EntParserTest.parseWhitespaceFile", "[EntParserTest]") {
            const std::string file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("EntParserTest.parseMalformedXML", "[EntParserTest]") {
            const std::string file =
R"(<?xml version="1.0"?>
<classes>
    <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
</classes>)";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            ASSERT_THROW(parser.parseDefinitions(status), ParserException);
        }

        TEST_CASE("EntParserTest.parseSimplePointEntityDefinition", "[EntParserTest]") {
            const std::string file = R"(
<?xml version="1.0"?>
<!--
Quake3 Arena entity definition file for Q3Radiant
Based on draft by Suicide 20 7.30.99 and inolen 9-3-99
Upgraded by Eutectic: eutectic@ritualistic.com
(visible models added by raYGunn - paths provided by Suicide 20)
(terrain information added to func_group entity by Paul Jaquays)
Q3Map2 entities/keys added by ydnar
Additional Q3Map2 and Q3A PR 1.32 entities/keys added by Obsidian
Entities.def for GtkRadiant 1.4 and ZeroRadiant 1.6
Entities.ent for GtkRadiant 1.5
Version: 1.7.3
Updated: 2011-03-02
-->
<classes>
    <!--
    =============================================================================

    Q3MAP2 ENTITIES

    =============================================================================
    -->

    <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
    -------- KEYS --------
    asdf<angle key="angle" name="Yaw Angle">Rotation angle of the sky surfaces.</angle>
    <angles key="angles" name="Pitch Yaw Roll">Individual control of PITCH, YAW, and ROLL (default 0 0 0).</angles>
    <real key="_scale" name="Scale" value="64">Scaling factor (default 64), good values are between 50 and 300, depending on the map.</real>
    -------- NOTES --------
    Compiler-only entity that specifies the origin of a skybox (a wholly contained, separate area of the map), similar to some games portal skies. When compiled with Q3Map2, the skybox surfaces will be visible from any place where sky is normally visible. It will cast shadows on the normal parts of the map, and can be used with cloud layers and other effects.
    </point>
</classes>
)";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            UNSCOPED_INFO("Expected one entity definition");
            ASSERT_EQ(1u, definitions.size());

            const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(definitions.front());
            UNSCOPED_INFO("Definition must be a point entity definition");
            ASSERT_NE(nullptr, pointDefinition);

            const auto expectedDescription = R"(
    -------- KEYS --------
    asdf
    -------- NOTES --------
    Compiler-only entity that specifies the origin of a skybox (a wholly contained, separate area of the map), similar to some games portal skies. When compiled with Q3Map2, the skybox surfaces will be visible from any place where sky is normally visible. It will cast shadows on the normal parts of the map, and can be used with cloud layers and other effects.
    )";
            UNSCOPED_INFO("Expected text value as entity defintion description");
            ASSERT_EQ(expectedDescription, pointDefinition->description());

            UNSCOPED_INFO("Expected matching color");
            ASSERT_TRUE(vm::is_equal(Color(0.77f, 0.88f, 1.0f, 1.0f), pointDefinition->color(), 0.01f));

            UNSCOPED_INFO("Expected matching bounds");
            ASSERT_TRUE(vm::is_equal(vm::bbox3(vm::vec3(-4.0, -4.0, -4.0), vm::vec3(+4.0, +4.0, +4.0)), pointDefinition->bounds(), 0.01));

            UNSCOPED_INFO("Expected three property definitions");
            ASSERT_EQ(3u, pointDefinition->propertyDefinitions().size());

            const auto* angleDefinition = pointDefinition->propertyDefinition("angle");
            UNSCOPED_INFO("Missing property definition for 'angle' key");
            ASSERT_NE(nullptr, angleDefinition);

            UNSCOPED_INFO("Expected angle property definition to be of String type");
            ASSERT_EQ(Assets::PropertyDefinitionType::StringProperty, angleDefinition->type());

            UNSCOPED_INFO("Expected matching property definition name");
            ASSERT_EQ("angle", angleDefinition->key());

            UNSCOPED_INFO("Expected property definition's short description to match name");
            ASSERT_EQ("Yaw Angle", angleDefinition->shortDescription());

            UNSCOPED_INFO("Expected property definition's long description to match element text");
            ASSERT_EQ("Rotation angle of the sky surfaces.", angleDefinition->longDescription());

            const auto* anglesDefinition = pointDefinition->propertyDefinition("angles");
            UNSCOPED_INFO("Missing property definition for 'angles' key");
            ASSERT_NE(nullptr, anglesDefinition);
            
            UNSCOPED_INFO("Expected angles property definition to be of String type");
            ASSERT_EQ(Assets::PropertyDefinitionType::StringProperty, anglesDefinition->type());

            UNSCOPED_INFO("Expected matching property definition name");
            ASSERT_EQ("angles", anglesDefinition->key());

            UNSCOPED_INFO("Expected property definition's short description to match name");
            ASSERT_EQ("Pitch Yaw Roll", anglesDefinition->shortDescription());

            UNSCOPED_INFO("Expected property definition's long description to match element text");
            ASSERT_EQ("Individual control of PITCH, YAW, and ROLL (default 0 0 0).", anglesDefinition->longDescription());

            const auto* scaleDefinition = dynamic_cast<const Assets::FloatPropertyDefinition*>(pointDefinition->propertyDefinition(
                "_scale"));
            UNSCOPED_INFO("Missing property definition for '_scale' key");
            ASSERT_NE(nullptr, scaleDefinition);

            UNSCOPED_INFO("Expected angles property definition to be of Float type");
            ASSERT_EQ(Assets::PropertyDefinitionType::FloatProperty, scaleDefinition->type());

            UNSCOPED_INFO("Expected matching property definition name");
            ASSERT_EQ("_scale", scaleDefinition->key());

            UNSCOPED_INFO("Expected property definition's short description to match name");
            ASSERT_EQ("Scale", scaleDefinition->shortDescription());

            UNSCOPED_INFO("Expected correct default value for '_scale' property definition");
            ASSERT_EQ(64.0f, scaleDefinition->defaultValue());

            UNSCOPED_INFO("Expected property definition's long description to match element text");
            ASSERT_EQ("Scaling factor (default 64), good values are between 50 and 300, depending on the map.", scaleDefinition->longDescription());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("EntParserTest.parseSimpleGroupEntityDefinition", "[EntParserTest]") {
            const std::string file = R"(
<?xml version="1.0"?>
<classes>
<group name="func_bobbing" color="0 .4 1">
Solid entity that oscillates back and forth in a linear motion. By default, it will have an amount of displacement in either direction equal to the dimension of the brush in the axis in which it's bobbing. Entity bobs on the Z axis (up-down) by default. It can also emit sound if the "noise" key is set. Will crush the player when blocked.
-------- KEYS --------
<sound key="noise" name="Sound File">Path/name of .wav file to play. Use looping sounds only (e.g. sound/world/drone6.wav - see notes).</sound>
<model key="model2" name="Model File">Path/name of model to include (.md3 files only, e.g. models/mapobjects/jets/jets01.md3).</model>
<color key="color" name="Model Light Color" value="1 1 1">Color of constant light of .md3 model, included with entity (default 1 1 1).</color>
-------- Q3MAP2 KEYS --------
<targetname key="targetname" name="Target Name">Used to attach a misc_model entity to this entity.</targetname>
<integer key="_castshadows" name="Shadow Caster Level" value="0">Allows per-entity control over shadow casting. Defaults to 0 on entities, 1 on world. 0 = no shadow casting. 1 = cast shadows on world. &gt; 1 = cast shadows on entities with _rs (or _receiveshadows) with the corresponding value, AND world. Negative values imply same, but DO NOT cast shadows on world.</integer>
<texture key="_celshader" name="Cel Shader">Sets the cel shader used for this geometry. Note: Omit the "textures/" prefix.</texture>
-------- SPAWNFLAGS --------
<flag key="X_AXIS" name="X Axis" bit="0">Entity will bob along the X axis.</flag>
<flag key="Y_AXIS" name="Y Axis" bit="1">Entity will bob along the Y axis.</flag>
-------- NOTES --------
In order for the sound to be emitted from the entity, it is recommended to include a brush with an origin shader at its center, otherwise the sound will not follow the entity as it moves. When using the model2 key, the origin point of the model will correspond to the origin point defined by the origin brush.

Target this entity with a misc_model to have the model attached to the entity (set the model's "target" key to the same value as this entity's "targetname").
</group>
</classes>)";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            UNSCOPED_INFO("Expected one entity definition");
            ASSERT_EQ(1u, definitions.size());

            const auto* brushDefinition = dynamic_cast<const Assets::BrushEntityDefinition*>(definitions.front());
            UNSCOPED_INFO("Definition must be a brush entity definition");
            ASSERT_NE(nullptr, brushDefinition);

            const auto expectedDescription = R"(
Solid entity that oscillates back and forth in a linear motion. By default, it will have an amount of displacement in either direction equal to the dimension of the brush in the axis in which it's bobbing. Entity bobs on the Z axis (up-down) by default. It can also emit sound if the "noise" key is set. Will crush the player when blocked.
-------- KEYS --------

-------- NOTES --------
In order for the sound to be emitted from the entity, it is recommended to include a brush with an origin shader at its center, otherwise the sound will not follow the entity as it moves. When using the model2 key, the origin point of the model will correspond to the origin point defined by the origin brush.

Target this entity with a misc_model to have the model attached to the entity (set the model's "target" key to the same value as this entity's "targetname").
)";
            UNSCOPED_INFO("Expected text value as entity defintion description");
            ASSERT_EQ(expectedDescription, brushDefinition->description());

            UNSCOPED_INFO("Expected matching color");
            ASSERT_TRUE(vm::is_equal(Color(0.0f, 0.4f, 1.0f), brushDefinition->color(), 0.01f));

            UNSCOPED_INFO("Expected seven property definitions");
            ASSERT_EQ(7u, brushDefinition->propertyDefinitions().size());
            assertPropertyDefinition("noise", Assets::PropertyDefinitionType::StringProperty, brushDefinition);
            assertPropertyDefinition("model2", Assets::PropertyDefinitionType::StringProperty, brushDefinition);
            assertPropertyDefinition("color", Assets::PropertyDefinitionType::StringProperty, brushDefinition);
            assertPropertyDefinition("targetname", Assets::PropertyDefinitionType::TargetSourceProperty,
                brushDefinition);
            assertPropertyDefinition("_castshadows", Assets::PropertyDefinitionType::IntegerProperty, brushDefinition);
            assertPropertyDefinition("_celshader", Assets::PropertyDefinitionType::StringProperty, brushDefinition);
            assertPropertyDefinition("spawnflags", Assets::PropertyDefinitionType::FlagsProperty, brushDefinition);

            UNSCOPED_INFO("Expected matching spawnflag definitions");
            const Assets::FlagsPropertyDefinition* spawnflags = brushDefinition->spawnflags();
            ASSERT_TRUE(spawnflags != nullptr);
            ASSERT_EQ(0, spawnflags->defaultValue());
            const Assets::FlagsPropertyOption::List& options = spawnflags->options();
            ASSERT_EQ(2u, options.size());
            ASSERT_EQ(std::string("X_AXIS"), options[0].shortDescription());
            ASSERT_EQ(std::string("X Axis"), options[0].longDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(1, options[0].value());
            ASSERT_EQ(std::string("Y_AXIS"), options[1].shortDescription());
            ASSERT_EQ(std::string("Y Axis"), options[1].longDescription());
            ASSERT_FALSE(options[1].isDefault());
            ASSERT_EQ(2, options[1].value());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("EntParserTest.parseListPropertyDefinition", "[EntParserTest]") {
            const std::string file = R"(
<?xml version="1.0"?>
<classes>
<list name="colorIndex">
<item name="white" value="0"/>
<item name="red" value="1"/>
<item name="green" value="2"/>
</list>
<point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
<colorIndex key="count" name="Text Color" value="0">Color of the location text displayed in parentheses during team chat. Set to 0-7 for color.
0 : White (default)
1 : Red
2 : Green
3 : Yellow
4 : Blue
5 : Cyan
6 : Magenta
7 : White</colorIndex>
</point>
</classes>
            )";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            UNSCOPED_INFO("Expected one entity definition");
            ASSERT_EQ(1u, definitions.size());

            const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(definitions.front());
            UNSCOPED_INFO("Definition must be a point entity definition");
            ASSERT_NE(nullptr, pointDefinition);

            UNSCOPED_INFO("Expected one property definitions");
            ASSERT_EQ(1u, pointDefinition->propertyDefinitions().size());

            const auto* colorIndexDefinition = dynamic_cast<const Assets::ChoicePropertyDefinition*>(pointDefinition->propertyDefinition(
                "count"));
            UNSCOPED_INFO("Missing property definition for 'count' key");
            ASSERT_NE(nullptr, colorIndexDefinition);

            UNSCOPED_INFO("Expected count property definition to be of choice type");
            ASSERT_EQ(Assets::PropertyDefinitionType::ChoiceProperty, colorIndexDefinition->type());

            UNSCOPED_INFO("Expected name value as entity property definition short description");
            ASSERT_EQ("Text Color", colorIndexDefinition->shortDescription());

            const auto expectedDescription = R"(Color of the location text displayed in parentheses during team chat. Set to 0-7 for color.
0 : White (default)
1 : Red
2 : Green
3 : Yellow
4 : Blue
5 : Cyan
6 : Magenta
7 : White)";
            UNSCOPED_INFO("Expected text value as entity property defintion long description");
            ASSERT_EQ(expectedDescription, colorIndexDefinition->longDescription());

            const auto& options = colorIndexDefinition->options();
            ASSERT_EQ(3u, options.size());

            ASSERT_EQ("0", options[0].value());
            ASSERT_EQ("white", options[0].description());

            ASSERT_EQ("1", options[1].value());
            ASSERT_EQ("red", options[1].description());

            ASSERT_EQ("2", options[2].value());
            ASSERT_EQ("green", options[2].description());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("EntParserTest.parseInvalidRealPropertyDefinition", "[EntParserTest]") {
            const std::string file = R"(
<?xml version="1.0"?>
<classes>
    <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
        <real key="_scale" name="Scale" value="asdf" />
    </point>
</classes>
                        )";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            UNSCOPED_INFO("Expected one entity definition");
            ASSERT_EQ(1u, definitions.size());

            const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(definitions.front());
            UNSCOPED_INFO("Definition must be a point entity definition");
            ASSERT_NE(nullptr, pointDefinition);

            UNSCOPED_INFO("Expected one property definitions");
            ASSERT_EQ(1u, pointDefinition->propertyDefinitions().size());

            const auto* scaleDefinition = dynamic_cast<const Assets::StringPropertyDefinition*>(pointDefinition->propertyDefinition(
                "_scale"));
            UNSCOPED_INFO("Missing property definition for '_scale' key");
            ASSERT_NE(nullptr, scaleDefinition);
            UNSCOPED_INFO("Expected angles property definition to be of Float type");
            ASSERT_EQ(Assets::PropertyDefinitionType::StringProperty, scaleDefinition->type());

            UNSCOPED_INFO("Expected correct default value for '_scale' property definition");
            ASSERT_EQ("asdf", scaleDefinition->defaultValue());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("EntParserTest.parseLegacyModelDefinition", "[EntParserTest]") {
            const std::string file = R"(
<?xml version="1.0"?>
<classes>
<point name="ammo_bfg" color=".3 .3 1" box="-16 -16 -16 16 16 16" model="models/powerups/ammo/bfgam.md3" />
</classes>
            )";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            UNSCOPED_INFO("Expected one entity definition");
            ASSERT_EQ(1u, definitions.size());

            const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(definitions.front());
            UNSCOPED_INFO("Definition must be a point entity definition");
            ASSERT_NE(nullptr, pointDefinition);

            const auto& modelDefinition = pointDefinition->modelDefinition();
            ASSERT_EQ(Path("models/powerups/ammo/bfgam.md3"), modelDefinition.defaultModelSpecification().path);

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("EntParserTest.parseELStaticModelDefinition", "[EntParserTest]") {
            const std::string file = R"(
            <?xml version="1.0"?>
            <classes>
            <point name="ammo_bfg" color=".3 .3 1" box="-16 -16 -16 16 16 16" model="{{ spawnflags == 1 -> 'models/powerups/ammo/bfgam.md3', 'models/powerups/ammo/bfgam2.md3' }}" />
            </classes>
            )";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            UNSCOPED_INFO("Expected one entity definition");
            ASSERT_EQ(1u, definitions.size());

            const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(definitions.front());
            UNSCOPED_INFO("Definition must be a point entity definition");
            ASSERT_NE(nullptr, pointDefinition);

            const auto& modelDefinition = pointDefinition->modelDefinition();
            ASSERT_EQ(Path("models/powerups/ammo/bfgam2.md3"), modelDefinition.defaultModelSpecification().path);

            kdl::vec_clear_and_delete(definitions);
        }
    }
}
