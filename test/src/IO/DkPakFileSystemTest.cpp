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

#include "IO/DiskFileSystem.h"
#include "IO/FileMatcher.h"
#include "IO/DkPakFileSystem.h"
#include "IO/MappedFile.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace IO {
        TEST(DkPakFileSystemTest, directoryExists) {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("data/IO/Pak/dkpak_test.pak");
            const MappedFile::Ptr pakFile = Disk::openFile(pakPath);
            assert(pakFile != NULL);

            const DkPakFileSystem fs(pakPath, pakFile);
            ASSERT_THROW(fs.directoryExists(Path("/asdf")), FileSystemException);
            ASSERT_THROW(fs.directoryExists(Path("/pics")), FileSystemException);
            
            ASSERT_TRUE(fs.directoryExists(Path("pics")));
            ASSERT_TRUE(fs.directoryExists(Path("PICS")));
            ASSERT_FALSE(fs.directoryExists(Path("pics/tag1.pcx")));
        }
        
        TEST(DkPakFileSystemTest, fileExists) {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("data/IO/Pak/dkpak_test.pak");
            const MappedFile::Ptr pakFile = Disk::openFile(pakPath);
            assert(pakFile != NULL);
            
            const DkPakFileSystem fs(pakPath, pakFile);
            ASSERT_THROW(fs.fileExists(Path("/asdf.blah")), FileSystemException);
            ASSERT_THROW(fs.fileExists(Path("/pics/tag1.pcx")), FileSystemException);
            
            ASSERT_TRUE(fs.fileExists(Path("pics/tag1.pcx")));
            ASSERT_TRUE(fs.fileExists(Path("PICS/TAG1.pcX")));
        }
        
        TEST(DkPakFileSystemTest, findItems) {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("data/IO/Pak/dkpak_test.pak");
            const MappedFile::Ptr pakFile = Disk::openFile(pakPath);
            assert(pakFile != NULL);
            
            const DkPakFileSystem fs(pakPath, pakFile);
            ASSERT_THROW(fs.findItems(Path("/")), FileSystemException);
            ASSERT_THROW(fs.findItems(Path("/pics/")), FileSystemException);
            ASSERT_THROW(fs.findItems(Path("pics/tag1.pcx")), FileSystemException);
            
            Path::List items = fs.findItems(Path(""));
            ASSERT_EQ(4u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("pics")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("amnet.cfg")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("bear.cfg")) != items.end());
            
            items = fs.findItems(Path(""), FileExtensionMatcher("cfg"));
            ASSERT_EQ(2u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("amnet.cfg")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("bear.cfg")) != items.end());

            items = fs.findItems(Path("pics"), FileExtensionMatcher("cfg"));
            ASSERT_TRUE(items.empty());

            items = fs.findItems(Path("pics"));
            ASSERT_EQ(2u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("pics/tag1.pcx")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("pics/tag2.pcx")) != items.end());
        }
        
        TEST(DkPakFileSystemTest, findItemsRecursively) {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("data/IO/Pak/dkpak_test.pak");
            const MappedFile::Ptr pakFile = Disk::openFile(pakPath);
            assert(pakFile != NULL);
            
            const DkPakFileSystem fs(pakPath, pakFile);
            ASSERT_THROW(fs.findItemsRecursively(Path("/")), FileSystemException);
            ASSERT_THROW(fs.findItemsRecursively(Path("/pics/")), FileSystemException);
            ASSERT_THROW(fs.findItemsRecursively(Path("pics/tag1.pcx")), FileSystemException);
            
            Path::List items = fs.findItemsRecursively(Path(""));
            ASSERT_EQ(16u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("pics")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("pics/tag1.pcx")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("pics/tag2.pcx")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u1")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u1/box1_3.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u1/brlava.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/angle1_1.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/angle1_2.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/basic1_7.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u3")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u3/stairs1_3.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u3/stflr1_5.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("amnet.cfg")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("bear.cfg")) != items.end());
            
            items = fs.findItemsRecursively(Path(""), FileExtensionMatcher("wal"));
            ASSERT_EQ(7u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u1/box1_3.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u1/brlava.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/angle1_1.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/angle1_2.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/basic1_7.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u3/stairs1_3.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u3/stflr1_5.wal")) != items.end());
            
            items = fs.findItemsRecursively(Path("textures"), FileExtensionMatcher("WAL"));
            ASSERT_EQ(7u, items.size());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u1/box1_3.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u1/brlava.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/angle1_1.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/angle1_2.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u2/basic1_7.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u3/stairs1_3.wal")) != items.end());
            ASSERT_TRUE(std::find(items.begin(), items.end(), Path("textures/e1u3/stflr1_5.wal")) != items.end());
        }
        
        TEST(DkPakFileSystemTest, openFile) {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("data/IO/Pak/dkpak_test.pak");
            const MappedFile::Ptr pakFile = Disk::openFile(pakPath);
            assert(pakFile != NULL);
            
            const DkPakFileSystem fs(pakPath, pakFile);
            ASSERT_THROW(fs.openFile(Path("")), FileSystemException);
            ASSERT_THROW(fs.openFile(Path("/amnet.cfg")), FileSystemException);
            ASSERT_THROW(fs.openFile(Path("/textures")), FileSystemException);
            
            ASSERT_TRUE(fs.openFile(Path("amnet.cfg")) != NULL);
        }
    }
}
