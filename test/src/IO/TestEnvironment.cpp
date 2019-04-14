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

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

namespace TrenchBroom {
    namespace IO {
        TestEnvironment::TestEnvironment(const String& dir) :
            m_dir(Path(QDir::current().path().toStdString()) + Path(dir)) {
            createTestEnvironment();
        }

        TestEnvironment::~TestEnvironment() {
            assertResult(deleteTestEnvironment());
        }

        const Path& TestEnvironment::dir() const {
            return m_dir;
        }

        void TestEnvironment::createTestEnvironment() {
            deleteTestEnvironment();
            createDirectory(Path(""));
            doCreateTestEnvironment();
        }

        void TestEnvironment::createDirectory(const Path& path) {
            auto dir = QDir((m_dir + path).asQString());
            assertResult(dir.mkpath("."));
        }

        void TestEnvironment::createFile(const Path& path, const String& contents) {
            auto file = QFile((m_dir + path).asQString());
            assertResult(file.open(QIODevice::ReadWrite));

            auto stream = QTextStream(&file);
            stream << QString::fromStdString(contents);
            stream.flush();
            assert(stream.status() == QTextStream::Ok);
        }

        bool TestEnvironment::deleteDirectoryAbsolute(const Path& absolutePath) {
            auto dir = QDir(absolutePath.asQString());
            if (!dir.exists()) {
                return true;
            }

            return dir.removeRecursively();
        }

        bool TestEnvironment::deleteTestEnvironment() {
            return deleteDirectoryAbsolute(m_dir);
        }

        bool TestEnvironment::directoryExists(const Path& path) const {
            auto file = QFileInfo((m_dir + path).asQString());

            return file.exists() && file.isDir();
        }

        bool TestEnvironment::fileExists(const Path& path) const {
            auto file = QFileInfo((m_dir + path).asQString());

            return file.exists() && file.isFile();
        }

        void TestEnvironment::doCreateTestEnvironment() {}
    }
}

