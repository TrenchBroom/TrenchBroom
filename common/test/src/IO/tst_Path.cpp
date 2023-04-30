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

namespace TrenchBroom::IO
{
#ifdef _WIN32
TEST_CASE("PathTest.constructWithString")
{
  CHECK(Path{}.asString() == std::string(""));
  CHECK(Path{" "}.asString() == std::string(""));
  CHECK(Path{"c:\\"}.asString() == std::string("c:"));
  CHECK(Path{"c:\\asdf"}.asString() == std::string("c:\\asdf"));
  CHECK(Path{"c:\\asdf\\"}.asString() == std::string("c:\\asdf"));
  CHECK(Path{"c:\\asdf\\df"}.asString() == std::string("c:\\asdf\\df"));
  CHECK(Path{"hey"}.asString() == std::string("hey"));
  CHECK(Path{"hey\\"}.asString() == std::string("hey"));
  CHECK(Path{"hey\\asdf"}.asString() == std::string("hey\\asdf"));
  CHECK(Path{".\\asdf"}.asString() == std::string(".\\asdf"));
}

TEST_CASE("PathTest.concatenate")
{
  CHECK_THROWS_AS(Path{} / Path{"c:\\"}, PathException);
  CHECK_THROWS_AS(Path{} / Path{"c:\\asdf"}, PathException);
  CHECK_THROWS_AS(Path{"asdf"} / Path{"c:\\asdf"}, PathException);
  CHECK_THROWS_AS(Path{"c:\\asdf"} / Path{"c:\\asdf"}, PathException);
  CHECK(Path{} / Path{} == Path{});
  CHECK(Path{"c:\\"} / Path{} == Path{"c:\\"});
  CHECK(Path{"c:\\asdf"} / Path{} == Path{"c:\\asdf"});
  CHECK(Path{"c:\\"} / Path{"asdf"} == Path{"c:\\asdf"});
  CHECK(Path{"c:\\asdf"} / Path{"hey"} == Path{"c:\\asdf\\hey"});
  CHECK(Path{"asdf"} / Path{"hey"} == Path{"asdf\\hey"});
}

TEST_CASE("PathTest.isEmpty")
{
  CHECK(Path{}.isEmpty());
  CHECK_FALSE(Path{"asdf"}.isEmpty());
  CHECK_FALSE(Path{"c:"}.isEmpty());
  CHECK_FALSE(Path{"c:\\asdf"}.isEmpty());
  CHECK_FALSE(Path{"."}.isEmpty());
  CHECK_FALSE(Path{"c:\\."}.isEmpty());
}

TEST_CASE("PathTest.getLastComponent")
{
  CHECK_THROWS_AS(Path{}.lastComponent().asString(), PathException);
  CHECK(Path{"c:\\asdf"}.lastComponent().asString() == "asdf");
  CHECK(Path{"asdf"}.lastComponent() == Path{"asdf"});
  CHECK(Path{"c:\\this\\is\\a\\path.map"}.lastComponent() == Path{"path.map"});
  CHECK(Path{"/"}.lastComponent() == Path{});
}

TEST_CASE("PathTest.deleteLastComponent")
{
  CHECK_THROWS_AS(Path{}.deleteLastComponent(), PathException);
  CHECK(Path{"c:\\asdf"}.deleteLastComponent() == Path{"c:\\"});
  CHECK(Path{"asdf"}.deleteLastComponent() == Path{});
  CHECK(
    Path{"c:\\this\\is\\a\\path.map"}.deleteLastComponent() == Path{"c:\\this\\is\\a"});
}

TEST_CASE("PathTest.getFirstComponent")
{
  CHECK_THROWS_AS(Path{}.firstComponent(), PathException);
  CHECK(Path{"/asdf"}.firstComponent().asString() == "\\");
  CHECK(Path{"c:\\asdf\\blah"}.firstComponent().asString() == "c:");
  CHECK(Path{"asdf\\bbab"}.firstComponent().asString() == "asdf");
}

TEST_CASE("PathTest.deleteFirstComponent")
{
  CHECK_THROWS_AS(Path{}.deleteFirstComponent(), PathException);
  CHECK(Path{"\\"}.deleteFirstComponent() == Path{});
  CHECK(Path{"\\asdf"}.deleteFirstComponent() == Path{"asdf"});
  CHECK(Path{"c:\\"}.deleteFirstComponent() == Path{});
  CHECK(Path{"c:\\asdf"}.deleteFirstComponent() == Path{"asdf"});
  CHECK(Path{"/asdf"}.deleteFirstComponent() == Path{"asdf"});
  CHECK(Path{"asdf/blah"}.deleteFirstComponent() == Path{"blah"});
}

TEST_CASE("PathTest.subPath")
{
  CHECK(Path{}.subPath(0, 0) == Path{});
  CHECK_THROWS_AS(Path{"test\\blah"}.subPath(1, 2), PathException);
  CHECK(Path{"test\\blah"}.subPath(0, 2) == Path{"test\\blah"});
  CHECK(Path{"test\\blah"}.subPath(0, 1) == Path{"test"});
  CHECK(Path{"c:\\test\\blah"}.subPath(0, 3) == Path{"c:\\test\\blah"});
  CHECK(Path{"c:\\test\\blah"}.subPath(0, 2) == Path{"c:\\test"});
  CHECK(Path{"test\\blah"}.subPath(1, 1) == Path{"blah"});
}

TEST_CASE("PathTest.getExtension")
{
  CHECK_THROWS_AS(Path{}.extension(), PathException);
  CHECK(Path{"asdf"}.extension() == std::string(""));
  CHECK(Path{"asdf.map"}.extension() == std::string("map"));
  CHECK(Path{"c:\\this\\is\\a\\path.map"}.extension() == std::string("map"));
  CHECK(
    Path{"c:\\this\\is\\a\\path.map.textfile"}.extension() == std::string("textfile"));
  CHECK(Path{"c:\\"}.extension() == std::string(""));
}

TEST_CASE("PathTest.deleteExtension")
{
  CHECK(Path{}.deleteExtension() == Path{});
  CHECK(Path{"asdf"}.deleteExtension() == Path{"asdf"});
  CHECK(Path{"asdf.jpeg"}.deleteExtension() == Path{"asdf"});
  CHECK(Path{"c:\\asdf.jpeg"}.deleteExtension() == Path{"c:\\asdf"});
  CHECK(Path{"x\\asdf.jpeg"}.deleteExtension() == Path{"x\\asdf"});
}

TEST_CASE("PathTest.addExtension")
{
  const auto test = Path{"c:\\"}.addExtension("map").asString();
  const auto test2 = test;
  const auto test3 = test + test2;

  CHECK_THROWS_AS(Path{}.addExtension("map"), PathException);
  CHECK(Path{"c:\\asdf"}.addExtension("") == Path{"c:\\asdf."});
  CHECK(Path{"c:\\asdf"}.addExtension("map") == Path{"c:\\asdf.map"});
  CHECK(Path{"c:\\asdf.map"}.addExtension("test") == Path{"c:\\asdf.map.test"});
  CHECK(Path{"c:\\"}.addExtension("map") == Path{"c:\\.map"});
}

TEST_CASE("PathTest.makeAbsolute")
{
  CHECK_THROWS_AS(Path{"c:\\asdf"}.makeAbsolute(Path{"c:\\hello"}), PathException);
  CHECK_THROWS_AS(Path{"asdf"}.makeAbsolute(Path{"hello"}), PathException);
  CHECK(Path{"c:\\asdf"}.makeAbsolute(Path{"hello"}) == Path{"c:\\asdf\\hello"});
}

TEST_CASE("PathTest.makeRelative")
{
  CHECK_THROWS_AS(Path{}.makeRelative(), PathException);
  CHECK_THROWS_AS(Path{"models\\barrel\\skin.tga"}.makeRelative(), PathException);
  CHECK(Path{"C:"}.makeRelative() == Path{});
  CHECK(Path{"C:\\"}.makeRelative() == Path{});
  CHECK(
    Path{"C:\\models\\barrel\\skin.tga"}.makeRelative()
    == Path{"models\\barrel\\skin.tga"});
}

TEST_CASE("PathTest.makeRelativeWithAbsolutePath")
{
  CHECK_THROWS_AS(Path{"c:\\asdf"}.makeRelative(Path{"asdf\\hello"}), PathException);
  CHECK_THROWS_AS(Path{"asdf"}.makeRelative(Path{"c:\\asdf\\hello"}), PathException);
  CHECK_THROWS_AS(Path{"asdf"}.makeRelative(Path{"c:\\"}), PathException);
  CHECK_THROWS_AS(Path{"c:\\asdf"}.makeRelative(Path{"d:\\asdf\\test"}), PathException);
  CHECK_THROWS_AS(Path{"\\"}.makeRelative(Path{"\\"}), PathException);
  CHECK(
    Path{"c:\\asdf"}.makeRelative(Path{"c:\\hurr\\hello"}) == Path{"..\\hurr\\hello"});
  CHECK(
    Path{"c:\\asdf\\test\\blah"}.makeRelative(Path{"c:\\asdf\\test\\hello"})
    == Path{"..\\hello"});
  CHECK(Path{"c:\\asdf"}.makeRelative(Path{"c:\\asdf\\hello"}) == Path{"hello"});
  CHECK(Path{"c:\\.\\asdf"}.makeRelative(Path{"c:\\asdf\\hello"}) == Path{"hello"});
  CHECK(Path{"c:\\.\\asdf"}.makeRelative(Path{"c:\\asdf\\hello"}) == Path{"hello"});
  CHECK(
    Path{"c:\\asdf\\test\\.."}.makeRelative(Path{"c:\\asdf\\.\\hello"}) == Path{"hello"});
  CHECK(
    Path{"c:\\asdf\\test\\..\\"}.makeRelative(Path{"c:\\asdf\\hurr\\..\\hello"})
    == Path{"hello"});
}

TEST_CASE("PathTest.makeCanonical")
{
  CHECK_THROWS_AS(Path{"c:\\.."}.makeCanonical(), PathException);
  CHECK_THROWS_AS(Path{"c:\\asdf\\..\\.."}.makeCanonical(), PathException);
  CHECK(Path{"c:\\asdf\\test\\.."}.makeCanonical() == Path{"c:\\asdf"});
}

TEST_CASE("PathTest.canMakeRelative")
{
  // copied from makeRelative test
  CHECK_FALSE(Path{"c:\\asdf"}.canMakeRelative(Path{"asdf\\hello"}));
  CHECK_FALSE(Path{"asdf"}.canMakeRelative(Path{"c:\\asdf\\hello"}));
  CHECK_FALSE(Path{"asdf"}.canMakeRelative(Path{"c:\\"}));
  CHECK_FALSE(Path{"c:\\asdf"}.canMakeRelative(Path{"d:\\asdf\\test"}));
  CHECK_FALSE(Path{"\\"}.canMakeRelative(Path{"\\"}));
  CHECK(Path{"c:\\asdf"}.canMakeRelative(Path{"c:\\hurr\\hello"}));
  CHECK(Path{"c:\\asdf\\test\\blah"}.canMakeRelative(Path{"c:\\asdf\\test\\hello"}));
  CHECK(Path{"c:\\asdf"}.canMakeRelative(Path{"c:\\asdf\\hello"}));
  CHECK(Path{"c:\\.\\asdf"}.canMakeRelative(Path{"c:\\asdf\\hello"}));
  CHECK(Path{"c:\\.\\asdf"}.canMakeRelative(Path{"c:\\asdf\\hello"}));
  CHECK(Path{"c:\\asdf\\test\\.."}.canMakeRelative(Path{"c:\\asdf\\.\\hello"}));
  CHECK(Path{"c:\\asdf\\test\\..\\"}.canMakeRelative(Path{"c:\\asdf\\hurr\\..\\hello"}));
}

TEST_CASE("PathTest.pathAsQString")
{
  CHECK(pathAsQString(Path{"c:\\asdf\\test"}) == QString::fromLatin1("c:\\asdf\\test"));
  CHECK(pathAsQString(Path{"asdf\\test"}) == QString::fromLatin1("asdf\\test"));
}

TEST_CASE("PathTest.pathFromQString")
{
  CHECK(pathFromQString(QString::fromLatin1("c:\\asdf\\test")) == Path{"c:\\asdf\\test"});
  CHECK(pathFromQString(QString::fromLatin1("asdf\\test")) == Path{"asdf\\test"});
}
#else
TEST_CASE("PathTest.constructWithString")
{
  CHECK(Path{}.asString() == std::string(""));
  CHECK(Path{" "}.asString() == std::string(" "));
  CHECK(Path{"/"}.asString() == std::string("/"));
  CHECK(Path{"/asdf"}.asString() == std::string("/asdf"));
  CHECK(Path{"/asdf/"}.asString() == std::string("/asdf/"));
  CHECK(Path{"/asdf/df"}.asString() == std::string("/asdf/df"));
  CHECK(Path{"hey"}.asString() == std::string("hey"));
  CHECK(Path{"hey/"}.asString() == std::string("hey/"));
  CHECK(Path{"hey/asdf"}.asString() == std::string("hey/asdf"));
  CHECK(Path{"./asdf"}.asString() == std::string("./asdf"));
}

TEST_CASE("PathTest.concatenate")
{
  CHECK(Path{} / Path{"/"} == Path{"/"});
  CHECK(Path{} / Path{"/asdf"} == Path{"/asdf"});
  CHECK(Path{"blah"} / Path{"/asdf"} == Path{"/asdf"});
  CHECK(Path{"/blah"} / Path{"/asdf"} == Path{"/asdf"});
  CHECK(Path{} / Path{} == Path{});
  CHECK(Path{"/"} / Path{} == Path{"/"});
  CHECK(Path{"/asdf"} / Path{} == Path{"/asdf/"});
  CHECK(Path{"/"} / Path{"asdf"} == Path{"/asdf"});
  CHECK(Path{"/asdf"} / Path{"hey"} == Path{"/asdf/hey"});
  CHECK(Path{"asdf"} / Path{"hey"} == Path{"asdf/hey"});
}

TEST_CASE("PathTest.isEmpty")
{
  CHECK(Path{}.isEmpty());
  CHECK_FALSE(Path{"asdf"}.isEmpty());
  CHECK_FALSE(Path{"/"}.isEmpty());
  CHECK_FALSE(Path{"/asdf"}.isEmpty());
  CHECK_FALSE(Path{"."}.isEmpty());
  CHECK_FALSE(Path{"/."}.isEmpty());
}

TEST_CASE("PathTest.getLastComponent")
{
  CHECK(Path{}.lastComponent() == Path{});
  CHECK(Path{"/asdf"}.lastComponent() == Path{"asdf"});
  CHECK(Path{"asdf"}.lastComponent() == Path{"asdf"});
  CHECK(Path{"/this/is/a/path.map"}.lastComponent() == Path{"path.map"});
  CHECK(Path{"/"}.lastComponent() == Path{"/"});
}

TEST_CASE("PathTest.deleteLastComponent")
{
  CHECK(Path{}.deleteLastComponent() == Path{});
  CHECK(Path{"/asdf"}.deleteLastComponent() == Path{"/"});
  CHECK(Path{"asdf"}.deleteLastComponent() == Path{});
  CHECK(Path{"/this/is/a/path.map"}.deleteLastComponent() == Path{"/this/is/a"});
  CHECK(Path{"/"}.deleteLastComponent() == Path{"/"});
}

TEST_CASE("PathTest.getFirstComponet")
{
  CHECK(Path{}.firstComponent() == Path{});
  CHECK(Path{"/"}.firstComponent() == Path{"/"});
  CHECK(Path{"/asdf"}.firstComponent() == Path{"/"});
  CHECK(Path{"asdf"}.firstComponent() == Path{"asdf"});
}

TEST_CASE("PathTest.deleteFirstComponent")
{
  CHECK(Path{}.deleteFirstComponent() == Path{});
  CHECK(Path{"/"}.deleteFirstComponent() == Path{});
  CHECK(Path{"/asdf"}.deleteFirstComponent() == Path{"asdf"});
  CHECK(Path{"asdf/blah"}.deleteFirstComponent() == Path{"blah"});
}

TEST_CASE("PathTest.subPath")
{
  CHECK(Path{}.subPath(0, 0) == Path{});
  CHECK(Path{"test/blah"}.subPath(1, 2) == Path{"blah"});
  CHECK(Path{"test/blah"}.subPath(3, 2) == Path{});
  CHECK(Path{"test/blah"}.subPath(0, 2) == Path{"test/blah"});
  CHECK(Path{"test/blah"}.subPath(0, 1) == Path{"test"});
  CHECK(Path{"test/blah"}.subPath(1, 1) == Path{"blah"});
  CHECK(Path{"/test/blah"}.subPath(0, 3) == Path{"/test/blah"});
  CHECK(Path{"/test/blah"}.subPath(1, 2) == Path{"test/blah"});
  CHECK(Path{"/test/blah"}.subPath(2, 1) == Path{"blah"});
  CHECK(Path{"/test/blah"}.subPath(0, 2) == Path{"/test"});
  CHECK(Path{"/test/blah"}.subPath(0, 1) == Path{"/"});
  CHECK(Path{"/test/blah"}.subPath(0, 0) == Path{""});
}

TEST_CASE("PathTest.getExtension")
{
  CHECK(Path{}.extension().empty());
  CHECK(Path{"asdf"}.extension() == std::string(""));
  CHECK(Path{"asdf.map"}.extension() == std::string(".map"));
  CHECK(Path{"/this/is/a/path.map"}.extension() == std::string(".map"));
  CHECK(Path{"/this/is/a/path.map.textfile"}.extension() == std::string(".textfile"));
  CHECK(Path{"/"}.extension() == std::string(""));
}

TEST_CASE("PathTest.deleteExtension")
{
  CHECK(Path{}.deleteExtension() == Path{});
  CHECK(Path{"asdf"}.deleteExtension() == Path{"asdf"});
  CHECK(Path{"asdf.jpeg"}.deleteExtension() == Path{"asdf"});
  CHECK(Path{"/asdf.jpeg"}.deleteExtension() == Path{"/asdf"});
  CHECK(Path{"x/asdf.jpeg"}.deleteExtension() == Path{"x/asdf"});
}

TEST_CASE("PathTest.addExtension")
{
  CHECK(Path{}.addExtension(".map") == Path{".map"});
  CHECK(Path{"/asdf"}.addExtension(".") == Path{"/asdf."});
  CHECK(Path{"/asdf"}.addExtension(".map") == Path{"/asdf.map"});
  CHECK(Path{"/asdf.map"}.addExtension(".test") == Path{"/asdf.map.test"});
  CHECK(Path{"/"}.addExtension(".map") == Path{"/.map"});
}

TEST_CASE("PathTest.makeRelative")
{
  CHECK(Path{}.makeRelative() == Path{});
  CHECK(Path{"models/barrel/skin.tga"}.makeRelative() == Path{"models/barrel/skin.tga"});
  CHECK(Path{"/"}.makeRelative() == Path{});
  CHECK(Path{"/models/barrel/skin.tga"}.makeRelative() == Path{"models/barrel/skin.tga"});
  CHECK(Path{"/models/barrel/skin.tga"}.makeRelative() == Path{"models/barrel/skin.tga"});
}

TEST_CASE("PathTest.makeRelativeWithAbsolutePath")
{
  CHECK(Path{"/asdf"}.makeRelative(Path{"asdf/hello"}) == Path{});
  CHECK(Path{"asdf"}.makeRelative(Path{"/asdf/hello"}) == Path{});
  CHECK(Path{"asdf"}.makeRelative(Path{"/"}) == Path{});
  CHECK(
    Path{"/asdf/test/blah"}.makeRelative(Path{"/asdf/test/hello"}) == Path{"../hello"});
  CHECK(Path{"/asdf"}.makeRelative(Path{"/hurr/hello"}) == Path{"../hurr/hello"});
  CHECK(Path{"/asdf"}.makeRelative(Path{"/asdf/hello"}) == Path{"hello"});
  CHECK(Path{"/./asdf"}.makeRelative(Path{"/asdf/hello"}) == Path{"../asdf/hello"});
  CHECK(Path{"/asdf/test/.."}.makeRelative(Path{"/asdf/./hello"}) == Path{"./hello"});
  CHECK(
    Path{"/asdf/test/../"}.makeRelative(Path{"/asdf/hurr/../hello"})
    == Path{"hurr/../hello"});
}

TEST_CASE("PathTest.makeCanonical")
{
  CHECK(Path{"/.."}.makeCanonical() == Path{"/"});
  CHECK(Path{"/../.."}.makeCanonical() == Path{"/"});
  CHECK(Path{"/asdf/../.."}.makeCanonical() == Path{"/"});
  CHECK(Path{"/asdf/test/.."}.makeCanonical() == Path{"/asdf/"});
  CHECK(Path{"/asdf/./test/.."}.makeCanonical() == Path{"/asdf/"});

  CHECK_THROWS_AS(Path{".."}.makeCanonical(), PathException);
  CHECK_THROWS_AS(Path{"asdf/../.."}.makeCanonical(), PathException);
  CHECK_THROWS_AS(Path{"./.."}.makeCanonical(), PathException);
  CHECK(Path{"./asdf/./test/.."}.makeCanonical() == Path{"asdf/"});
  CHECK(Path{"asdf/test/.."}.makeCanonical() == Path{"asdf/"});
}

TEST_CASE("PathTest.operatorLT")
{
  CHECK_FALSE(Path{} < Path{});
  CHECK_FALSE(Path{"/"} < Path{});
  CHECK_FALSE(Path{"/"} < Path{"/"});
  CHECK_FALSE(Path{"dir"} < Path{});
  CHECK_FALSE(Path{"dir"} < Path{"dir"});
  CHECK_FALSE(Path{"/dir"} < Path{"dir"});
  CHECK_FALSE(Path{"/dir"} < Path{"/dir"});
  CHECK(Path{"dir"} < Path{"/dir"});
  CHECK(Path{"dir"} < Path{"dir/dir2"});
  CHECK(Path{"dir/dir"} < Path{"dir/dir2"});
  CHECK_FALSE(Path{"dir/dir2"} < Path{"dir/dir2"});
  CHECK_FALSE(Path{"dir/dir2/dir3"} < Path{"dir/dir2"});
}

TEST_CASE("PathTest.pathAsQString")
{
  CHECK(pathAsQString(Path{"/asdf/test"}) == QString::fromLatin1("/asdf/test"));
  CHECK(pathAsQString(Path{"asdf/test"}) == QString::fromLatin1("asdf/test"));
}

TEST_CASE("PathTest.pathFromQString")
{
  CHECK(pathFromQString(QString::fromLatin1("/asdf/test")) == Path{"/asdf/test"});
  CHECK(pathFromQString(QString::fromLatin1("asdf/test")) == Path{"asdf/test"});
}
#endif
} // namespace TrenchBroom::IO
