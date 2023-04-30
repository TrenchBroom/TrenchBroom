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

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "Assets/PropertyDefinition.h"
#include "IO/DefParser.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/PathMatcher.h"
#include "IO/TestParserStatus.h"
#include "Model/EntityProperties.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("DefParserTest.parseIncludedDefFiles")
{
  const auto basePath = Disk::getCurrentWorkingDir() / Path("fixture/games/");
  const auto cfgFiles =
    Disk::findRecursively(basePath, makeExtensionPathMatcher({".def"}));

  for (const auto& path : cfgFiles)
  {
    CAPTURE(path);

    auto file = Disk::openFile(path);
    auto reader = file->reader().buffer();
    auto parser = DefParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}};

    auto status = TestParserStatus{};
    CHECK_NOTHROW(parser.parseDefinitions(status));

    /* Disabled because our files are full of previously undetected problems
    if (status.countStatus(LogLevel::Warn) > 0u) {
        UNSCOPED_INFO("Parsing DEF file " << path.asString() << " produced warnings");
        for (const auto& message : status.messages(LogLevel::Warn)) {
            UNSCOPED_INFO(message);
        }
        CHECK(status.countStatus(LogLevel::Warn) == 0u);
    }

    if (status.countStatus(LogLevel::Error) > 0u) {
        UNSCOPED_INFO("Parsing DEF file " << path.asString() << " produced errors");
        for (const auto& message : status.messages(LogLevel::Error)) {
            UNSCOPED_INFO(message);
        }
        CHECK(status.countStatus(LogLevel::Error) == 0u);
    }
    */
  }
}

TEST_CASE("DefParserTest.parseExtraDefFiles")
{
  const auto basePath = Disk::getCurrentWorkingDir() / Path("fixture/test/IO/Def");
  const auto cfgFiles =
    Disk::findRecursively(basePath, makeExtensionPathMatcher({".def"}));

  for (const Path& path : cfgFiles)
  {
    auto file = Disk::openFile(path);
    auto reader = file->reader().buffer();
    auto parser = DefParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}};

    auto status = TestParserStatus{};
    CHECK_NOTHROW(parser.parseDefinitions(status));
    CHECK(status.countStatus(LogLevel::Warn) == 0u);
    CHECK(status.countStatus(LogLevel::Error) == 0u);
  }
}

TEST_CASE("DefParserTest.parseEmptyFile")
{
  const auto file = R"()";
  auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("DefParserTest.parseWhitespaceFile")
{
  const auto file = R"(     
  	 
  )";

  auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("DefParserTest.parseCommentsFile")
{
  const auto file = R"(// asdfasdfasdf
//kj3k4jkdjfkjdf
)";

  auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("DefParserTest.parseSolidClass")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::BrushEntity);
  CHECK(definition->name() == "worldspawn");
  CHECK(definition->color() == Color{0.0f, 0.0f, 0.0f, 1.0f});
  CHECK(definition->description() == R"(Only used for the world entity. 
Set message to the level name. 
Set sounds to the cd track to play. 
"worldtype"	type of world)");

  const auto& properties = definition->propertyDefinitions();
  CHECK(properties.size() == 1u);

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("DefParserTest.parsePointClass")
{
  const auto file = R"(
    /*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush
    If crucified, stick the bounding box 12 pixels back into a wall to look right.
    */
)";

  auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == "monster_zombie");
  CHECK(definition->color() == Color{1.0f, 0.0f, 0.0f, 1.0f});
  CHECK(
    definition->description()
    == R"(If crucified, stick the bounding box 12 pixels back into a wall to look right.)");

  const auto* pointDefinition =
    static_cast<const Assets::PointEntityDefinition*>(definition);
  CHECK(
    pointDefinition->bounds() == vm::bbox3{{-16.0, -16.0, -24.0}, {16.0, 16.0, 32.0}});

  const auto& properties = definition->propertyDefinitions();
  CHECK(properties.size() == 1u); // spawnflags

  const auto property = properties[0];
  CHECK(property->type() == Assets::PropertyDefinitionType::FlagsProperty);

  const auto* spawnflags = definition->spawnflags();
  CHECK(spawnflags != nullptr);
  CHECK(spawnflags->defaultValue() == 0);

  CHECK(
    spawnflags->options()
    == std::vector<Assets::FlagsPropertyOption>{
      {1, "Crucified", "", false},
      {2, "ambush", "", false},
    });

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("DefParserTest.parseSpawnflagWithSkip")
{
  const auto file = R"(
    /*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) - SUSPENDED SPIN - RESPAWN
    some desc
    */)";

  auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == "item_health");
  CHECK(definition->color() == Color{0.3f, 0.3f, 1.0f, 1.0f});
  CHECK(definition->description() == "some desc");

  const auto* pointDefinition =
    static_cast<const Assets::PointEntityDefinition*>(definition);
  CHECK(
    pointDefinition->bounds() == vm::bbox3{{-16.0, -16.0, -16.0}, {16.0, 16.0, 16.0}});

  const auto& properties = definition->propertyDefinitions();
  CHECK(properties.size() == 1u); // spawnflags

  const auto property = properties[0];
  CHECK(property->type() == Assets::PropertyDefinitionType::FlagsProperty);

  const auto* spawnflags = definition->spawnflags();
  CHECK(spawnflags != nullptr);
  CHECK(spawnflags->defaultValue() == 0);

  CHECK(
    spawnflags->options()
    == std::vector<Assets::FlagsPropertyOption>{
      {1, "", "", false},
      {2, "SUSPENDED", "", false},
      {4, "SPIN", "", false},
      {8, "", "", false},
      {16, "RESPAWN", "", false},
    });

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("DefParserTest.parseBrushEntityWithMissingBBoxAndNoQuestionMark")
{
  const auto file = R"(
    /*QUAKED item_health (.3 .3 1) SUSPENDED SPIN - RESPAWN
    some desc
    */)";

  auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::BrushEntity);
  CHECK(definition->name() == "item_health");
  CHECK(definition->color() == Color{0.3f, 0.3f, 1.0f, 1.0f});
  CHECK(definition->description() == "some desc");

  const auto& properties = definition->propertyDefinitions();
  CHECK(properties.size() == 1u); // spawnflags

  const auto property = properties[0];
  CHECK(property->type() == Assets::PropertyDefinitionType::FlagsProperty);

  const auto* spawnflags = definition->spawnflags();
  CHECK(spawnflags != nullptr);
  CHECK(spawnflags->defaultValue() == 0);

  CHECK(
    spawnflags->options()
    == std::vector<Assets::FlagsPropertyOption>{
      {1, "SUSPENDED", "", false},
      {2, "SPIN", "", false},
      {4, "", "", false},
      {8, "RESPAWN", "", false},
    });

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("DefParserTest.parsePointClassWithBaseClasses")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == "light");

  CHECK(definition->propertyDefinitions().size() == 2u);

  const auto* stylePropertyDefinition = definition->propertyDefinition("style");
  CHECK(stylePropertyDefinition != nullptr);
  CHECK(stylePropertyDefinition->key() == "style");
  CHECK(
    stylePropertyDefinition->type() == Assets::PropertyDefinitionType::ChoiceProperty);

  const auto* spawnflagsPropertyDefinition =
    definition->propertyDefinition(Model::EntityPropertyKeys::Spawnflags);
  CHECK(spawnflagsPropertyDefinition != nullptr);
  CHECK(spawnflagsPropertyDefinition->key() == Model::EntityPropertyKeys::Spawnflags);
  CHECK(
    spawnflagsPropertyDefinition->type()
    == Assets::PropertyDefinitionType::FlagsProperty);

  const auto* choice =
    static_cast<const Assets::ChoicePropertyDefinition*>(stylePropertyDefinition);

  CHECK(
    choice->options()
    == std::vector<Assets::ChoicePropertyOption>{
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
    });

  kdl::vec_clear_and_delete(definitions);
}

static const auto DefModelDefinitionTemplate = R"(
  /*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush
  {
  model(${MODEL});
  }
  */)";

using Assets::assertModelDefinition;

TEST_CASE("DefParserTest.parseLegacyStaticModelDefinition")
{
  static const auto ModelDefinition =
    R"(":maps/b_shell0.bsp", ":maps/b_shell1.bsp" spawnflags = 1)";

  assertModelDefinition<DefParser>(
    Assets::ModelSpecification{IO::Path{"maps/b_shell0.bsp"}, 0, 0},
    ModelDefinition,
    DefModelDefinitionTemplate);
  assertModelDefinition<DefParser>(
    Assets::ModelSpecification{IO::Path{"maps/b_shell1.bsp"}, 0, 0},
    ModelDefinition,
    DefModelDefinitionTemplate,
    "{ 'spawnflags': 1 }");
}

TEST_CASE("DefParserTest.parseLegacyDynamicModelDefinition")
{
  static const auto ModelDefinition =
    R"(pathKey = "model" skinKey = "skin" frameKey = "frame")";

  assertModelDefinition<DefParser>(
    Assets::ModelSpecification{IO::Path{"maps/b_shell1.bsp"}, 0, 0},
    ModelDefinition,
    DefModelDefinitionTemplate,
    "{ 'model': 'maps/b_shell1.bsp' }");
  assertModelDefinition<DefParser>(
    Assets::ModelSpecification{IO::Path{"maps/b_shell1.bsp"}, 1, 2},
    ModelDefinition,
    DefModelDefinitionTemplate,
    "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
}

TEST_CASE("DefParserTest.parseELModelDefinition")
{
  static const std::string ModelDefinition =
    R"({{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }})";

  assertModelDefinition<DefParser>(
    Assets::ModelSpecification{IO::Path{"maps/b_shell0.bsp"}, 0, 0},
    ModelDefinition,
    DefModelDefinitionTemplate);
}

TEST_CASE("DefParserTest.parseInvalidBounds")
{
  const std::string file = R"(
    /*QUAKED light (0.0 1.0 0.0) (8 -8 -8) (-8 8 8) START_OFF
    {
    base("_light_style");
    }
    Non-displayed light.
    Default light value is 300
    If targeted, it will toggle between on or off.
    Default "style" is 0.
    */)";

  auto parser = DefParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto definition = static_cast<Assets::PointEntityDefinition*>(definitions[0]);
  CHECK(definition->bounds() == vm::bbox3d{8.0});

  kdl::vec_clear_and_delete(definitions);
}
} // namespace IO
} // namespace TrenchBroom
