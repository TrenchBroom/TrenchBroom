/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Macros.h"
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
                assertResult(deleteTestEnvironment());
            }
            
            inline const Path& dir() const {
                return m_dir;
            }
        private:
            void createTestEnvironment() {
                createDirectory(Path(""));
                createDirectory(Path("dir1"));
                createDirectory(Path("dir2"));
                createDirectory(Path("anotherDir"));
                createDirectory(Path("anotherDir/subDirTest"));

                createFile(Path("test.txt"), "some content");
                createFile(Path("test2.map"), "//test file\n{}");
                createFile(Path("anotherDir/subDirTest/test2.map"), "//sub dir test file\n{}");
                createFile(Path("anotherDir/test3.map"), "//yet another test file\n{}");
            }
            
            void createDirectory(const Path& path) {
                assertResult(::wxMkdir((m_dir + path).asString()));
            }
            
            void createFile(const Path& path, const wxString& contents) {
                wxFile file;
                assertResult(file.Create((m_dir + path).asString()));
                assertResult(file.Write(contents));
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
        
        TEST(DiskTest, resolvePath) {
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
            
            ASSERT_EQ(env.dir() + Path("test.txt"), Disk::resolvePath(rootPaths, paths[0]));
            ASSERT_EQ(env.dir() + Path("anotherDir/test3.map"), Disk::resolvePath(rootPaths, paths[1]));
            ASSERT_EQ(env.dir() + Path("anotherDir/subDirTest/test2.map"), Disk::resolvePath(rootPaths, paths[2]));
            ASSERT_EQ(Path(""), Disk::resolvePath(rootPaths, paths[3]));
            ASSERT_EQ(Path(""), Disk::resolvePath(rootPaths, paths[4]));
        }
        
        TEST(DiskFileSystemTest, createDiskFileSystem) {
            TestEnvironment env;
            
            ASSERT_THROW(DiskFileSystem(env.dir() + Path("asdf"), true), FileSystemException);
            ASSERT_NO_THROW(DiskFileSystem(env.dir() + Path("asdf"), false));
            ASSERT_NO_THROW(DiskFileSystem(env.dir(), true));
            
            // for case sensitive file systems
            ASSERT_NO_THROW(DiskFileSystem(env.dir() + Path("ANOTHERDIR"), true));
            
            const DiskFileSystem fs(env.dir() + Path("anotherDir/.."), true);
            ASSERT_EQ(env.dir(), fs.getPath());
        }
        
        TEST(DiskFileSystemTest, directoryExists) {
            TestEnvironment env;
            const DiskFileSystem fs(env.dir());

#if defined _WIN32
            ASSERT_THROW(fs.directoryExists(Path("c:\\")), FileSystemException);
#else
            ASSERT_THROW(fs.directoryExists(Path("/")), FileSystemException);
#endif
            ASSERT_THROW(fs.directoryExists(Path("..")), FileSystemException);
            
            ASSERT_TRUE(fs.directoryExists(Path(".")));
            ASSERT_TRUE(fs.directoryExists(Path("anotherDir")));
            ASSERT_TRUE(fs.directoryExists(Path("anotherDir/subDirTest")));
            ASSERT_TRUE(fs.directoryExists(Path("anotherDir/./subDirTest/..")));
            ASSERT_TRUE(fs.directoryExists(Path("ANOTHerDir")));
            ASSERT_FALSE(fs.directoryExists(Path("test.txt")));
            ASSERT_FALSE(fs.directoryExists(Path("fasdf")));
        }
        
        TEST(DiskFileSystemTest, fileExists) {
            TestEnvironment env;
            const DiskFileSystem fs(env.dir());
            
#if defined _WIN32
            ASSERT_THROW(fs.fileExists(Path("C:\\does_not_exist_i_hope.txt")), FileSystemException);
#else
            ASSERT_THROW(fs.fileExists(Path("/does_not_exist_i_hope.txt")), FileSystemException);
#endif
            ASSERT_THROW(fs.fileExists(Path("../test.txt")), FileSystemException);
            
            ASSERT_TRUE(fs.fileExists(Path("test.txt")));
            ASSERT_TRUE(fs.fileExists(Path("./test.txt")));
            ASSERT_TRUE(fs.fileExists(Path("anotherDir/test3.map")));
            ASSERT_TRUE(fs.fileExists(Path("anotherDir/./subDirTest/../subDirTest/test2.map")));
            ASSERT_TRUE(fs.fileExists(Path("ANOtherDir/test3.MAP")));
            ASSERT_FALSE(fs.fileExists(Path("anotherDir/whatever.txt")));
            ASSERT_FALSE(fs.fileExists(Path("fdfdf.blah")));
        }
        
        TEST(DiskFileSystemTest, findItems) {
            TestEnvironment env;
            const DiskFileSystem fs(env.dir());

#if defined _WIN32
            ASSERT_THROW(fs.findItems(Path("c:\\")), FileSystemException);
#else
            ASSERT_THROW(fs.findItems(Path("/")), FileSystemException);
#endif
            ASSERT_THROW(fs.findItems(Path("..")), FileSystemException);
            
            Path::List items = fs.findItems(Path("."));
            ASSERT_EQ(5u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./dir1")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./dir2")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./anotherDir")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./test.txt")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./test2.map")) != items.end());
            
            items = fs.findItems(Path(""), FileSystem::ExtensionMatcher("TXT"));
            ASSERT_EQ(1u, items.size());
            ASSERT_EQ(Path("test.txt"), items.front());

            items = fs.findItems(Path("anotherDir"));
            ASSERT_EQ(2u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("anotherDir/subDirTest")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("anotherDir/test3.map")) != items.end());
        }
        
        TEST(DiskFileSystemTest, findItemsRecursively) {
            TestEnvironment env;
            const DiskFileSystem fs(env.dir());
            
#if defined _WIN32
            ASSERT_THROW(fs.findItemsRecursively(Path("c:\\")), FileSystemException);
#else
            ASSERT_THROW(fs.findItemsRecursively(Path("/")), FileSystemException);
#endif
            ASSERT_THROW(fs.findItemsRecursively(Path("..")), FileSystemException);
            
            Path::List items = fs.findItemsRecursively(Path("."));
            ASSERT_EQ(8u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./dir1")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./dir2")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./anotherDir")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./anotherDir/test3.map")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./anotherDir/subDirTest")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./anotherDir/subDirTest/test2.map")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./test.txt")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("./test2.map")) != items.end());
            
            items = fs.findItemsRecursively(Path(""), FileSystem::ExtensionMatcher("MAP"));
            ASSERT_EQ(3u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("anotherDir/test3.map")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("anotherDir/subDirTest/test2.map")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("test2.map")) != items.end());

            items = fs.findItemsRecursively(Path("anotherDir"));
            ASSERT_EQ(3u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("anotherDir/test3.map")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("anotherDir/subDirTest")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("anotherDir/subDirTest/test2.map")) != items.end());
        }
        
        // getDirectoryContents gets tested thoroughly by the tests for the find* methods
        
        TEST(DiskFileSystemTest, openFile) {
            TestEnvironment env;
            const DiskFileSystem fs(env.dir());
            
#if defined _WIN32
            ASSERT_THROW(fs.openFile(Path("c:\\hopefully_nothing.here")), FileSystemException);
#else
            ASSERT_THROW(fs.openFile(Path("/hopefully_nothing.here")), FileSystemException);
#endif
            ASSERT_THROW(fs.openFile(Path("..")), FileSystemException);
            ASSERT_THROW(fs.openFile(Path(".")), FileSystemException);
            ASSERT_THROW(fs.openFile(Path("anotherDir")), FileSystemException);
            
            ASSERT_TRUE(fs.openFile(Path("test.txt")) != NULL);
            ASSERT_TRUE(fs.openFile(Path("anotherDir/test3.map")) != NULL);
            ASSERT_TRUE(fs.openFile(Path("anotherDir/../anotherDir/./test3.map")) != NULL);
        }
        
        TEST(WritableDiskFileSystemTest, createWritableDiskFileSystem) {
            TestEnvironment env;
            
            ASSERT_THROW(WritableDiskFileSystem(env.dir() + Path("asdf"), false), FileSystemException);
            ASSERT_NO_THROW(WritableDiskFileSystem(env.dir() + Path("asdf"), true));
            ASSERT_NO_THROW(WritableDiskFileSystem(env.dir(), true));
            
            // for case sensitive file systems
            ASSERT_NO_THROW(WritableDiskFileSystem(env.dir() + Path("ANOTHERDIR"), false));
            
            const WritableDiskFileSystem fs(env.dir() + Path("anotherDir/.."), false);
            ASSERT_EQ(env.dir(), fs.getPath());
        }
        
        TEST(WritableDiskFileSystemTest, createDirectory) {
            TestEnvironment env;
            WritableDiskFileSystem fs(env.dir(), false);
            
#if defined _WIN32
            ASSERT_THROW(fs.createDirectory(Path("c:\\hopefully_nothing_here")), FileSystemException);
#else
            ASSERT_THROW(fs.createDirectory(Path("/hopefully_nothing_here")), FileSystemException);
#endif
            ASSERT_THROW(fs.createDirectory(Path("")), FileSystemException);
            ASSERT_THROW(fs.createDirectory(Path(".")), FileSystemException);
            ASSERT_THROW(fs.createDirectory(Path("..")), FileSystemException);
            ASSERT_THROW(fs.createDirectory(Path("dir1")), FileSystemException);
            ASSERT_THROW(fs.createDirectory(Path("test.txt")), FileSystemException);
            
            fs.createDirectory(Path("newDir"));
            ASSERT_TRUE(fs.directoryExists(Path("newDir")));
            
            fs.createDirectory(Path("newDir/someOtherDir"));
            ASSERT_TRUE(fs.directoryExists(Path("newDir/someOtherDir")));
            
            fs.createDirectory(Path("newDir/someOtherDir/.././yetAnotherDir/."));
            ASSERT_TRUE(fs.directoryExists(Path("newDir/yetAnotherDir")));
        }
        
        TEST(WritableDiskFileSystemTest, deleteFile) {
            TestEnvironment env;
            WritableDiskFileSystem fs(env.dir(), false);
            
#if defined _WIN32
            ASSERT_THROW(fs.deleteFile(Path("c:\\hopefully_nothing_here.txt")), FileSystemException);
#else
            ASSERT_THROW(fs.deleteFile(Path("/hopefully_nothing_here.txt")), FileSystemException);
#endif
            ASSERT_THROW(fs.deleteFile(Path("")), FileSystemException);
            ASSERT_THROW(fs.deleteFile(Path(".")), FileSystemException);
            ASSERT_THROW(fs.deleteFile(Path("..")), FileSystemException);
            ASSERT_THROW(fs.deleteFile(Path("dir1")), FileSystemException);
            ASSERT_THROW(fs.deleteFile(Path("asdf.txt")), FileSystemException);
            ASSERT_THROW(fs.deleteFile(Path("/dir1/asdf.txt")), FileSystemException);

            fs.deleteFile(Path("test.txt"));
            ASSERT_FALSE(fs.fileExists(Path("test.txt")));
            
            fs.deleteFile(Path("anotherDir/test3.map"));
            ASSERT_FALSE(fs.fileExists(Path("anotherDir/test3.map")));
            
            fs.deleteFile(Path("anotherDir/subDirTest/.././subDirTest/./test2.map"));
            ASSERT_FALSE(fs.fileExists(Path("anotherDir/subDirTest/test2.map")));
        }
        
        TEST(WritableDiskFileSystemTest, moveFile) {
            TestEnvironment env;
            WritableDiskFileSystem fs(env.dir(), false);
            
#if defined _WIN32
            ASSERT_THROW(fs.moveFile(Path("c:\\hopefully_nothing_here.txt"),
                                     Path("dest.txt"), false), FileSystemException);
            ASSERT_THROW(fs.moveFile(Path("test.txt"),
                                     Path("C:\\dest.txt"), false), FileSystemException);
#else
            ASSERT_THROW(fs.moveFile(Path("/hopefully_nothing_here.txt"),
                                     Path("dest.txt"), false), FileSystemException);
            ASSERT_THROW(fs.moveFile(Path("test.txt"),
                                     Path("/dest.txt"), false), FileSystemException);
#endif

            ASSERT_THROW(fs.moveFile(Path("test.txt"),
                                     Path("test2.map"), false), FileSystemException);
            ASSERT_THROW(fs.moveFile(Path("test.txt"),
                                     Path("anotherDir/test3.map"), false), FileSystemException);
            ASSERT_THROW(fs.moveFile(Path("test.txt"),
                                     Path("anotherDir/../anotherDir/./test3.map"), false), FileSystemException);

            fs.moveFile(Path("test.txt"),
                        Path("test2.txt"), true);
            ASSERT_FALSE(fs.fileExists(Path("test.txt")));
            ASSERT_TRUE(fs.fileExists(Path("test2.txt")));
            
            fs.moveFile(Path("test2.txt"),
                        Path("test2.map"), true);
            ASSERT_FALSE(fs.fileExists(Path("test2.txt")));
            ASSERT_TRUE(fs.fileExists(Path("test2.map")));
            // we're trusting that the file is actually overwritten (should really test the contents here...)

            fs.moveFile(Path("test2.map"),
                        Path("dir1/test2.map"), true);
            ASSERT_FALSE(fs.fileExists(Path("test2.map")));
            ASSERT_TRUE(fs.fileExists(Path("dir1/test2.map")));
        }
    }
}
