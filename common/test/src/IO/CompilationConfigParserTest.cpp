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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Exceptions.h"
#include "IO/CompilationConfigParser.h"
#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include <string>

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("CompilationConfigParserTest.parseBlankConfig", "[CompilationConfigParserTest]") {
            const std::string config("   ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseEmptyConfig", "[CompilationConfigParserTest]") {
            const std::string config("  {  } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseEmptyConfigWithTrailingGarbage", "[CompilationConfigParserTest]") {
            const std::string config("  {  } asdf");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseMissingProfiles", "[CompilationConfigParserTest]") {
            const std::string config("  { 'version' : 1 } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseMissingVersion", "[CompilationConfigParserTest]") {
            const std::string config("  { 'profiles': {} } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseEmptyProfiles", "[CompilationConfigParserTest]") {
            const std::string config("  { 'version': 1, 'profiles': [] } ");
            CompilationConfigParser parser(config);

            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(0u, result.profileCount());
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithMissingNameAndMissingTasks", "[CompilationConfigParserTest]") {
            const std::string config("{"
                                "    'version': 1,"
                                "    'profiles': ["
                                "        {}"
                                "    ]"
                                "}");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndMissingTasks", "[CompilationConfigParserTest]") {
            const std::string config("{"
                                "    'version': 1,"
                                "    'profiles': ["
                                "        {"
                                "             'name': 'A profile'"
                                "        }"
                                "    ]"
                                "}");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithMissingNameAndEmptyTasks", "[CompilationConfigParserTest]") {
            const std::string config("{"
                                "    'version': 1,"
                                "    'profiles': ["
                                "        {"
                                "             'tasks': []"
                                "        }"
                                "    ]"
                                "}");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndEmptyTasks", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
            ASSERT_EQ(1u, result.profileCount());

            const Model::CompilationProfile* profile = result.profile(0);
            ASSERT_EQ(std::string("A profile"), profile->name());
            ASSERT_EQ(0u, profile->taskCount());
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneInvalidTask", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneTaskWithUnknownType", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTaskWithMissingSource", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTaskWithMissingTarget", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
            ASSERT_THROW(parser.parse(), ParserException);
        }

        class AssertCompilationCopyFilesVisitor : public Model::ConstCompilationTaskConstVisitor {
        private:
            const std::string& m_sourceSpec;
            const std::string& m_targetSpec;
        public:
            AssertCompilationCopyFilesVisitor(const std::string& sourceSpec, const std::string& targetSpec) :
            m_sourceSpec(sourceSpec),
            m_targetSpec(targetSpec) {}

            void visit(const Model::CompilationExportMap& /* task */) const override {
                ASSERT_TRUE(false);
            }

            void visit(const Model::CompilationExportObj& /* task */) const override {
                ASSERT_TRUE(false);
            }

            void visit(const Model::CompilationCopyFiles& task) const override {
                ASSERT_EQ(m_sourceSpec, task.sourceSpec());
                ASSERT_EQ(m_targetSpec, task.targetSpec());
            }

            void visit(const Model::CompilationRunTool& /* task */) const override {
                ASSERT_TRUE(false);
            }
        };

        class AssertCompilationRunToolVisitor : public Model::ConstCompilationTaskConstVisitor {
        private:
            const std::string& m_toolSpec;
            const std::string& m_parameterSpec;
        public:
            AssertCompilationRunToolVisitor(const std::string& toolSpec, const std::string& parameterSpec) :
            m_toolSpec(toolSpec),
            m_parameterSpec(parameterSpec) {}

            void visit(const Model::CompilationExportMap& /* task */) const override {
                ASSERT_TRUE(false);
            }

            void visit(const Model::CompilationExportObj& /* task */) const override {
                ASSERT_TRUE(false);
            }

            void visit(const Model::CompilationCopyFiles& /* task */) const override {
                ASSERT_TRUE(false);
            }

            void visit(const Model::CompilationRunTool& task) const override {
                ASSERT_EQ(m_toolSpec, task.toolSpec());
                ASSERT_EQ(m_parameterSpec, task.parameterSpec());
            }
        };

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneCopyTask", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
                                "    'version': 1,\n"
                                "    'profiles': [\n"
                                "        {\n"
                                "             'name': 'A profile',\n"
                                "             'workdir': '',\n"
                                "             'tasks': [\n"
                                "                 {\n"
                                "                      'type':'copy',\n"
                                "                      'source': 'the source',\n"
                                "                      'target': 'the target'\n"
                                "                 }\n"
                                "             ]\n"
                                "        }\n"
                                "    ]\n"
                                "}\n");
            CompilationConfigParser parser(config);

            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(1u, result.profileCount());

            const Model::CompilationProfile* profile = result.profile(0);
            ASSERT_EQ(std::string("A profile"), profile->name());
            ASSERT_EQ(1u, profile->taskCount());

            profile->task(0)->accept(AssertCompilationCopyFilesVisitor("the source", "the target"));
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneToolTaskWithMissingTool", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneToolTaskWithMissingParameters", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndOneToolTask", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
                                "                 }\n"
                                "             ]\n"
                                "        }\n"
                                "    ]\n"
                                "}\n");
            CompilationConfigParser parser(config);

            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(1u, result.profileCount());

            const Model::CompilationProfile* profile = result.profile(0);
            ASSERT_EQ(std::string("A profile"), profile->name());
            ASSERT_EQ(1u, profile->taskCount());

            profile->task(0)->accept(AssertCompilationRunToolVisitor("tyrbsp.exe", "this and that"));
        }

        TEST_CASE("CompilationConfigParserTest.parseOneProfileWithNameAndTwoTasks", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
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
                                "                      'target': 'the target'\n"
                                "                 }\n"
                                "             ]\n"
                                "        }\n"
                                "    ]\n"
                                "}\n");
            CompilationConfigParser parser(config);

            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(1u, result.profileCount());

            const Model::CompilationProfile* profile = result.profile(0);
            ASSERT_EQ(std::string("A profile"), profile->name());
            ASSERT_EQ(2u, profile->taskCount());

            profile->task(0)->accept(AssertCompilationRunToolVisitor("tyrbsp.exe", "this and that"));
            profile->task(1)->accept(AssertCompilationCopyFilesVisitor("the source", "the target"));
        }

        TEST_CASE("CompilationConfigParserTest.parseError_1437_unescaped_backslashes", "[CompilationConfigParserTest]") {
            const std::string config("{\n"
                                "	\"profiles\": [\n"
                                "		{\n"
                                "			\"name\": \"Full Compile\",\n"
                                "			\"tasks\": [\n"
                                "				{\n"
                                "					\"source\": \"${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp\",\n"
                                "					\"target\": \"C:\\\\quake2\\\\chaos\\\\maps\\\\\",\n" // The trailing backslash of the path has to be escaped.
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
            ASSERT_EQ(1u, result.profileCount());

            const Model::CompilationProfile* profile = result.profile(0);
            ASSERT_EQ(std::string("Full Compile"), profile->name());
            ASSERT_EQ(1u, profile->taskCount());

            profile->task(0)->accept(AssertCompilationCopyFilesVisitor("${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp", "C:\\quake2\\chaos\\maps\\"));
        }
    }
}
