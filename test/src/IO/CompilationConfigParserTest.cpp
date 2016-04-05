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
            const String config("  { version = \"1\" } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseMissingVersion) {
            const String config("  { profiles = {} } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseEmptyProfiles) {
            const String config("  { version = \"1\", profiles = {} } ");
            CompilationConfigParser parser(config);
            
            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(0u, result.profileCount());
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithMissingNameAndMissingTasks) {
            const String config("{"
                                "    version = \"1\","
                                "    profiles = {"
                                "        {}"
                                "    }"
                                "}");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndMissingTasks) {
            const String config("{"
                                "    version = \"1\","
                                "    profiles = {"
                                "        {"
                                "             name = \"A profile\""
                                "        }"
                                "    }"
                                "}");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithMissingNameAndEmptyTasks) {
            const String config("{"
                                "    version = \"1\","
                                "    profiles = {"
                                "        {"
                                "             tasks = {}"
                                "        }"
                                "    }"
                                "}");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndEmptyTasks) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {}\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            
            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(1u, result.profileCount());
            
            const Model::CompilationProfile& profile = result.profile(0);
            ASSERT_EQ(String("A profile"), profile.name());
            ASSERT_EQ(0u, profile.taskCount());
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneInvalidTask) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      asdf = \"asdf\""
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneTaskWithUnknownType) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"unknown\""
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneCopyTaskWithMissingSource) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"copy\",\n"
                                "                      target = \"somewhere\"\n"
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneCopyTaskWithMissingTarget) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"copy\",\n"
                                "                      source = \"somewhere\"\n"
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneCopyTask) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"copy\",\n"
                                "                      source = \"the source\",\n"
                                "                      target = \"the target\"\n"
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            
            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(1u, result.profileCount());
            
            const Model::CompilationProfile& profile = result.profile(0);
            ASSERT_EQ(String("A profile"), profile.name());
            ASSERT_EQ(1u, profile.taskCount());
            
            const Model::CompilationTask& task = profile.task(0);
            ASSERT_EQ(Model::CompilationTask::Type_Copy, task.type());
            
            const Model::CompilationCopyFiles& copyTask = static_cast<const Model::CompilationCopyFiles&>(task);
            ASSERT_EQ(String("the source"), copyTask.sourceSpec());
            ASSERT_EQ(String("the target"), copyTask.targetSpec());
        }

        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneToolTaskWithMissingTool) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"tool\",\n"
                                "                      parameters = \"this and that\"\n"
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneToolTaskWithMissingParameters) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"tool\",\n"
                                "                      tool = \"tyrbsp.exe\"\n"
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndOneToolTask) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"tool\",\n"
                                "                      tool = \"tyrbsp.exe\",\n"
                                "                      parameters = \"this and that\"\n"
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            
            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(1u, result.profileCount());
            
            const Model::CompilationProfile& profile = result.profile(0);
            ASSERT_EQ(String("A profile"), profile.name());
            ASSERT_EQ(1u, profile.taskCount());
            
            const Model::CompilationTask& task = profile.task(0);
            ASSERT_EQ(Model::CompilationTask::Type_Tool, task.type());
            
            const Model::CompilationRunTool& toolTask = static_cast<const Model::CompilationRunTool&>(task);
            ASSERT_EQ(String("tyrbsp.exe"), toolTask.toolSpec());
            ASSERT_EQ(String("this and that"), toolTask.parameterSpec());
        }
        
        TEST(CompilationConfigParserTest, parseOneProfileWithNameAndTwoTasks) {
            const String config("{\n"
                                "    version = \"1\",\n"
                                "    profiles = {\n"
                                "        {\n"
                                "             name = \"A profile\",\n"
                                "             tasks = {\n"
                                "                 {\n"
                                "                      type = \"tool\",\n"
                                "                      tool = \"tyrbsp.exe\",\n"
                                "                      parameters = \"this and that\"\n"
                                "                 },\n"
                                "                 {\n"
                                "                      type = \"copy\",\n"
                                "                      source = \"the source\",\n"
                                "                      target = \"the target\"\n"
                                "                 }\n"
                                "             }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            CompilationConfigParser parser(config);
            
            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(1u, result.profileCount());
            
            const Model::CompilationProfile& profile = result.profile(0);
            ASSERT_EQ(String("A profile"), profile.name());
            ASSERT_EQ(2u, profile.taskCount());
            
            const Model::CompilationTask& firstTask = profile.task(0);
            ASSERT_EQ(Model::CompilationTask::Type_Tool, firstTask.type());
            
            const Model::CompilationRunTool& toolTask = static_cast<const Model::CompilationRunTool&>(firstTask);
            ASSERT_EQ(String("tyrbsp.exe"), toolTask.toolSpec());
            ASSERT_EQ(String("this and that"), toolTask.parameterSpec());
        
            const Model::CompilationTask& secondTask = profile.task(1);
            ASSERT_EQ(Model::CompilationTask::Type_Copy, secondTask.type());
            
            const Model::CompilationCopyFiles& copyTask = static_cast<const Model::CompilationCopyFiles&>(secondTask);
            ASSERT_EQ(String("the source"), copyTask.sourceSpec());
            ASSERT_EQ(String("the target"), copyTask.targetSpec());
        }
    }
}
