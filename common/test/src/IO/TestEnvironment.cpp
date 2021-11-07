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

#include "TestEnvironment.h"

#include "Macros.h"
#include "Uuid.h"
#include "IO/PathQt.h"

#include <string>

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

#include "Catch2.h"

namespace TrenchBroom {
    namespace IO {
        TestEnvironment::TestEnvironment(const std::string& dir, const SetupFunction& setup) :
        m_sandboxPath{pathFromQString(QDir::current().path()) + Path{generateUuid()}},
        m_dir{m_sandboxPath + Path{dir}} {
            createTestEnvironment(setup);
        }

        TestEnvironment::~TestEnvironment() {
            assertResult(deleteTestEnvironment());
        }

        const Path& TestEnvironment::dir() const {
            return m_dir;
        }

        void TestEnvironment::createTestEnvironment(const SetupFunction& setup) {
            deleteTestEnvironment();
            createDirectory(Path{});
            setup(*this);
        }

        void TestEnvironment::createDirectory(const Path& path) {
            const auto dir = QDir{IO::pathAsQString(m_dir + path)};
            assertResult(dir.mkpath("."));
        }

        void TestEnvironment::createFile(const Path& path, const std::string& contents) {
            auto file = QFile{IO::pathAsQString(m_dir + path)};
            assertResult(file.open(QIODevice::ReadWrite));

            auto stream = QTextStream{&file};
            stream << QString::fromStdString(contents);
            stream.flush();
            assert(stream.status() == QTextStream::Ok);
        }

        static bool deleteDirectoryAbsolute(const Path& absolutePath) {
            auto dir = QDir{IO::pathAsQString(absolutePath)};
            if (!dir.exists()) {
                return true;
            }

            return dir.removeRecursively();
        }

        bool TestEnvironment::deleteTestEnvironment() {
            return deleteDirectoryAbsolute(m_sandboxPath);
        }

        bool TestEnvironment::directoryExists(const Path& path) const {
            const auto file = QFileInfo{IO::pathAsQString(m_dir + path)};

            return file.exists() && file.isDir();
        }

        bool TestEnvironment::fileExists(const Path& path) const {
            const auto file = QFileInfo{IO::pathAsQString(m_dir + path)};

            return file.exists() && file.isFile();
        }
    }
}

