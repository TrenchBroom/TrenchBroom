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
TEST_CASE("CompilationConfigParserTest.parseBlankConfig", "[CompilationConfigParserTest]")
{
  const std::string config("   ");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("CompilationConfigParserTest.parseEmptyConfig", "[CompilationConfigParserTest]")
{
  const std::string config("  {  } ");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseEmptyConfigWithTrailingGarbage",
  "[CompilationConfigParserTest]")
{
  const std::string config("  {  } asdf");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseMissingProfiles", "[CompilationConfigParserTest]")
{
  const std::string config("  { 'version' : 1 } ");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseMissingVersion", "[CompilationConfigParserTest]")
{
  const std::string config("  { 'profiles': {} } ");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseEmptyProfiles", "[CompilationConfigParserTest]")
{
  const std::string config("  { 'version': 1, 'profiles': [] } ");
  CompilationConfigParser parser(config);

  Model::CompilationConfig result = parser.parse();
  CHECK(result.profileCount() == 0u);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithMissingNameAndMissingTasks",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{"
    "    'version': 1,"
    "    'profiles': ["
    "        {}"
    "    ]"
    "}");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndMissingTasks",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{"
    "    'version': 1,"
    "    'profiles': ["
    "        {"
    "             'name': 'A profile'"
    "        }"
    "    ]"
    "}");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithMissingNameAndEmptyTasks",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{"
    "    'version': 1,"
    "    'profiles': ["
    "        {"
    "             'tasks': []"
    "        }"
    "    ]"
    "}");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndEmptyTasks",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': []\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);

  Model::CompilationConfig result = parser.parse();
  CHECK(result.profileCount() == 1u);

  const Model::CompilationProfile* profile = result.profile(0);
  CHECK(profile->name() == std::string("A profile"));
  CHECK(profile->taskCount() == 0u);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneInvalidTask",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'asdf': 'asdf'"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneTaskWithUnknownType",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type': 'unknown'"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTaskWithMissingSource",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type':'copy',\n"
    "                      'target': 'somewhere'\n"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTaskWithMissingTarget",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type':'copy',\n"
    "                      'source': 'somewhere'\n"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

class AssertCompilationCopyFilesVisitor : public Model::ConstCompilationTaskConstVisitor
{
private:
  const bool& m_enabled;
  const bool& m_targetIsFileSpec;
  const std::string& m_sourceSpec;
  const std::string& m_targetSpec;

public:
  AssertCompilationCopyFilesVisitor(
    const bool& enabled,
    const bool& targetIsFileSpec,
    const std::string& sourceSpec,
    const std::string& targetSpec)
    : m_enabled(enabled)
    , m_targetIsFileSpec(targetIsFileSpec)
    , m_sourceSpec(sourceSpec)
    , m_targetSpec(targetSpec)
  {
  }

  void visit(const Model::CompilationExportMap& /* task */) const override
  {
    CHECK(false);
  }

  void visit(const Model::CompilationCopyFiles& task) const override
  {
    CHECK(task.targetIsFileSpec() == m_targetIsFileSpec);
    CHECK(task.sourceSpec() == m_sourceSpec);
    CHECK(task.targetSpec() == m_targetSpec);
    CHECK(m_enabled == task.enabled());
  }

  void visit(const Model::CompilationRunTool& /* task */) const override { CHECK(false); }
};

class AssertCompilationRunToolVisitor : public Model::ConstCompilationTaskConstVisitor
{
private:
  const std::string& m_toolSpec;
  const std::string& m_parameterSpec;

public:
  AssertCompilationRunToolVisitor(
    const std::string& toolSpec, const std::string& parameterSpec)
    : m_toolSpec(toolSpec)
    , m_parameterSpec(parameterSpec)
  {
  }

  void visit(const Model::CompilationExportMap& /* task */) const override
  {
    CHECK(false);
  }

  void visit(const Model::CompilationCopyFiles& /* task */) const override
  {
    CHECK(false);
  }

  void visit(const Model::CompilationRunTool& task) const override
  {
    CHECK(task.toolSpec() == m_toolSpec);
    CHECK(task.parameterSpec() == m_parameterSpec);
  }
};

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndCopyTasks",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type':'copy',\n"
    "                      'source': 'the source',\n"
    "                      'target': 'the target dir'\n"
    "                 },\n"
    "                 {\n"
    "                      'type':'copy',\n"
    "                      'targetIsFile': false,\n"
    "                      'source': 'the source',\n"
    "                      'target': 'another target dir'\n"
    "                 },\n"
    "                 {\n"
    "                      'type':'copy',\n"
    "                      'targetIsFile': true,\n"
    "                      'source': 'the source',\n"
    "                      'target': 'the target file'\n"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);

  Model::CompilationConfig result = parser.parse();
  CHECK(result.profileCount() == 1u);

  const Model::CompilationProfile* profile = result.profile(0);
  CHECK(profile->name() == std::string("A profile"));
  CHECK(profile->taskCount() == 3u);

  profile->task(0)->accept(
    AssertCompilationCopyFilesVisitor(true, false, "the source", "the target dir"));
  profile->task(1)->accept(
    AssertCompilationCopyFilesVisitor(true, false, "the source", "another target dir"));
  profile->task(2)->accept(
    AssertCompilationCopyFilesVisitor(true, true, "the source", "the target file"));
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneToolTaskWithMissingTool",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type':'tool',\n"
    "                      'parameters': 'this and that'\n"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest."
  "parseOneProfileWithNameAndOneToolTaskWithMissingParameters",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type':'tool',\n"
    "                      'tool': 'tyrbsp.exe'\n"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndOneToolTask",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'unexpectedKey': '',\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'unexpectedKey': '',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type':'tool',\n"
    "                      'unexpectedKey': '',\n"
    "                      'tool': 'tyrbsp.exe',\n"
    "                      'parameters': 'this and that'\n"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);

  Model::CompilationConfig result = parser.parse();
  CHECK(result.profileCount() == 1u);

  const Model::CompilationProfile* profile = result.profile(0);
  CHECK(profile->name() == std::string("A profile"));
  CHECK(profile->taskCount() == 1u);

  profile->task(0)->accept(
    AssertCompilationRunToolVisitor("tyrbsp.exe", "this and that"));
}

TEST_CASE(
  "CompilationConfigParserTest.parseOneProfileWithNameAndTwoTasks",
  "[CompilationConfigParserTest]")
{
  const std::string config(
    "{\n"
    "    'version': 1,\n"
    "    'profiles': [\n"
    "        {\n"
    "             'name': 'A profile',\n"
    "             'workdir': '',\n"
    "             'tasks': [\n"
    "                 {\n"
    "                      'type':'tool',\n"
    "                      'tool': 'tyrbsp.exe',\n"
    "                      'parameters': 'this and that'\n"
    "                 },\n"
    "                 {\n"
    "                      'type':'copy',\n"
    "                      'source': 'the source',\n"
    "                      'target': 'the target',\n"
    "                      'enabled': false\n"
    "                 }\n"
    "             ]\n"
    "        }\n"
    "    ]\n"
    "}\n");
  CompilationConfigParser parser(config);

  Model::CompilationConfig result = parser.parse();
  CHECK(result.profileCount() == 1u);

  const Model::CompilationProfile* profile = result.profile(0);
  CHECK(profile->name() == std::string("A profile"));
  CHECK(profile->taskCount() == 2u);

  profile->task(0)->accept(
    AssertCompilationRunToolVisitor("tyrbsp.exe", "this and that"));
  profile->task(1)->accept(
    AssertCompilationCopyFilesVisitor(false, false, "the source", "the target"));
}

TEST_CASE(
  "CompilationConfigParserTest.parseUnescapedBackslashes",
  "[CompilationConfigParserTest]")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/1437
  const std::string config(
    "{\n"
    "	\"profiles\": [\n"
    "		{\n"
    "			\"name\": \"Full Compile\",\n"
    "			\"tasks\": [\n"
    "				{\n"
    "					\"source\": "
    "\"${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp\",\n"
    "					\"target\": "
    "\"C:\\\\quake2\\\\chaos\\\\maps\\\\\",\n" // The trailing
                                               // backslash of the
                                               // path has to be
                                               // escaped.
    "					\"type\": \"copy\"\n"
    "				}\n"
    "			],\n"
    "			\"workdir\": \"${MAP_DIR_PATH}\"\n"
    "		}\n"
    "	],\n"
    "	\"version\": 1\n"
    "}\n");
  CompilationConfigParser parser(config);

  Model::CompilationConfig result = parser.parse();
  CHECK(result.profileCount() == 1u);

  const Model::CompilationProfile* profile = result.profile(0);
  CHECK(profile->name() == std::string("Full Compile"));
  CHECK(profile->taskCount() == 1u);

  profile->task(0)->accept(AssertCompilationCopyFilesVisitor(
    true, false, "${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp", "C:\\quake2\\chaos\\maps\\"));
}
} // namespace IO
} // namespace TrenchBroom
