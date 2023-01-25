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

#include "IO/CompilationConfigParser.h"
#include "Exceptions.h"
#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("CompilationConfigParserTest.parseBlankConfig")
{
  const auto config = "   ";
  auto parser = CompilationConfigParser{config};
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseEmptyConfig")
{
  const auto config = "  {  } ";
  auto parser = CompilationConfigParser{config};
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseEmptyConfigWithTrailingGarbage")
{
  const auto config = "  {  } asdf";
  auto parser = CompilationConfigParser{config};
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseMissingProfiles")
{
  const auto config = "  { 'version' : 1 } ";
  auto parser = CompilationConfigParser{config};
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseMissingVersion")
{
  const auto config = "  { 'profiles': {} } ";
  auto parser = CompilationConfigParser{config};
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseEmptyProfiles")
{
  const auto config = "  { 'version': 1, 'profiles': [] } ";
  auto parser = CompilationConfigParser{config};
  CHECK(parser.parse() == Model::CompilationConfig{{}});
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithMissingNameAndMissingTasks")
{
  const auto config = R"(
{
  'version': 1,
  'profiles': [
    {}
  ]
})";

  auto parser = CompilationConfigParser{config};
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndMissingTasks")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithMissingNameAndEmptyTasks")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndEmptyTasks")
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
    == Model::CompilationConfig{{
      {"A profile", "", {}},
    }});
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneInvalidTask")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneTaskWithUnknownType")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTaskWithMissingSource")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTaskWithMissingTarget")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneDeleteTaskWithMissingTarget")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTask")
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
    == Model::CompilationConfig{{
      {"A profile",
       "",
       {
         Model::CompilationCopyFiles{true, "the source", "the target"},
       }},
    }});
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneDeleteTask")
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
    == Model::CompilationConfig{{
      {"A profile",
       "",
       {
         Model::CompilationDeleteFiles{true, "the target"},
       }},
    }});
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneToolTaskWithMissingTool")
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest."
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
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneToolTask")
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
    == Model::CompilationConfig{{
      {"A profile",
       "",
       {
         Model::CompilationRunTool{true, "tyrbsp.exe", "this and that"},
       }},
    }});
}

TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndThreeTasks")
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
      'parameters': 'this and that'
    },
    {
      'type':'copy',
      'source': 'the source',
      'target': 'the target',
      'enabled': false
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
    == Model::CompilationConfig{{
      {"A profile",
       "",
       {
         Model::CompilationRunTool{true, "tyrbsp.exe", "this and that"},
         Model::CompilationCopyFiles{false, "the source", "the target"},
         Model::CompilationDeleteFiles{false, "some other target"},
       }},
    }});
}

TEST_CASE("CompilationConfigParserTest.parseUnescapedBackslashes")
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
    == Model::CompilationConfig{{
      {"Full Compile",
       "${MAP_DIR_PATH}",
       {
         Model::CompilationCopyFiles{
           true, "${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp", R"(C:\quake2\chaos\maps\)"},
       }},
    }});
}
} // namespace IO
} // namespace TrenchBroom
