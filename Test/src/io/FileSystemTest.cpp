/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Exceptions.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

#include <iostream>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/file.h>

namespace TrenchBroom {
    namespace IO {
        class TestEnvironment {
        private:
            String m_dir;
        public:
            TestEnvironment(const String& dir = "fstest") :
            m_dir(dir) {
                deleteTestEnvironment();
                createTestEnvironment();
            }
            
            ~TestEnvironment() {
                bool success = deleteTestEnvironment();
                assert(success);
            }
            
            inline const String& dir() const {
                return m_dir;
            }
        private:
            void createTestEnvironment() {
                bool success = wxMkdir(m_dir);
                assert(success);
                success = wxMkdir(m_dir + "/dir1");
                assert(success);
                success = wxMkdir(m_dir + "/dir2");
                assert(success);
                success = wxMkdir(m_dir + "/anotherDir");
                assert(success);
                
                success = wxFile().Create(m_dir + "/test.txt");
                assert(success);
                success = wxFile().Create(m_dir + "/test2.map");
                assert(success);
            }
            
            bool deleteDirectory(String path) {
#ifdef WIN32
                if (path[path.size() - 1] != '\\')
                    path += '\\';
#else
                if (path[path.size() - 1] != '/')
                    path += '/';
#endif
                if (!::wxDirExists(path))
                    return true;
                
                wxDir dir(path);
                assert(dir.IsOpened());
                
                wxString filename;
                if (dir.GetFirst(&filename)) {
                    do {
                        const String subPath = path + filename.ToStdString();
                        if (::wxDirExists(subPath))
                            deleteDirectory(subPath);
                        else
                            ::wxRemoveFile(subPath);
                    } while (dir.GetNext(&filename));
                }
                return ::wxRmdir(path);
            }
            
            bool deleteTestEnvironment() {
                return deleteDirectory(m_dir);
            }
        };
        
        TEST(FileSystemTest, DirectoryContents) {
            TestEnvironment env;
            FileSystem fs;
            StringList names;
            
            Path::List contents = fs.directoryContents(Path(env.dir()));
            for (size_t i = 0; i < contents.size(); i++)
                names.push_back(contents[i].asString());
            StringUtils::sortCaseSensitive(names);
            ASSERT_EQ(String("anotherDir"), names[0]);
            ASSERT_EQ(String("dir1"), names[1]);
            ASSERT_EQ(String("dir2"), names[2]);
            ASSERT_EQ(String("test.txt"), names[3]);
            ASSERT_EQ(String("test2.map"), names[4]);
            names.clear();
            
            contents = fs.directoryContents(Path(env.dir()), FileSystem::FSFiles);
            for (size_t i = 0; i < contents.size(); i++)
                names.push_back(contents[i].asString());
            StringUtils::sortCaseSensitive(names);
            ASSERT_EQ(String("test.txt"), names[0]);
            ASSERT_EQ(String("test2.map"), names[1]);
            names.clear();

            contents = fs.directoryContents(Path(env.dir()), FileSystem::FSDirectories);
            for (size_t i = 0; i < contents.size(); i++)
                names.push_back(contents[i].asString());
            StringUtils::sortCaseSensitive(names);
            ASSERT_EQ(String("anotherDir"), names[0]);
            ASSERT_EQ(String("dir1"), names[1]);
            ASSERT_EQ(String("dir2"), names[2]);
            names.clear();

            contents = fs.directoryContents(Path(env.dir()), FileSystem::FSBoth, "*.txt");
            for (size_t i = 0; i < contents.size(); i++)
                names.push_back(contents[i].asString());
            StringUtils::sortCaseSensitive(names);
            ASSERT_EQ(String("test.txt"), names[0]);
            names.clear();
        }
    }
}
