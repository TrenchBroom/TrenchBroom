/*
Copyright (C) 2020 Eric Wasylishen

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

#include <catch2/catch.hpp>

#include "View/TextOutputAdapter.h"

#include <QTextEdit>

namespace TrenchBroom {
    namespace View {
        TEST_CASE("TextOutputAdapterTest.test", "[TextOutputAdapterTest]") {
            QTextEdit textEdit;
            TextOutputAdapter adapter(&textEdit);

            SECTION("string literal") {
                adapter << "abc";
                CHECK(textEdit.toPlainText() == "abc");
            }
            SECTION("trailing CR LF") {
                adapter << "abc\r\n";
                CHECK(textEdit.toPlainText() == "abc\n");
            }
            SECTION("CR LF") {
                adapter << "abc\r\ndef";
                CHECK(textEdit.toPlainText() == "abc\ndef");
            }
            SECTION("two CR LF") {
                adapter << "abc\r\n\r\ndef";
                CHECK(textEdit.toPlainText() == "abc\n\ndef");
            }

            // CR tests
            SECTION("CR then CR LF mid line") {
                adapter << "abc\rA\r\nline 2";
                CHECK(textEdit.toPlainText() == "Abc\nline 2");
            }
            SECTION("several CR's") {
                adapter << "abc\rAB\ra\r\nline 2";
                CHECK(textEdit.toPlainText() == "aBc\nline 2");
            }
            SECTION("CR then CR LF") {
                adapter << "abc\rABC\r\nline 2";
                CHECK(textEdit.toPlainText() == "ABC\nline 2");
            }
            SECTION("CR then LF") {
                adapter << "abc\rABC\nline 2";
                CHECK(textEdit.toPlainText() == "ABC\nline 2");
            }
        }
    }
}
