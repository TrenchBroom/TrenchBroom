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

#include "el/ELTestUtils.h"
#include "io/DiskIO.h"
#include "io/EntParser.h"
#include "io/TestParserStatus.h"
#include "io/TraversalMode.h"
#include "mdl/EntityDefinition.h"
#include "mdl/PropertyDefinition.h"

#include <filesystem>
#include <string>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::io
{
using namespace mdl::PropertyValueTypes;

TEST_CASE("EntParser")
{
  SECTION("parseIncludedEntFiles")
  {
    const auto basePath = std::filesystem::current_path() / "fixture/games/";
    const auto cfgFiles =
      Disk::find(basePath, TraversalMode::Recursive, makeExtensionPathMatcher({".ent"}))
      | kdl::value();

    for (const auto& path : cfgFiles)
    {
      CAPTURE(path);

      auto file = Disk::openFile(path) | kdl::value();
      auto reader = file->reader().buffer();

      auto parser = EntParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}};

      auto status = TestParserStatus{};
      CHECK(parser.parseDefinitions(status));

      /* Disabled because our files are full of previously undetected problems
      if (status.countStatus(LogLevel::Warn) > 0u) {
          for (const auto& message : status.messages(LogLevel::Warn)) {
          }
          CHECK(status.countStatus(LogLevel::Warn) == 0u);
      }

      if (status.countStatus(LogLevel::Error) > 0u) {
          for (const auto& message : status.messages(LogLevel::Error)) {
          }
          CHECK(status.countStatus(LogLevel::Error) == 0u);
      }
      */
    }
  }

  SECTION("parseEmptyFile")
  {
    const auto file = "";
    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseWhitespaceFile")
  {
    const auto file = R"(     
  	 
  )";

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseMalformedXML")
  {
    const std::string file =
      R"(<?xml version="1.0"?>
<classes>
    <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
</classes>)";

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status).is_error());
  }

  SECTION("parseSimplePointEntityDefinition")
  {
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

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "_skybox",
          Color{0.77f, 0.88f, 1.0f},
          R"(
    -------- KEYS --------
    asdf
    -------- NOTES --------
    Compiler-only entity that specifies the origin of a skybox (a wholly contained, separate area of the map), similar to some games portal skies. When compiled with Q3Map2, the skybox surfaces will be visible from any place where sky is normally visible. It will cast shadows on the normal parts of the map, and can be used with cloud layers and other effects.
    )",
          std::vector<mdl::PropertyDefinition>{
            {"angle", Unknown{}, "Yaw Angle", R"(Rotation angle of the sky surfaces.)"},
            {"angles",
             Unknown{},
             "Pitch Yaw Roll",
             R"(Individual control of PITCH, YAW, and ROLL (default 0 0 0).)"},
            {"_scale",
             Float{64.0f},
             "Scale",
             R"(Scaling factor (default 64), good values are between 50 and 300, depending on the map.)"},
          },
          mdl::PointEntityDefinition{
            {{-4, -4, -4}, {+4, +4, +4}},
            {},
            {},
          },
        },
      });
  }

  SECTION("parseSimpleGroupEntityDefinition")
  {
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

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "func_bobbing",
          Color{0.0f, 0.4f, 1.0f},
          R"(
Solid entity that oscillates back and forth in a linear motion. By default, it will have an amount of displacement in either direction equal to the dimension of the brush in the axis in which it's bobbing. Entity bobs on the Z axis (up-down) by default. It can also emit sound if the "noise" key is set. Will crush the player when blocked.
-------- KEYS --------

-------- NOTES --------
In order for the sound to be emitted from the entity, it is recommended to include a brush with an origin shader at its center, otherwise the sound will not follow the entity as it moves. When using the model2 key, the origin point of the model will correspond to the origin point defined by the origin brush.

Target this entity with a misc_model to have the model attached to the entity (set the model's "target" key to the same value as this entity's "targetname").
)",
          {
            {"spawnflags",
             Flags{{
               {1, "X_AXIS", "X Axis"},
               {2, "Y_AXIS", "Y Axis"},
             }},
             "",
             ""},
            {"noise",
             Unknown{},
             "Sound File",
             R"(Path/name of .wav file to play. Use looping sounds only (e.g. sound/world/drone6.wav - see notes).)"},
            {"model2",
             Unknown{},
             "Model File",
             R"(Path/name of model to include (.md3 files only, e.g. models/mapobjects/jets/jets01.md3).)"},
            {"color",
             Unknown{"1 1 1"},
             "Model Light Color",
             R"(Color of constant light of .md3 model, included with entity (default 1 1 1).)"},
            {"targetname",
             LinkTarget{},
             "Target Name",
             R"(Used to attach a misc_model entity to this entity.)"},
            {"_castshadows",
             Integer{0},
             "Shadow Caster Level",
             R"(Allows per-entity control over shadow casting. Defaults to 0 on entities, 1 on world. 0 = no shadow casting. 1 = cast shadows on world. > 1 = cast shadows on entities with _rs (or _receiveshadows) with the corresponding value, AND world. Negative values imply same, but DO NOT cast shadows on world.)"},
            {"_celshader",
             Unknown{},
             "Cel Shader",
             R"(Sets the cel shader used for this geometry. Note: Omit the "textures/" prefix.)"},
          },
        },
      });
  }

  SECTION("parseListPropertyDefinition")
  {
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

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "_skybox",
          Color{0.77f, 0.88f, 1.0f},
          "",
          {
            {"count",
             Choice{
               {
                 {"0", "white"},
                 {"1", "red"},
                 {"2", "green"},
               },
               "0"},
             "Text Color",
             R"(Color of the location text displayed in parentheses during team chat. Set to 0-7 for color.
0 : White (default)
1 : Red
2 : Green
3 : Yellow
4 : Blue
5 : Cyan
6 : Magenta
7 : White)"},
          },
          mdl::PointEntityDefinition{
            {{-4, -4, -4}, {4, 4, 4}},
            {},
            {},
          },
        },
      });
  }

  SECTION("parseBooleanProperty")
  {
    const auto file = R"(
<?xml version="1.0"?>
<classes>
  <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
    <boolean key="prop_true"  name="true"  value="true" />
    <boolean key="prop_false" name="false" value="false" />
    <boolean key="prop_True"  name="True"  value="true" />
    <boolean key="prop_False" name="False" value="false" />
    <boolean key="prop_0"     name="0"     value="0" />
    <boolean key="prop_1"     name="1"     value="1" />
    <boolean key="prop_2"     name="2"     value="2" />
    <boolean key="prop_n1"    name="-1"    value="-1" />
  </point>
</classes>
)";

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "_skybox",
          Color{0.77f, 0.88f, 1.0f, 1.0f},
          "",
          {
            {"prop_true", Boolean{true}, "true", ""},
            {"prop_false", Boolean{false}, "false", ""},
            {"prop_True", Boolean{true}, "True", ""},
            {"prop_False", Boolean{false}, "False", ""},
            {"prop_0", Boolean{false}, "0", ""},
            {"prop_1", Boolean{true}, "1", ""},
            {"prop_2", Boolean{true}, "2", ""},
            {"prop_n1", Boolean{true}, "-1", ""},
          },
          mdl::PointEntityDefinition{
            {{-4, -4, -4}, {4, 4, 4}},
            {},
            {},
          },
        },
      });
  }

  SECTION("parseInvalidRealPropertyDefinition")
  {
    const std::string file = R"(
<?xml version="1.0"?>
<classes>
    <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
        <real key="_scale" name="Scale" value="asdf" />
    </point>
</classes>
                        )";

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "_skybox",
          Color{0.77f, 0.88f, 1.0f, 1.0f},
          "",
          {
            {"_scale", Unknown{"asdf"}, "Scale", ""},
          },
          mdl::PointEntityDefinition{
            {{-4, -4, -4}, {4, 4, 4}},
            {},
            {},
          },
        },
      });
  }

  SECTION("parseLegacyModelDefinition")
  {
    const std::string file = R"(
<?xml version="1.0"?>
<classes>
<point name="ammo_bfg" color=".3 .3 1" box="-16 -16 -16 16 16 16" model="models/powerups/ammo/bfgam.md3" />
</classes>
            )";

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "ammo_bfg",
          Color{0.3f, 0.3f, 1.0f, 1.0f},
          "",
          {},
          mdl::PointEntityDefinition{
            {{-16, -16, -16}, {16, 16, 16}},
            mdl::ModelDefinition{el::lit(
              el::MapType{{"path", el::Value{"models/powerups/ammo/bfgam.md3"}}})},
            {},
          },
        },
      });
  }

  SECTION("parseELStaticModelDefinition")
  {
    const std::string file = R"(
            <?xml version="1.0"?>
            <classes>
            <point name="ammo_bfg" color=".3 .3 1" box="-16 -16 -16 16 16 16" model="{{ spawnflags == 1 -> 'models/powerups/ammo/bfgam.md3', 'models/powerups/ammo/bfgam2.md3' }}" />
            </classes>
            )";

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "ammo_bfg",
          Color{0.3f, 0.3f, 1.0f, 1.0f},
          "",
          {},
          mdl::PointEntityDefinition{
            {{-16, -16, -16}, {16, 16, 16}},
            mdl::ModelDefinition{el::swt({
              el::cs(
                el::eq(el::var("spawnflags"), el::lit(1)),
                el::lit("models/powerups/ammo/bfgam.md3")),
              el::lit("models/powerups/ammo/bfgam2.md3"),
            })},
            {},
          },
        },
      });
  }

  SECTION("parsePointEntityWithMissingBoxAttribute")
  {
    const auto file = R"(
<?xml version="1.0"?>
  <classes>
    <point name= "linkEmitter" color="0.2 0.5 0.2 ">
      <target key="target" name="target"></target>
    </point>
  </classes>
)";

    auto parser = EntParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "linkEmitter",
          Color{0.2f, 0.5f, 0.2f, 1.0f},
          "",
          {
            {
              "target",
              LinkSource{},
              "target",
              "",
            },
          },
          mdl::PointEntityDefinition{
            {{-8, -8, -8}, {8, 8, 8}},
            {},
            {},
          },
        },
      });
  }
}

} // namespace tb::io
