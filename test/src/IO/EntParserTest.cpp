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

#include <gtest/gtest.h>

#include "CollectionUtils.h"
#include "TestUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "IO/DiskIO.h"
#include "IO/EntParser.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "Model/ModelTypes.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        TEST(EntParserTest, parseIncludedEntFiles) {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("data/games");
            const Path::List cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("ent"));

            for (const Path& path : cfgFiles) {
                MappedFile::Ptr file = Disk::openFile(path);
                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                EntParser parser(file->begin(), file->end(), defaultColor);

                TestParserStatus status;
                ASSERT_NO_THROW(parser.parseDefinitions(status)) << "Parsing ENT file " << path.asString() << " failed";
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Warn))
                                    << "Parsing FGD file " << path.asString() << " produced warnings";
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Error))
                                    << "Parsing FGD file " << path.asString() << " produced errors";
            }
        }

        TEST(EntParserTest, parseEmptyFile) {
            const String file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(EntParserTest, parseWhitespaceFile) {
            const String file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
    }
}
