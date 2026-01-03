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

#include "mdl/CatchConfig.h"
#include "mdl/CompilationConfig.h"
#include "mdl/CompilationTask.h"
#include "mdl/ParseCompilationConfig.h"

#include "kd/k.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("CompilationConfigParser")
{
  SECTION("parseBlankConfig")
  {
    const auto config = "   ";
    CHECK(parseCompilationConfig(config).is_error());
  }

  SECTION("parseEmptyConfig")
  {
    const auto config = "  {  } ";
    CHECK(parseCompilationConfig(config).is_error());
  }

  SECTION("parseEmptyConfigWithTrailingGarbage")
  {
    const auto config = "  {  } asdf";
    CHECK(parseCompilationConfig(config).is_error());
  }

  SECTION("parseMissingProfiles")
  {
    const auto config = "  { 'version' : 1 } ";
    CHECK(parseCompilationConfig(config).is_error());
  }

  SECTION("parseMissingVersion")
  {
    const auto config = "  { 'profiles': {} } ";
    CHECK(parseCompilationConfig(config).is_error());
  }

  SECTION("parseEmptyProfiles")
  {
    const auto config = "  { 'version': 1, 'profiles': [] } ";
    CHECK(parseCompilationConfig(config) == mdl::CompilationConfig{{}});
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(
      parseCompilationConfig(config)
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(
      parseCompilationConfig(config)
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationCopyFiles{K(enabled), "the source", "the target"},
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

    CHECK(
      parseCompilationConfig(config)
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationRenameFile{K(enabled), "the source", "the target"},
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

    CHECK(
      parseCompilationConfig(config)
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationDeleteFiles{K(enabled), "the target"},
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(parseCompilationConfig(config).is_error());
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

    CHECK(
      parseCompilationConfig(config)
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationRunTool{
             K(enabled),
             "tyrbsp.exe",
             "this and that",
             !K(treatNonZeroResultCodeAsError)},
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

    CHECK(
      parseCompilationConfig(config)
      == mdl::CompilationConfig{{
        {"A profile",
         "",
         {
           mdl::CompilationRunTool{
             K(enabled), "tyrbsp.exe", "this and that", K(treatNonZeroResultCodeAsError)},
           mdl::CompilationCopyFiles{!K(enabled), "the source", "the target"},
           mdl::CompilationRenameFile{K(enabled), "the source", "the target"},
           mdl::CompilationDeleteFiles{!K(enabled), "some other target"},
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

    CHECK(
      parseCompilationConfig(config)
      == mdl::CompilationConfig{{
        {"Full Compile",
         "${MAP_DIR_PATH}",
         {
           mdl::CompilationCopyFiles{
             K(enabled),
             "${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp",
             R"(C:\quake2\chaos\maps\)"},
         }},
      }});
  }
}

} // namespace tb::mdl
