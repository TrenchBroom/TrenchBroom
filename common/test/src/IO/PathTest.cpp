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

#include "Exceptions.h"
#include "IO/Path.h"
#include "IO/PathQt.h"

#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
#ifdef _WIN32
        TEST_CASE("PathTest.constructWithString", "[PathTest]") {
            ASSERT_EQ(std::string(""), Path("").asString());
            ASSERT_EQ(std::string(""), Path(" ").asString());
            ASSERT_EQ(std::string("c:"), Path("c:\\").asString());
            ASSERT_EQ(std::string("c:\\asdf"), Path("c:\\asdf").asString());
            ASSERT_EQ(std::string("c:\\asdf"), Path("c:\\asdf\\").asString());
            ASSERT_EQ(std::string("c:\\asdf\\df"), Path("c:\\asdf\\df").asString());
            ASSERT_EQ(std::string("hey"), Path("hey").asString());
            ASSERT_EQ(std::string("hey"), Path("hey\\").asString());
            ASSERT_EQ(std::string("hey\\asdf"), Path("hey\\asdf").asString());
            ASSERT_EQ(std::string(".\\asdf"), Path(".\\asdf").asString());
        }

        TEST_CASE("PathTest.concatenate", "[PathTest]") {
            ASSERT_THROW(Path("") + Path("c:\\"), PathException);
            ASSERT_THROW(Path("") + Path("c:\\asdf"), PathException);
            ASSERT_THROW(Path("asdf") + Path("c:\\asdf"), PathException);
            ASSERT_THROW(Path("c:\\asdf") + Path("c:\\asdf"), PathException);
            ASSERT_EQ(Path(""), Path("") + Path(""));
            ASSERT_EQ(Path("c:\\"), Path("c:\\") + Path(""));
            ASSERT_EQ(Path("c:\\asdf"), Path("c:\\asdf") + Path(""));
            ASSERT_EQ(Path("c:\\asdf"), Path("c:\\") + Path("asdf"));
            ASSERT_EQ(Path("c:\\asdf\\hey"), Path("c:\\asdf") + Path("hey"));
            ASSERT_EQ(Path("asdf\\hey"), Path("asdf") + Path("hey"));
        }

        TEST_CASE("PathTest.isEmpty", "[PathTest]") {
            ASSERT_TRUE(Path("").isEmpty());
            ASSERT_FALSE(Path("asdf").isEmpty());
            ASSERT_FALSE(Path("c:").isEmpty());
            ASSERT_FALSE(Path("c:\\asdf").isEmpty());
            ASSERT_FALSE(Path(".").isEmpty());
            ASSERT_FALSE(Path("c:\\.").isEmpty());
        }

        TEST_CASE("PathTest.getLastComponent", "[PathTest]") {
            ASSERT_THROW(Path("").lastComponent().asString(), PathException);
            ASSERT_EQ("asdf", Path("c:\\asdf").lastComponent().asString());
            ASSERT_EQ(Path("asdf"), Path("asdf").lastComponent());
            ASSERT_EQ(Path("path.map"), Path("c:\\this\\is\\a\\path.map").lastComponent());
            ASSERT_EQ(Path(""), Path("/").lastComponent());
        }

        TEST_CASE("PathTest.deleteLastComponent", "[PathTest]") {
            ASSERT_THROW(Path("").deleteLastComponent(), PathException);
            ASSERT_EQ(Path("c:\\"), Path("c:\\asdf").deleteLastComponent());
            ASSERT_EQ(Path(""), Path("asdf").deleteLastComponent());
            ASSERT_EQ(Path("c:\\this\\is\\a"), Path("c:\\this\\is\\a\\path.map").deleteLastComponent());
        }

        TEST_CASE("PathTest.getFirstComponent", "[PathTest]") {
            ASSERT_THROW(Path("").firstComponent(), PathException);
            ASSERT_EQ("\\", Path("/asdf").firstComponent().asString());
            ASSERT_EQ("c:", Path("c:\\asdf\\blah").firstComponent().asString());
            ASSERT_EQ("asdf", Path("asdf\\bbab").firstComponent().asString());
        }

        TEST_CASE("PathTest.deleteFirstComponent", "[PathTest]") {
            ASSERT_THROW(Path("").deleteFirstComponent(), PathException);
            ASSERT_EQ(Path(""), Path("\\").deleteFirstComponent());
            ASSERT_EQ(Path("asdf"), Path("\\asdf").deleteFirstComponent());
            ASSERT_EQ(Path(""), Path("c:\\").deleteFirstComponent());
            ASSERT_EQ(Path("asdf"), Path("c:\\asdf").deleteFirstComponent());
            ASSERT_EQ(Path("asdf"), Path("/asdf").deleteFirstComponent());
            ASSERT_EQ(Path("blah"), Path("asdf/blah").deleteFirstComponent());
        }

        TEST_CASE("PathTest.subPath", "[PathTest]") {
            ASSERT_EQ(Path(""), Path("").subPath(0, 0));
            ASSERT_THROW(Path("test\\blah").subPath(1, 2), PathException);
            ASSERT_EQ(Path("test\\blah"), Path("test\\blah").subPath(0, 2));
            ASSERT_EQ(Path("test"), Path("test\\blah").subPath(0, 1));
            ASSERT_EQ(Path("c:\\test\\blah"), Path("c:\\test\\blah").subPath(0, 3));
            ASSERT_EQ(Path("c:\\test"), Path("c:\\test\\blah").subPath(0, 2));
            ASSERT_EQ(Path("blah"), Path("test\\blah").subPath(1, 1));
        }

        TEST_CASE("PathTest.getExtension", "[PathTest]") {
            ASSERT_THROW(Path("").extension(), PathException);
            ASSERT_EQ(std::string(""), Path("asdf").extension());
            ASSERT_EQ(std::string("map"), Path("asdf.map").extension());
            ASSERT_EQ(std::string("map"), Path("c:\\this\\is\\a\\path.map").extension());
            ASSERT_EQ(std::string("textfile"), Path("c:\\this\\is\\a\\path.map.textfile").extension());
            ASSERT_EQ(std::string(""), Path("c:\\").extension());
        }

        TEST_CASE("PathTest.addExtension", "[PathTest]") {
            const auto test = Path("c:\\").addExtension("map").asString();
            const auto test2 = test;
            const auto test3 = test + test2;

            ASSERT_THROW(Path("").addExtension("map"), PathException);
            ASSERT_EQ(Path("c:\\asdf."), Path("c:\\asdf").addExtension(""));
            ASSERT_EQ(Path("c:\\asdf.map"), Path("c:\\asdf").addExtension("map"));
            ASSERT_EQ(Path("c:\\asdf.map.test"), Path("c:\\asdf.map").addExtension("test"));
            ASSERT_EQ(Path("c:\\.map"), Path("c:\\").addExtension("map"));

        }

        TEST_CASE("PathTest.makeAbsolute", "[PathTest]") {
            ASSERT_THROW(Path("c:\\asdf").makeAbsolute(Path("c:\\hello")), PathException);
            ASSERT_THROW(Path("asdf").makeAbsolute(Path("hello")), PathException);
            ASSERT_EQ(Path("c:\\asdf\\hello"), Path("c:\\asdf").makeAbsolute(Path("hello")));
        }

        TEST_CASE("PathTest.makeRelative", "[PathTest]") {
            ASSERT_THROW(Path("").makeRelative(), PathException);
            ASSERT_THROW(Path("models\\barrel\\skin.tga").makeRelative(), PathException);
            ASSERT_EQ(Path(""), Path("C:").makeRelative());
            ASSERT_EQ(Path(""), Path("C:\\").makeRelative());
            ASSERT_EQ(Path("models\\barrel\\skin.tga"), Path("C:\\models\\barrel\\skin.tga").makeRelative());
        }

        TEST_CASE("PathTest.makeRelativeWithAbsolutePath", "[PathTest]") {
            ASSERT_THROW(Path("c:\\asdf").makeRelative(Path("asdf\\hello")), PathException);
            ASSERT_THROW(Path("asdf").makeRelative(Path("c:\\asdf\\hello")), PathException);
            ASSERT_THROW(Path("asdf").makeRelative(Path("c:\\")), PathException);
            ASSERT_THROW(Path("c:\\asdf").makeRelative(Path("d:\\asdf\\test")), PathException);
            ASSERT_THROW(Path("\\").makeRelative(Path("\\")), PathException);
            ASSERT_EQ(Path("..\\hurr\\hello"), Path("c:\\asdf").makeRelative(Path("c:\\hurr\\hello")));
            ASSERT_EQ(Path("..\\hello"), Path("c:\\asdf\\test\\blah").makeRelative(Path("c:\\asdf\\test\\hello")));
            ASSERT_EQ(Path("hello"), Path("c:\\asdf").makeRelative(Path("c:\\asdf\\hello")));
            ASSERT_EQ(Path("hello"), Path("c:\\.\\asdf").makeRelative(Path("c:\\asdf\\hello")));
            ASSERT_EQ(Path("hello"), Path("c:\\.\\asdf").makeRelative(Path("c:\\asdf\\hello")));
            ASSERT_EQ(Path("hello"), Path("c:\\asdf\\test\\..").makeRelative(Path("c:\\asdf\\.\\hello")));
            ASSERT_EQ(Path("hello"), Path("c:\\asdf\\test\\..\\").makeRelative(Path("c:\\asdf\\hurr\\..\\hello")));
        }

        TEST_CASE("PathTest.makeCanonical", "[PathTest]") {
            ASSERT_THROW(Path("c:\\..").makeCanonical(), PathException);
            ASSERT_THROW(Path("c:\\asdf\\..\\..").makeCanonical(), PathException);
            ASSERT_EQ(Path("c:\\asdf"), Path("c:\\asdf\\test\\..").makeCanonical());
        }

        TEST_CASE("PathTest.canMakeRelative", "[PathTest]") {
            // copied from makeRelative test
            ASSERT_FALSE(Path("c:\\asdf").canMakeRelative(Path("asdf\\hello")));
            ASSERT_FALSE(Path("asdf").canMakeRelative(Path("c:\\asdf\\hello")));
            ASSERT_FALSE(Path("asdf").canMakeRelative(Path("c:\\")));
            ASSERT_FALSE(Path("c:\\asdf").canMakeRelative(Path("d:\\asdf\\test")));
            ASSERT_FALSE(Path("\\").canMakeRelative(Path("\\")));
            ASSERT_TRUE(Path("c:\\asdf").canMakeRelative(Path("c:\\hurr\\hello")));
            ASSERT_TRUE(Path("c:\\asdf\\test\\blah").canMakeRelative(Path("c:\\asdf\\test\\hello")));
            ASSERT_TRUE(Path("c:\\asdf").canMakeRelative(Path("c:\\asdf\\hello")));
            ASSERT_TRUE(Path("c:\\.\\asdf").canMakeRelative(Path("c:\\asdf\\hello")));
            ASSERT_TRUE(Path("c:\\.\\asdf").canMakeRelative(Path("c:\\asdf\\hello")));
            ASSERT_TRUE(Path("c:\\asdf\\test\\..").canMakeRelative(Path("c:\\asdf\\.\\hello")));
            ASSERT_TRUE(Path("c:\\asdf\\test\\..\\").canMakeRelative(Path("c:\\asdf\\hurr\\..\\hello")));
        }

        TEST_CASE("PathTest.pathAsQString", "[PathTest]") {
            ASSERT_EQ(QString::fromLatin1("c:\\asdf\\test"), pathAsQString(Path("c:\\asdf\\test")));
            ASSERT_EQ(QString::fromLatin1("asdf\\test"), pathAsQString(Path("asdf\\test")));
        }

        TEST_CASE("PathTest.pathFromQString", "[PathTest]") {
            ASSERT_EQ(Path("c:\\asdf\\test"), pathFromQString(QString::fromLatin1("c:\\asdf\\test")));
            ASSERT_EQ(Path("asdf\\test"), pathFromQString(QString::fromLatin1("asdf\\test")));
        }
#else
        TEST_CASE("PathTest.constructWithString", "[PathTest]") {
            ASSERT_EQ(std::string(""), Path("").asString());
            ASSERT_EQ(std::string(""), Path(" ").asString());
            ASSERT_EQ(std::string("/"), Path("/").asString());
            ASSERT_EQ(std::string("/asdf"), Path("/asdf").asString());
            ASSERT_EQ(std::string("/asdf"), Path("/asdf/").asString());
            ASSERT_EQ(std::string("/asdf/df"), Path("/asdf/df").asString());
            ASSERT_EQ(std::string("hey"), Path("hey").asString());
            ASSERT_EQ(std::string("hey"), Path("hey/").asString());
            ASSERT_EQ(std::string("hey/asdf"), Path("hey/asdf").asString());
            ASSERT_EQ(std::string("./asdf"), Path("./asdf").asString());
        }

        TEST_CASE("PathTest.concatenate", "[PathTest]") {
            ASSERT_THROW(Path("") + Path("/"), PathException);
            ASSERT_THROW(Path("") + Path("/asdf"), PathException);
            ASSERT_THROW(Path("asdf") + Path("/asdf"), PathException);
            ASSERT_THROW(Path("/asdf") + Path("/asdf"), PathException);
            ASSERT_EQ(Path(""), Path("") + Path(""));
            ASSERT_EQ(Path("/"), Path("/") + Path(""));
            ASSERT_EQ(Path("/asdf"), Path("/asdf") + Path(""));
            ASSERT_EQ(Path("/asdf"), Path("/") + Path("asdf"));
            ASSERT_EQ(Path("/asdf/hey"), Path("/asdf") + Path("hey"));
            ASSERT_EQ(Path("asdf/hey"), Path("asdf") + Path("hey"));
        }

        TEST_CASE("PathTest.isEmpty", "[PathTest]") {
            ASSERT_TRUE(Path("").isEmpty());
            ASSERT_FALSE(Path("asdf").isEmpty());
            ASSERT_FALSE(Path("/").isEmpty());
            ASSERT_FALSE(Path("/asdf").isEmpty());
            ASSERT_FALSE(Path(".").isEmpty());
            ASSERT_FALSE(Path("/.").isEmpty());
        }

        TEST_CASE("PathTest.getLastComponent", "[PathTest]") {
            ASSERT_THROW(Path("").lastComponent().asString(), PathException);
            ASSERT_EQ("asdf", Path("/asdf").lastComponent().asString());
            ASSERT_EQ(Path("asdf"), Path("asdf").lastComponent());
            ASSERT_EQ(Path("path.map"), Path("/this/is/a/path.map").lastComponent());
            ASSERT_EQ(Path(""), Path("/").lastComponent());
        }

        TEST_CASE("PathTest.deleteLastComponent", "[PathTest]") {
            ASSERT_THROW(Path("").deleteLastComponent(), PathException);
            ASSERT_EQ(Path("/"), Path("/asdf").deleteLastComponent());
            ASSERT_EQ(Path(""), Path("asdf").deleteLastComponent());
            ASSERT_EQ(Path("/this/is/a"), Path("/this/is/a/path.map").deleteLastComponent());
            ASSERT_EQ(Path("/"), Path("/").deleteLastComponent());
        }

        TEST_CASE("PathTest.getFirstComponet", "[PathTest]") {
            ASSERT_THROW(Path("").firstComponent(), PathException);
            ASSERT_EQ("/", Path("/").firstComponent().asString());
            ASSERT_EQ("/", Path("/asdf").firstComponent().asString());
            ASSERT_EQ("asdf", Path("asdf").firstComponent().asString());
        }

        TEST_CASE("PathTest.deleteFirstComponent", "[PathTest]") {
            ASSERT_THROW(Path("").deleteFirstComponent(), PathException);
            ASSERT_EQ(Path(""), Path("/").deleteFirstComponent());
            ASSERT_EQ(Path("asdf"), Path("/asdf").deleteFirstComponent());
            ASSERT_EQ(Path("blah"), Path("asdf/blah").deleteFirstComponent());
        }

        TEST_CASE("PathTest.subPath", "[PathTest]") {
            ASSERT_EQ(Path(""), Path("").subPath(0, 0));
            ASSERT_THROW(Path("test/blah").subPath(1, 2), PathException);
            ASSERT_EQ(Path("test/blah"), Path("test/blah").subPath(0, 2));
            ASSERT_EQ(Path("test"), Path("test/blah").subPath(0, 1));
            ASSERT_EQ(Path("/test/blah"), Path("/test/blah").subPath(0, 2));
            ASSERT_EQ(Path("/test"), Path("/test/blah").subPath(0, 1));
            ASSERT_EQ(Path("blah"), Path("test/blah").subPath(1, 1));
        }

        TEST_CASE("PathTest.getExtension", "[PathTest]") {
            ASSERT_THROW(Path("").extension(), PathException);
            ASSERT_EQ(std::string(""), Path("asdf").extension());
            ASSERT_EQ(std::string("map"), Path("asdf.map").extension());
            ASSERT_EQ(std::string("map"), Path("/this/is/a/path.map").extension());
            ASSERT_EQ(std::string("textfile"), Path("/this/is/a/path.map.textfile").extension());
            ASSERT_EQ(std::string(""), Path("/").extension());
        }

        TEST_CASE("PathTest.addExtension", "[PathTest]") {
            ASSERT_THROW(Path("").addExtension("map"), PathException);
            ASSERT_EQ(Path("/asdf."), Path("/asdf").addExtension(""));
            ASSERT_EQ(Path("/asdf.map"), Path("/asdf").addExtension("map"));
            ASSERT_EQ(Path("/asdf.map.test"), Path("/asdf.map").addExtension("test"));
            ASSERT_EQ(Path("/.map"), Path("/").addExtension("map"));
        }

        TEST_CASE("PathTest.makeAbsolute", "[PathTest]") {
            ASSERT_THROW(Path("/asdf").makeAbsolute(Path("/hello")), PathException);
            ASSERT_THROW(Path("asdf").makeAbsolute(Path("hello")), PathException);
            ASSERT_EQ(Path("/asdf/hello"), Path("/asdf").makeAbsolute(Path("hello")));
        }

        TEST_CASE("PathTest.makeRelative", "[PathTest]") {
            ASSERT_THROW(Path("").makeRelative(), PathException);
            ASSERT_THROW(Path("models/barrel/skin.tga").makeRelative(), PathException);
            ASSERT_EQ(Path(""), Path("/").makeRelative());
            ASSERT_EQ(Path("models/barrel/skin.tga"), Path("/models/barrel/skin.tga").makeRelative());
        }

        TEST_CASE("PathTest.makeRelativeWithAbsolutePath", "[PathTest]") {
            ASSERT_THROW(Path("/asdf").makeRelative(Path("asdf/hello")), PathException);
            ASSERT_THROW(Path("asdf").makeRelative(Path("/asdf/hello")), PathException);
            ASSERT_THROW(Path("asdf").makeRelative(Path("/")), PathException);
            ASSERT_EQ(Path("../hello"), Path("/asdf/test/blah").makeRelative(Path("/asdf/test/hello")));
            ASSERT_EQ(Path("../hurr/hello"), Path("/asdf").makeRelative(Path("/hurr/hello")));
            ASSERT_EQ(Path("hello"), Path("/asdf").makeRelative(Path("/asdf/hello")));
            ASSERT_EQ(Path("hello"), Path("/./asdf").makeRelative(Path("/asdf/hello")));
            ASSERT_EQ(Path("hello"), Path("/./asdf").makeRelative(Path("/asdf/hello")));
            ASSERT_EQ(Path("hello"), Path("/asdf/test/..").makeRelative(Path("/asdf/./hello")));
            ASSERT_EQ(Path("hello"), Path("/asdf/test/../").makeRelative(Path("/asdf/hurr/../hello")));
        }

        TEST_CASE("PathTest.makeCanonical", "[PathTest]") {
            ASSERT_THROW(Path("/..").makeCanonical(), PathException);
            ASSERT_THROW(Path("/asdf/../..").makeCanonical(), PathException);
            ASSERT_EQ(Path("/asdf"), Path("/asdf/test/..").makeCanonical());
        }

        TEST_CASE("PathTest.operatorLT", "[PathTest]") {
            ASSERT_FALSE(Path("") < Path(""));
            ASSERT_FALSE(Path("/") < Path(""));
            ASSERT_FALSE(Path("/") < Path("/"));
            ASSERT_FALSE(Path("dir") < Path(""));
            ASSERT_FALSE(Path("dir") < Path("dir"));
            ASSERT_FALSE(Path("/dir") < Path("dir"));
            ASSERT_FALSE(Path("/dir") < Path("/dir"));
            ASSERT_TRUE(Path("dir") < Path("/dir"));
            ASSERT_TRUE(Path("dir") < Path("dir/dir2"));
            ASSERT_TRUE(Path("dir/dir") < Path("dir/dir2"));
            ASSERT_FALSE(Path("dir/dir2") < Path("dir/dir2"));
            ASSERT_FALSE(Path("dir/dir2/dir3") < Path("dir/dir2"));
        }

        TEST_CASE("PathTest.pathAsQString", "[PathTest]") {
            ASSERT_EQ(QString::fromLatin1("/asdf/test"), pathAsQString(Path("/asdf/test")));
            ASSERT_EQ(QString::fromLatin1("asdf/test"), pathAsQString(Path("asdf/test")));
        }

        TEST_CASE("PathTest.pathFromQString", "[PathTest]") {
            ASSERT_EQ(Path("/asdf/test"), pathFromQString(QString::fromLatin1("/asdf/test")));
            ASSERT_EQ(Path("asdf/test"), pathFromQString(QString::fromLatin1("asdf/test")));
        }
#endif
    }
}
