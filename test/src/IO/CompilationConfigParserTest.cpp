/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include <gtest/gtest.h>

#include "CollectionUtils.h"
#include "IO/CompilationConfigParser.h"
#include "IO/Path.h"
#include "Model/CompilationConfig.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        TEST(CompilationConfigParserTest, parseBlankConfig) {
            const String config("   ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST(CompilationConfigParserTest, parseEmptyConfig) {
            const String config("  {  } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseMissingProfiles) {
            const String config("  { 'version' : 1 } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseMissingVersion) {
            const String config("  { 'profiles': {} } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseEmptyProfiles) {
            const String config("  { 'version': 1, 'profiles': [] } ");
            CompilationConfigParser parser(config);
            
            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(0u, result.profileCount());
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithMissingNameAndMissingTasks) {
            const String config("{"
                                "    'version': 1,"
                                "    'profiles': ["
                                "        {}"
                                "    ]"
                                "}");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndMissingTasks) {
            const String config("{"
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
        
        TEST(CompilationConfigParserTest, parseOneProfileWithMissingNameAndEmptyTasks) {
            const String config("{"
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
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndEmptyTasks) {
            const String config("{\n"
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
            ASSERT_EQ(String("A profile"), profile->name());
            ASSERT_EQ(0u, profile->taskCount());
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneInvalidTask) {
            const String config("{\n"
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
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneTaskWithUnknownType) {
            const String config("{\n"
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
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneCopyTaskWithMissingSource) {
            const String config("{\n"
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
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneCopyTaskWithMissingTarget) {
            const String config("{\n"
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
            const String& m_sourceSpec;
            const String& m_targetSpec;
        public:
            AssertCompilationCopyFilesVisitor(const String& sourceSpec, const String& targetSpec) :
            m_sourceSpec(sourceSpec),
            m_targetSpec(targetSpec) {}

            void visit(const Model::CompilationExportMap* task) const {
                ASSERT_TRUE(false);
            }
            
            void visit(const Model::CompilationCopyFiles* task) const {
                ASSERT_EQ(m_sourceSpec, task->sourceSpec());
                ASSERT_EQ(m_targetSpec, task->targetSpec());
            }
            
            void visit(const Model::CompilationRunTool* task) const {
                ASSERT_TRUE(false);
            }
        };
        
        class AssertCompilationRunToolVisitor : public Model::ConstCompilationTaskConstVisitor {
        private:
            const String& m_toolSpec;
            const String& m_parameterSpec;
        public:
            AssertCompilationRunToolVisitor(const String& toolSpec, const String& parameterSpec) :
            m_toolSpec(toolSpec),
            m_parameterSpec(parameterSpec) {}
            
            void visit(const Model::CompilationExportMap* task) const {
                ASSERT_TRUE(false);
            }
            
            void visit(const Model::CompilationCopyFiles* task) const {
                ASSERT_TRUE(false);
            }
            
            void visit(const Model::CompilationRunTool* task) const {
                ASSERT_EQ(m_toolSpec, task->toolSpec());
                ASSERT_EQ(m_parameterSpec, task->parameterSpec());
            }
        };
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneCopyTask) {
            const String config("{\n"
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
            ASSERT_EQ(String("A profile"), profile->name());
            ASSERT_EQ(1u, profile->taskCount());
            
            profile->task(0)->accept(AssertCompilationCopyFilesVisitor("the source", "the target"));
        }

        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneToolTaskWithMissingTool) {
            const String config("{\n"
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
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneToolTaskWithMissingParameters) {
            const String config("{\n"
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
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneToolTask) {
            const String config("{\n"
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
            ASSERT_EQ(String("A profile"), profile->name());
            ASSERT_EQ(1u, profile->taskCount());
            
            profile->task(0)->accept(AssertCompilationRunToolVisitor("tyrbsp.exe", "this and that"));
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndTwoTasks) {
            const String config("{\n"
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
            ASSERT_EQ(String("A profile"), profile->name());
            ASSERT_EQ(2u, profile->taskCount());
            
            profile->task(0)->accept(AssertCompilationRunToolVisitor("tyrbsp.exe", "this and that"));
            profile->task(1)->accept(AssertCompilationCopyFilesVisitor("the source", "the target"));
        }
        
        TEST(CompilationConfigParserTest, parseError_1437_unescaped_backslashes) {
            const String config("{\n"
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
            ASSERT_EQ(String("Full Compile"), profile->name());
            ASSERT_EQ(1u, profile->taskCount());
            
            profile->task(0)->accept(AssertCompilationCopyFilesVisitor("${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp", "C:\\quake2\\chaos\\maps\\"));
        }
    }
}
