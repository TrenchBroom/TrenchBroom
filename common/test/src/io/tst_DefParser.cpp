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

#include "io/DefParser.h"
#include "io/DiskIO.h"
#include "io/PathMatcher.h"
#include "io/TestParserStatus.h"
#include "io/TraversalMode.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionTestUtils.h"
#include "mdl/EntityProperties.h"
#include "mdl/PropertyDefinition.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::io
{
using namespace mdl::PropertyValueTypes;

TEST_CASE("DefParser")
{
  SECTION("parseIncludedDefFiles")
  {
    const auto basePath = std::filesystem::current_path() / "fixture/games/";
    const auto cfgFiles =
      Disk::find(basePath, TraversalMode::Flat, makeExtensionPathMatcher({".def"}))
      | kdl::value();

    for (const auto& path : cfgFiles)
    {
      CAPTURE(path);

      auto file = Disk::openFile(path) | kdl::value();
      auto reader = file->reader().buffer();
      auto parser = DefParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}};

      auto status = TestParserStatus{};
      CHECK(parser.parseDefinitions(status));

      /* Disabled because our files are full of previously undetected problems
      if (status.countStatus(LogLevel::Warn) > 0u) {
          UNSCOPED_INFO("Parsing DEF file " << path.string() << " produced warnings");
          for (const auto& message : status.messages(LogLevel::Warn)) {
              UNSCOPED_INFO(message);
          }
          CHECK(status.countStatus(LogLevel::Warn) == 0u);
      }

      if (status.countStatus(LogLevel::Error) > 0u) {
          UNSCOPED_INFO("Parsing DEF file " << path.string() << " produced errors");
          for (const auto& message : status.messages(LogLevel::Error)) {
              UNSCOPED_INFO(message);
          }
          CHECK(status.countStatus(LogLevel::Error) == 0u);
      }
      */
    }
  }

  SECTION("parseExtraDefFiles")
  {
    const auto basePath = std::filesystem::current_path() / "fixture/test/io/Def";
    const auto cfgFiles =
      Disk::find(basePath, TraversalMode::Recursive, makeExtensionPathMatcher({".def"}))
      | kdl::value();

    for (const auto& path : cfgFiles)
    {
      auto file = Disk::openFile(path) | kdl::value();
      auto reader = file->reader().buffer();
      auto parser = DefParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}};

      auto status = TestParserStatus{};
      CHECK(parser.parseDefinitions(status));
      CHECK(status.countStatus(LogLevel::Warn) == 0u);
      CHECK(status.countStatus(LogLevel::Error) == 0u);
    }
  }

  SECTION("parseEmptyFile")
  {
    const auto file = R"()";
    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseWhitespaceFile")
  {
    const auto file = R"(     
  	 
  )";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseCommentsFile")
  {
    const auto file = R"(// asdfasdfasdf
//kj3k4jkdjfkjdf
)";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseSolidClass")
  {
    const auto file = R"(
/*QUAKED worldspawn (0.0 0.0 0.0) ?
{
choice "worldtype"
  (
  (0,"medieval")
  (1,"metal")
  (2,"base")
  );
}
Only used for the world entity. 
Set message to the level name. 
Set sounds to the cd track to play. 
"worldtype"	type of world
*/
)";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "worldspawn",
          Color{0.0f, 0.0f, 0.0f, 1.0f},
          R"(Only used for the world entity. 
Set message to the level name. 
Set sounds to the cd track to play. 
"worldtype"	type of world)",
          {
            {
              "worldtype",
              Choice{
                {
                  {"0", "medieval"},
                  {"1", "metal"},
                  {"2", "base"},
                },
              },
              "",
              "",
            },
          },
        },
      });
  }

  SECTION("parsePointClass")
  {
    const auto file = R"(
    /*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush
    If crucified, stick the bounding box 12 pixels back into a wall to look right.
    */
)";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "monster_zombie",
          Color{1.0f, 0.0f, 0.0f, 1.0f},
          R"(If crucified, stick the bounding box 12 pixels back into a wall to look right.)",
          {
            {
              mdl::EntityPropertyKeys::Spawnflags,
              Flags{
                {
                  {1, "Crucified", ""},
                  {2, "ambush", ""},
                },
              },
              "",
              "",
            },
          },
          mdl::PointEntityDefinition{
            {{-16.0, -16.0, -24.0}, {16.0, 16.0, 32.0}},
            {},
            {},
          },
        },
      });
  }

  SECTION("parseSpawnflagWithSkip")
  {
    const auto file = R"(
    /*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) - SUSPENDED SPIN - RESPAWN
    some desc
    */)";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "item_health",
          Color{0.3f, 0.3f, 1.0f, 1.0f},
          "some desc",
          {
            {
              mdl::EntityPropertyKeys::Spawnflags,
              Flags{
                {
                  {1, "", ""},
                  {2, "SUSPENDED", ""},
                  {4, "SPIN", ""},
                  {8, "", ""},
                  {16, "RESPAWN", ""},
                },
              },
              "",
              "",
            },
          },
          mdl::PointEntityDefinition{
            {{-16.0, -16.0, -16.0}, {16.0, 16.0, 16.0}},
            {},
            {},
          },
        },
      });
  }

  SECTION("parseBrushEntityWithMissingBBoxAndNoQuestionMark")
  {
    const auto file = R"(
    /*QUAKED item_health (.3 .3 1) SUSPENDED SPIN - RESPAWN
    some desc
    */)";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "item_health",
          Color{0.3f, 0.3f, 1.0f, 1.0f},
          "some desc",
          {
            {
              mdl::EntityPropertyKeys::Spawnflags,
              Flags{
                {
                  {1, "SUSPENDED", ""},
                  {2, "SPIN", ""},
                  {4, "", ""},
                  {8, "RESPAWN", ""},
                },
              },
              "",
              "",
            },
          },
        },
      });
  }

  SECTION("parsePointClassWithBaseClasses")
  {
    const auto file = R"-(
    /*QUAKED _light_style
    {
    choice "style"
     (
      (0,"normal")
      (1,"flicker (first variety)")
      (2,"slow strong pulse")
      (3,"candle (first variety)")
      (4,"fast strobe")
      (5,"gentle pulse 1")
      (6,"flicker (second variety)")
      (7,"candle (second variety)")
      (8,"candle (third variety)")
      (9,"slow strobe (fourth variety)")
      (10,"fluorescent flicker")
      (11,"slow pulse not fade to black")
     );
}
    */
    
    /*QUAKED light (0.0 1.0 0.0) (-8 -8 -8) (8 8 8) START_OFF
    {
    base("_light_style");
    }
    Non-displayed light.
    Default light value is 300
    If targeted, it will toggle between on or off.
    Default "style" is 0.
    */)-";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "light",
          Color{0.0f, 1.0f, 0.0f, 1.0f},
          R"(Non-displayed light.
    Default light value is 300
    If targeted, it will toggle between on or off.
    Default "style" is 0.)",
          {
            {
              mdl::EntityPropertyKeys::Spawnflags,
              Flags{
                {
                  {1, "START_OFF", ""},
                },
              },
              "",
              "",
            },
            {"style",
             Choice{
               {
                 {"0", "normal"},
                 {"1", "flicker (first variety)"},
                 {"2", "slow strong pulse"},
                 {"3", "candle (first variety)"},
                 {"4", "fast strobe"},
                 {"5", "gentle pulse 1"},
                 {"6", "flicker (second variety)"},
                 {"7", "candle (second variety)"},
                 {"8", "candle (third variety)"},
                 {"9", "slow strobe (fourth variety)"},
                 {"10", "fluorescent flicker"},
                 {"11", "slow pulse not fade to black"},
               },
             },
             "",
             ""},
          },
          mdl::PointEntityDefinition{
            {{-8, -8, -8}, {8, 8, 8}},
            {},
            {},
          },

        },
      });
  }

  static const auto DefModelDefinitionTemplate = R"(
  /*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush
  {
  model(${MODEL});
  }
  */)";

  using mdl::getModelSpecification;

  SECTION("parseLegacyStaticModelDefinition")
  {
    static const auto ModelDefinition =
      R"(":maps/b_shell0.bsp", ":maps/b_shell1.bsp" spawnflags = 1)";

    CHECK(
      getModelSpecification<DefParser>(ModelDefinition, DefModelDefinitionTemplate)
      == mdl::ModelSpecification{"maps/b_shell0.bsp", 0, 0});
    CHECK(
      getModelSpecification<DefParser>(
        ModelDefinition, DefModelDefinitionTemplate, "{ 'spawnflags': 1 }")
      == mdl::ModelSpecification{"maps/b_shell1.bsp", 0, 0});
  }

  SECTION("parseLegacyDynamicModelDefinition")
  {
    static const auto ModelDefinition =
      R"(pathKey = "model" skinKey = "skin" frameKey = "frame")";

    CHECK(
      getModelSpecification<DefParser>(
        ModelDefinition, DefModelDefinitionTemplate, "{ 'model': 'maps/b_shell1.bsp' }")
      == mdl::ModelSpecification{"maps/b_shell1.bsp", 0, 0});
    CHECK(
      getModelSpecification<DefParser>(
        ModelDefinition,
        DefModelDefinitionTemplate,
        "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }")
      == mdl::ModelSpecification{"maps/b_shell1.bsp", 1, 2});
  }

  SECTION("parseELModelDefinition")
  {
    static const std::string ModelDefinition =
      R"({{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }})";

    CHECK(
      getModelSpecification<DefParser>(ModelDefinition, DefModelDefinitionTemplate)
      == mdl::ModelSpecification{"maps/b_shell0.bsp", 0, 0});
  }

  SECTION("parseInvalidBounds")
  {
    const std::string file = R"(
    /*QUAKED light (0.0 1.0 0.0) (8 -8 -8) (-8 8 8) START_OFF
    {
    }
    */)";

    auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {"light",
         Color{0.0f, 1.0f, 0.0f, 1.0f},
         "",
         {
           {
             mdl::EntityPropertyKeys::Spawnflags,
             Flags{
               {
                 {1, "START_OFF", ""},
               },
             },
             "",
             "",
           },
         },
         mdl::PointEntityDefinition{
           vm::bbox3d{8.0},
           {},
           {},
         }},
      });
  }
}

} // namespace tb::io
