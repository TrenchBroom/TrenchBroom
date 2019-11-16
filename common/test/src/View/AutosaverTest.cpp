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

#include "Logger.h"
#include "IO/Path.h"
#include "IO/TestEnvironment.h"
#include "Model/Brush.h"
#include "Model/Layer.h"
#include "View/Autosaver.h"
#include "View/MapDocumentTest.h"

#include <chrono>
#include <thread>

namespace TrenchBroom {
    namespace View {
        TEST(AutosaverTest, backupFileMatcher) {
            Autosaver::BackupFileMatcher matcher(IO::Path("test"));

            ASSERT_TRUE(matcher(IO::Path("test.1.map"), false));
            ASSERT_TRUE(matcher(IO::Path("test.2.map"), false));
            ASSERT_TRUE(matcher(IO::Path("test.20.map"), false));
            ASSERT_FALSE(matcher(IO::Path("dir"), true));
            ASSERT_FALSE(matcher(IO::Path("test.map"), false));
            ASSERT_FALSE(matcher(IO::Path("test.1-crash.map"), false));
            ASSERT_FALSE(matcher(IO::Path("test.2-crash.map"), false));
        }

        TEST_F(MapDocumentTest, autosaverNoSaveUntilSaveInterval) {
            IO::TestEnvironment env("autosaver_test");
            NullLogger logger;

            document->saveDocumentAs(env.dir() + IO::Path("test.map"));
            assert(env.fileExists(IO::Path("test.map")));

            Autosaver autosaver(document, 10, 0);

            // modify the map
            document->addNode(createBrush("some_texture"), document->currentLayer());

            autosaver.triggerAutosave(logger);

            ASSERT_FALSE(env.fileExists(IO::Path("autosave/test.1.map")));
            ASSERT_FALSE(env.directoryExists(IO::Path("autosave")));
        }

        TEST_F(MapDocumentTest, autosaverNoSaveOfUnchangedMap) {
            IO::TestEnvironment env("autosaver_test");
            NullLogger logger;

            document->saveDocumentAs(env.dir() + IO::Path("test.map"));
            assert(env.fileExists(IO::Path("test.map")));

            Autosaver autosaver(document, 0, 0);
            autosaver.triggerAutosave(logger);

            ASSERT_FALSE(env.fileExists(IO::Path("autosave/test.1.map")));
            ASSERT_FALSE(env.directoryExists(IO::Path("autosave")));
        }

        TEST_F(MapDocumentTest, autosaverSavesAfterSaveInterval) {
            IO::TestEnvironment env("autosaver_test");
            NullLogger logger;

            document->saveDocumentAs(env.dir() + IO::Path("test.map"));
            assert(env.fileExists(IO::Path("test.map")));

            Autosaver autosaver(document, 1, 0);

            // modify the map
            document->addNode(createBrush("some_texture"), document->currentLayer());

            // Wait for 2 seconds.
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);

            autosaver.triggerAutosave(logger);

            ASSERT_TRUE(env.fileExists(IO::Path("autosave/test.1.map")));
            ASSERT_TRUE(env.directoryExists(IO::Path("autosave")));
        }

        TEST_F(MapDocumentTest, autosaverNoSaveUntilIdleInterval) {
            IO::TestEnvironment env("autosaver_test");
            NullLogger logger;

            document->saveDocumentAs(env.dir() + IO::Path("test.map"));
            assert(env.fileExists(IO::Path("test.map")));

            Autosaver autosaver(document, 0, 1);

            // modify the map
            document->addNode(createBrush("some_texture"), document->currentLayer());

            autosaver.triggerAutosave(logger);

            ASSERT_FALSE(env.fileExists(IO::Path("autosave/test.1.map")));
            ASSERT_FALSE(env.directoryExists(IO::Path("autosave")));

        }

        TEST_F(MapDocumentTest, autosaverSavesAfterIdleInterval) {
            IO::TestEnvironment env("autosaver_test");
            NullLogger logger;

            document->saveDocumentAs(env.dir() + IO::Path("test.map"));
            assert(env.fileExists(IO::Path("test.map")));

            Autosaver autosaver(document, 0, 1);

            // modify the map
            document->addNode(createBrush("some_texture"), document->currentLayer());

            // Wait for 2 seconds.
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);

            autosaver.triggerAutosave(logger);

            ASSERT_TRUE(env.fileExists(IO::Path("autosave/test.1.map")));
            ASSERT_TRUE(env.directoryExists(IO::Path("autosave")));
        }

        TEST_F(MapDocumentTest, autosaverSavesAgainAfterSaveInterval) {
            IO::TestEnvironment env("autosaver_test");
            NullLogger logger;

            document->saveDocumentAs(env.dir() + IO::Path("test.map"));
            assert(env.fileExists(IO::Path("test.map")));

            Autosaver autosaver(document, 1, 0);

            // modify the map
            document->addNode(createBrush("some_texture"), document->currentLayer());

            // Wait for 2 seconds.
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);

            autosaver.triggerAutosave(logger);

            ASSERT_TRUE(env.fileExists(IO::Path("autosave/test.1.map")));
            ASSERT_TRUE(env.directoryExists(IO::Path("autosave")));

            // Wait for 2 seconds.
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);

            autosaver.triggerAutosave(logger);
            ASSERT_FALSE(env.fileExists(IO::Path("autosave/test.2.map")));

            // modify the map
            document->addNode(createBrush("some_texture"), document->currentLayer());

            autosaver.triggerAutosave(logger);
            ASSERT_TRUE(env.fileExists(IO::Path("autosave/test.2.map")));
        }

        TEST_F(MapDocumentTest, autosaverSavesWhenCrashFilesPresent) {
            // https://github.com/kduske/TrenchBroom/issues/2544

            IO::TestEnvironment env("autosaver_test");
            env.createDirectory(IO::Path("autosave"));
            env.createFile(IO::Path("autosave/test.1.map"), "some content");
            env.createFile(IO::Path("autosave/test.1-crash.map"), "some content again");

            NullLogger logger;

            document->saveDocumentAs(env.dir() + IO::Path("test.map"));
            assert(env.fileExists(IO::Path("test.map")));

            Autosaver autosaver(document, 0, 0);

            // modify the map
            document->addNode(createBrush("some_texture"), document->currentLayer());

            autosaver.triggerAutosave(logger);

            ASSERT_TRUE(env.fileExists(IO::Path("autosave/test.2.map")));
        }
    }
}
