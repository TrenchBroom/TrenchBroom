/*
 Copyright (C) 2018 Eric Wasylishen

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

#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"

#include <memory>
#include <string>

#include "Catch2.h"

namespace TrenchBroom {
    namespace IO {
        static const char* buff() {
            static const auto* result = "abcdefghij_";
            return result;
        }

        static std::shared_ptr<File> file() {
            static auto result = Disk::openFile(Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Reader/10byte"));
            return result;
        }

        static void createEmpty(Reader&& r) {
            CHECK(r.size() == 0U);
            CHECK(r.position() == 0U);
            CHECK_NOTHROW(r.seekFromBegin(0U));
            CHECK_NOTHROW(r.seekFromEnd(0U));
            CHECK_NOTHROW(r.seekForward(0U));
            CHECK_FALSE(r.canRead(1U));
            CHECK(r.canRead(0U));
            CHECK(r.eof());
            CHECK_THROWS_AS(r.readChar<char>(), ReaderException);
        }

        TEST_CASE("BufferReaderTest.createEmpty", "[BufferReaderTest]") {
            createEmpty(Reader::from(buff(), buff()));
        }

        TEST_CASE("FileReaderTest.createEmpty", "[FileReaderTest]") {
            const auto emptyFile = Disk::openFile(Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Reader/empty"));
            createEmpty(emptyFile->reader());
        }

        static void createNonEmpty(Reader&& r) {
            CHECK(r.size() == 10U);
            CHECK(r.position() == 0U);
            CHECK(r.canRead(0U));
            CHECK(r.canRead(10U));
            CHECK_FALSE(r.canRead(11U));
            CHECK_FALSE(r.eof());

            // read a char
            CHECK(r.readChar<char>() == 'a');
            CHECK(r.position() == 1U);
            CHECK(r.canRead(1U));
            CHECK(r.canRead(9U));
            CHECK_FALSE(r.canRead(10U));

            // read remainder
            CHECK(r.readString(9) == std::string("bcdefghij"));
            CHECK(r.position() == 10U);
            CHECK_FALSE(r.canRead(1U));
            CHECK(r.canRead(0U));
            CHECK(r.eof());
            CHECK_THROWS_AS(r.readChar<char>(), ReaderException);
        }

        TEST_CASE("BufferReaderTest.createNonEmpty", "[BufferReaderTest]") {
            createNonEmpty(Reader::from(buff(), buff() + 10));
        }

        TEST_CASE("FileReaderTest.createNonEmpty", "[FileReaderTest]") {
            createNonEmpty(file()->reader());
        }

        static void seekFromBegin(Reader&& r) {
            r.seekFromBegin(0U);
            CHECK(r.position() == 0U);

            r.seekFromBegin(1U);
            CHECK(r.position() == 1U);

            r.seekFromBegin(2U);
            CHECK(r.position() == 2U);

            CHECK_THROWS_AS(r.seekFromBegin(11U), ReaderException);
            CHECK(r.position() == 2U);
        }

        TEST_CASE("BufferReaderTest.seekFromBegin", "[BufferReaderTest]") {
            seekFromBegin(Reader::from(buff(), buff() + 10));

        }

        TEST_CASE("FileReaderTest.seekFromBegin", "[FileReaderTest]") {
            seekFromBegin(file()->reader());
        }

        static void seekFromEnd(Reader&& r) {
            r.seekFromEnd(0U);
            CHECK(r.position() == 10U);

            r.seekFromEnd(1U);
            CHECK(r.position() == 9U);

            r.seekFromEnd(10U);
            CHECK(r.position() == 0U);

            CHECK_THROWS_AS(r.seekFromEnd(11U), ReaderException);
            CHECK(r.position() == 0U);
        }

        TEST_CASE("BufferReaderTest.seekFromEnd", "[BufferReaderTest]") {
            seekFromEnd(Reader::from(buff(), buff() + 10));
        }

        TEST_CASE("FileReaderTest.seekFromEnd", "[FileReaderTest]") {
            seekFromEnd(file()->reader());
        }

        static void seekForward(Reader&& r) {
            r.seekForward(1U);
            CHECK(r.position() == 1U);

            r.seekForward(1U);
            CHECK(r.position() == 2U);

            CHECK_THROWS_AS(r.seekForward(9U), ReaderException);
            CHECK(r.position() == 2U);
        }

        TEST_CASE("BufferReaderTest.seekForward", "[BufferReaderTest]") {
            seekForward(Reader::from(buff(), buff() + 10));
        }

        TEST_CASE("FileReaderTest.seekForward", "[FileReaderTest]") {
            seekForward(file()->reader());
        }

        static void subReader(Reader&& r) {
            auto s = r.subReaderFromBegin(5, 3);

            CHECK(s.size() == 3U);
            CHECK(s.position() == 0U);

            CHECK(s.readChar<char>() == 'f');
            CHECK(s.position() == 1U);

            CHECK(s.readChar<char>() == 'g');
            CHECK(s.position() == 2U);

            CHECK(s.readChar<char>() == 'h');
            CHECK(s.position() == 3U);

            CHECK_THROWS_AS(s.seekForward(1U), ReaderException);
            CHECK(s.position() == 3U);
        }

        TEST_CASE("BufferReaderTest.subReader", "[BufferReaderTest]") {
            subReader(Reader::from(buff(), buff() + 10));
        }

        TEST_CASE("FileReaderTest.subReader", "[FileReaderTest]") {
            subReader(file()->reader());
        }
    }
}
