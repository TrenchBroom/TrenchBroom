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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"

#include <algorithm>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filefn.h>

namespace TrenchBroom {
    namespace IO {
        class TestEnvironment {
        private:
            Path m_dir;
        public:
            TestEnvironment(const String& dir = "fstest") :
            m_dir(Path(::wxGetCwd().ToStdString()) + Path(dir)) {
                deleteTestEnvironment();
                createTestEnvironment();
            }
            
            ~TestEnvironment() {
                bool success = deleteTestEnvironment();
                assert(success);
            }
            
            inline const Path& dir() const {
                return m_dir;
            }
        private:
            void createTestEnvironment() {
                bool success = wxMkdir(m_dir.asString());
                assert(success);
                success = wxMkdir((m_dir + Path("dir1")).asString());
                assert(success);
                success = wxMkdir((m_dir + Path("dir2")).asString());
                assert(success);
                success = wxMkdir((m_dir + Path("anotherDir")).asString());
                assert(success);
                success = wxMkdir((m_dir + Path("anotherDir/subDirTest")).asString());
                assert(success);
                
                success = wxFile().Create((m_dir + Path("test.txt")).asString());
                assert(success);
                success = wxFile().Create((m_dir + Path("test2.map")).asString());
                assert(success);
                success = wxFile().Create((m_dir + Path("anotherDir/subDirTest/test2.map")).asString());
                assert(success);
                success = wxFile().Create((m_dir + Path("anotherDir/test3.map")).asString());
                assert(success);
            }
            
            bool deleteDirectory(const Path& path) {
                if (!::wxDirExists(path.asString()))
                    return true;
                
                { // put in a block so that dir gets closed before we call wxRmdir
                    wxDir dir(path.asString());
                    assert(dir.IsOpened());
                    
                    wxString filename;
                    if (dir.GetFirst(&filename)) {
                        do {
                            const Path subPath = path + Path(filename.ToStdString());
                            if (::wxDirExists(subPath.asString()))
                                deleteDirectory(subPath);
                            else
                                ::wxRemoveFile(subPath.asString());
                        } while (dir.GetNext(&filename));
                    }
                }
                return ::wxRmdir(path.asString());
            }
            
            bool deleteTestEnvironment() {
                return deleteDirectory(m_dir);
            }
        };
        
        TEST(DiskTest, fixPath) {
            TestEnvironment env;
            
            ASSERT_THROW(Disk::fixPath(Path("asdf/blah")), FileSystemException);
            ASSERT_THROW(Disk::fixPath(Path("/../../test")), FileSystemException);
            
            // on case sensitive file systems, this should also work
            ASSERT_TRUE(::wxFileExists(Disk::fixPath(env.dir() + Path("TEST.txt")).asString()));
            ASSERT_TRUE(::wxFileExists(Disk::fixPath(env.dir() + Path("anotHERDIR/./SUBdirTEST/../SubdirTesT/TesT2.MAP")).asString()));
        }
        
        TEST(DiskTest, directoryExists) {
            TestEnvironment env;
            
            ASSERT_THROW(Disk::directoryExists(Path("asdf/bleh")), FileSystemException);
            
            ASSERT_TRUE(Disk::directoryExists(env.dir() + Path("anotherDir")));
            ASSERT_TRUE(Disk::directoryExists(env.dir() + Path("anotherDir/subDirTest")));
        }
        
        TEST(DiskTest, fileExists) {
            TestEnvironment env;
            
            ASSERT_THROW(Disk::fileExists(Path("asdf/bleh")), FileSystemException);
            
            ASSERT_TRUE(Disk::fileExists(env.dir() + Path("test.txt")));
            ASSERT_TRUE(Disk::fileExists(env.dir() + Path("anotherDir/subDirTest/test2.map")));
        }
        
        TEST(DiskTest, getDirectoryContents) {
            TestEnvironment env;
            
            ASSERT_THROW(Disk::getDirectoryContents(Path("asdf/bleh")), FileSystemException);
            ASSERT_THROW(Disk::getDirectoryContents(env.dir() + Path("does/not/exist")), FileSystemException);
            
            const Path::List contents = Disk::getDirectoryContents(env.dir());
            ASSERT_EQ(5u, contents.size());
            ASSERT_TRUE(std::find(contents.begin(), contents.end(), Path("dir1")) != contents.end());
            ASSERT_TRUE(std::find(contents.begin(), contents.end(), Path("dir2")) != contents.end());
            ASSERT_TRUE(std::find(contents.begin(), contents.end(), Path("anotherDir")) != contents.end());
            ASSERT_TRUE(std::find(contents.begin(), contents.end(), Path("test.txt")) != contents.end());
            ASSERT_TRUE(std::find(contents.begin(), contents.end(), Path("test2.map")) != contents.end());
        }
        
        TEST(DiskTest, openFile) {
            TestEnvironment env;
            
            ASSERT_THROW(Disk::openFile(Path("asdf/bleh")), FileSystemException);
            ASSERT_THROW(Disk::openFile(env.dir() + Path("does/not/exist")), FileSystemException);
            
            ASSERT_THROW(Disk::openFile(env.dir() + Path("does_not_exist.txt")), FileSystemException);
            ASSERT_TRUE(Disk::openFile(env.dir() + Path("test.txt")) != NULL);
            ASSERT_TRUE(Disk::openFile(env.dir() + Path("anotherDir/subDirTest/test2.map")) != NULL);
        }
        
        TEST(DiskTest, resolvePaths) {
            TestEnvironment env;

            Path::List rootPaths;
            rootPaths.push_back(env.dir());
            rootPaths.push_back(env.dir() + Path("anotherDir"));
            
            Path::List paths;
            paths.push_back(Path("test.txt"));
            paths.push_back(Path("test3.map"));
            paths.push_back(Path("subDirTest/test2.map"));
            paths.push_back(Path("/asfd/blah"));
            paths.push_back(Path("adk3kdk/bhb"));
            
            Path::List found, notFound;
            
            Disk::resolvePaths(rootPaths, paths, found, notFound);
            ASSERT_EQ(3u, found.size());
            ASSERT_EQ(2u, notFound.size());
            
            ASSERT_EQ(env.dir() + Path("test.txt"), found[0]);
            ASSERT_EQ(env.dir() + Path("anotherDir/test3.map"), found[1]);
            ASSERT_EQ(env.dir() + Path("anotherDir/subDirTest/test2.map"), found[2]);
            ASSERT_EQ(paths[3], notFound[0]);
            ASSERT_EQ(paths[4], notFound[1]);
            
            rootPaths.push_back(Path("asdf"));
            ASSERT_THROW(Disk::resolvePaths(rootPaths, paths, found, notFound), FileSystemException);
        }
    }
}
