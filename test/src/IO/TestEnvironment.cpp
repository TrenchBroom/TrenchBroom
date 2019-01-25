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

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filefn.h>

namespace TrenchBroom {
    namespace IO {
        TestEnvironment::TestEnvironment(const String& dir) :
            m_dir(Path(::wxGetCwd().ToStdString()) + Path(dir)) {
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
            assertResult(::wxMkdir((m_dir + path).asString()));
        }

        void TestEnvironment::createFile(const Path& path, const String& contents) {
            wxFile file;
            assertResult(file.Create((m_dir + path).asString()));
            assertResult(file.Write(QString(contents)));
        }

        bool TestEnvironment::deleteDirectory(const Path& path) {
            if (!::wxDirExists(path.asString())) {
                return true;
            }

            { // put in a block so that dir gets closed before we call wxRmdir
                wxDir dir(path.asString());
                assert(dir.IsOpened());

                QString filename;
                if (dir.GetFirst(&filename)) {
                    do {
                        const Path subPath = path + Path(filename.ToStdString());
                        if (::wxDirExists(subPath.asString())) {
                            deleteDirectory(subPath);
                        } else {
                            ::wxRemoveFile(subPath.asString());
                        }
                    } while (dir.GetNext(&filename));
                }
            }
            return ::wxRmdir(path.asString());
        }

        bool TestEnvironment::deleteTestEnvironment() {
            return deleteDirectory(m_dir);
        }

        bool TestEnvironment::directoryExists(const Path& path) const {
            return ::wxDirExists((m_dir + path).asString());
        }

        bool TestEnvironment::fileExists(const Path& path) const {
            return ::wxFileExists((m_dir + path).asString());
        }

        void TestEnvironment::doCreateTestEnvironment() {}
    }
}

