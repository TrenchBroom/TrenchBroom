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

#include "mdl/CompilationConfigParser.h"
#include "mdl/CompilationConfig.h"
#include "mdl/CompilationTask.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("CompilationConfigParser")
{
  SECTION("parseBlankConfig")
  {
    const auto config = "   ";
    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseEmptyConfig")
  {
    const auto config = "  {  } ";
    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseEmptyConfigWithTrailingGarbage")
  {
    const auto config = "  {  } asdf";
    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseMissingProfiles")
  {
    const auto config = "  { 'version' : 1 } ";
    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseMissingVersion")
  {
    const auto config = "  { 'profiles': {} } ";
    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseEmptyProfiles")
  {
    const auto config = "  { 'version': 1, 'profiles': [] } ";
    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse() == mdl::CompilationConfig{{}});
  }

  SECTION("parseOneProfileWithMissingNameAndMissingTasks")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {}
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithNameAndMissingTasks")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name': 'A profile'
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithMissingNameAndEmptyTasks")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'tasks': []
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithNameAndEmptyTasks")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile', 
      'workdir' : '', 
      'tasks' : []
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(
      parser.parse()
      == mdl::CompilationConfig{{
        {"A profile", "", {}},
      }});
  }

  SECTION("parseOneProfileWithNameAndOneInvalidTask")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ { 'asdf' : 'asdf' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithNameAndOneTaskWithUnknownType")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ { 'type' : 'unknown' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithNameAndOneCopyTaskWithMissingSource")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ {  'type' : 'copy', 'target' : 'somewhere' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithNameAndOneCopyTaskWithMissingTarget")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ {  'type' : 'copy', 'source' : 'somewhere' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION(
    ""
    "parseOneProfileWithNameAndOneDeleteTaskWithMissingTarget")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ {  'type' : 'delete', } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithNameAndOneCopyTask")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks' : [ { 'type' : 'copy', 'source' : 'the source', 'target' : 'the target' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(
      parser.parse()
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationCopyFiles{true, "the source", "the target"},
         }},
      }});
  }

  SECTION("parseOneProfileWithNameAndOneRenameTask")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks' : [ { 'type' : 'rename', 'source' : 'the source', 'target' : 'the target' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(
      parser.parse()
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationRenameFile{true, "the source", "the target"},
         }},
      }});
  }

  SECTION("parseOneProfileWithNameAndOneDeleteTask")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ {  'type' : 'delete', 'target' : 'the target' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(
      parser.parse()
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationDeleteFiles{true, "the target"},
         }},
      }});
  }

  SECTION("parseOneProfileWithNameAndOneToolTaskWithMissingTool")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ {  'type' : 'tool', 'parameters' : 'this and that' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION(
    ""
    "parseOneProfileWithNameAndOneToolTaskWithMissingParameters")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [
    {
      'name' : 'A profile',
      'workdir' : '',
      'tasks': [ {  'type' : 'tool', 'tool' : 'tyrbsp.exe' } ]
    }
  ]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(parser.parse().is_error());
  }

  SECTION("parseOneProfileWithNameAndOneToolTask")
  {
    const auto config = R"(
{
  'version': 1,
  'unexpectedKey': '',
  'profiles': [{
      'name' : 'A profile',
      'unexpectedKey' : '',
      'workdir' : '',
      'tasks' : [{ 
        'type' : 'tool',
        'unexpectedKey' : '',
        'tool' : 'tyrbsp.exe',
        'parameters': 'this and that'
      }]
    }]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(
      parser.parse()
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationRunTool{true, "tyrbsp.exe", "this and that", false},
         }},
      }});
  }

  SECTION("parseOneProfileWithNameAndFourTasks")
  {
    const auto config = R"(
{
  'version': 1,
  'profiles': [{
    'name': 'A profile',
    'workdir': '',
    'tasks': [{
      'type':'tool',
      'tool': 'tyrbsp.exe',
      'parameters': 'this and that',
      'treatNonZeroResultCodeAsError': true
    },
    {
      'type':'copy',
      'source': 'the source',
      'target': 'the target',
      'enabled': false
    },
    {
      'type':'rename',
      'source': 'the source',
      'target': 'the target',
      'enabled': true
    },
    {
      'type':'delete',
      'target': 'some other target',
      'enabled': false
    }]
  }]
})";

    auto parser = CompilationConfigParser{config};
    CHECK(
      parser.parse()
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationRunTool{true, "tyrbsp.exe", "this and that", true},
           mdl::CompilationCopyFiles{false, "the source", "the target"},
           mdl::CompilationRenameFile{true, "the source", "the target"},
           mdl::CompilationDeleteFiles{false, "some other target"},
         }},
      }});
  }

  SECTION("parseUnescapedBackslashes")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/1437
    const auto config = R"(
{
  "profiles": [{
    "name": "Full Compile",
    "tasks": [{
      "source": "${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp",
      "target": "C:\\quake2\\chaos\\maps\\",
      "type": "copy"
    }],
    "workdir": "${MAP_DIR_PATH}"
  }],
  "version": 1
})";

    auto parser = CompilationConfigParser{config};
    CHECK(
      parser.parse()
      == mdl::CompilationConfig{{
        {"Full Compile",
         "${MAP_DIR_PATH}",
         {
           mdl::CompilationCopyFiles{
             true, "${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp", R"(C:\quake2\chaos\maps\)"},
         }},
      }});
  }
}

} // namespace tb::mdl
