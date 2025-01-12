/*
 Copyright 2023 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "kdl/k.h"
#include "kdl/path_utils.h"

#include "catch2.h"

namespace kdl
{
using std::filesystem::path;

TEST_CASE("parse_path")
{
  CHECK(parse_path(R"()") == path{R"()"});
  CHECK(parse_path(R"(/)") == path{R"(/)"});
  CHECK(parse_path(R"(\)") == path{R"(/)"});
  CHECK(parse_path(R"(\)", !K(replace_backslashes)) == path{R"(\)"});
  CHECK(parse_path(R"(a/b/c)") == path{R"(a/b/c)"});
  CHECK(parse_path(R"(a\b\c)") == path{R"(a/b/c)"});
  CHECK(parse_path(R"(a\b\c)", !K(replace_backslashes)) == path{R"(a\b\c)"});
}

TEST_CASE("path_length")
{
  CHECK(path_length(path{}) == 0);
  CHECK(path_length(path{""}) == 0);
  CHECK(path_length(path{"/"}) == 1);
  CHECK(path_length(path{"/asdf"}) == 2);
  CHECK(path_length(path{"/asdf/"}) == 3);
  CHECK(path_length(path{"/asdf/blah"}) == 3);
  CHECK(path_length(path{"asdf"}) == 1);
  CHECK(path_length(path{"asdf/"}) == 2);
  CHECK(path_length(path{"asdf/blah"}) == 2);
}

TEST_CASE("path_has_prefix")
{
  CHECK(path_has_prefix(path{}, path{}));
  CHECK(path_has_prefix(path{""}, path{}));
  CHECK(path_has_prefix(path{"/"}, path{}));
  CHECK(path_has_prefix(path{"asdf"}, path{}));
  CHECK(path_has_prefix(path{"/asdf"}, path{}));
  CHECK(path_has_prefix(path{"asdf/blah"}, path{}));
  CHECK(path_has_prefix(path{"/asdf/blah"}, path{}));

  CHECK(path_has_prefix(path{""}, path{""}));
  CHECK(path_has_prefix(path{"/"}, path{""}));
  CHECK(path_has_prefix(path{"asdf"}, path{""}));
  CHECK(path_has_prefix(path{"/asdf"}, path{""}));
  CHECK(path_has_prefix(path{"asdf/blah"}, path{""}));
  CHECK(path_has_prefix(path{"/asdf/blah"}, path{""}));

  CHECK_FALSE(path_has_prefix(path{""}, path{"/"}));
  CHECK(path_has_prefix(path{"/"}, path{"/"}));
  CHECK_FALSE(path_has_prefix(path{"asdf"}, path{"/"}));
  CHECK(path_has_prefix(path{"/asdf"}, path{"/"}));
  CHECK_FALSE(path_has_prefix(path{"asdf/blah"}, path{"/"}));
  CHECK(path_has_prefix(path{"/asdf/blah"}, path{"/"}));

  CHECK_FALSE(path_has_prefix(path{""}, path{"/asdf"}));
  CHECK_FALSE(path_has_prefix(path{"/"}, path{"/asdf"}));
  CHECK_FALSE(path_has_prefix(path{"asdf"}, path{"/asdf"}));
  CHECK(path_has_prefix(path{"/asdf"}, path{"/asdf"}));
  CHECK_FALSE(path_has_prefix(path{"asdf/blah"}, path{"/asdf"}));
  CHECK(path_has_prefix(path{"/asdf/blah"}, path{"/asdf"}));

  CHECK_FALSE(path_has_prefix(path{""}, path{"asdf"}));
  CHECK_FALSE(path_has_prefix(path{"/"}, path{"asdf"}));
  CHECK(path_has_prefix(path{"asdf"}, path{"asdf"}));
  CHECK_FALSE(path_has_prefix(path{"/asdf"}, path{"asdf"}));
  CHECK(path_has_prefix(path{"asdf/blah"}, path{"asdf"}));
  CHECK_FALSE(path_has_prefix(path{"/asdf/blah"}, path{"asdf"}));
}

TEST_CASE("path_front")
{
  CHECK(path_front(path{}) == path{});
  CHECK(path_front(path{""}) == path{});
  CHECK(path_front(path{"/"}) == path{"/"});
  CHECK(path_front(path{"/asdf"}) == path{"/"});
  CHECK(path_front(path{"/asdf/blah"}) == path{"/"});
  CHECK(path_front(path{"asdf"}) == path{"asdf"});
  CHECK(path_front(path{"asdf/blah"}) == path{"asdf"});
}

TEST_CASE("path_to_lower")
{
  CHECK(path_to_lower(path{}) == path{});
  CHECK(path_to_lower(path{"/"}) == path{"/"});
  CHECK(path_to_lower(path{"/this/that"}) == path{"/this/that"});
  CHECK(path_to_lower(path{"/THIS/that"}) == path{"/this/that"});
  CHECK(path_to_lower(path{"/THIS/THAT"}) == path{"/this/that"});
  CHECK(path_to_lower(path{"C:\\THIS\\THAT"}) == path{"c:\\this\\that"});
}

TEST_CASE("PathTest.path_clip")
{
  CHECK(path_clip(path{}, 0, 0) == path{});
  CHECK(path_clip(path{"test"}, 0, 1) == path{"test"});
  CHECK(path_clip(path{"test"}, 0, 2) == path{"test"});
  CHECK(path_clip(path{"test/blah"}, 1, 1) == path{"blah"});
  CHECK(path_clip(path{"test/blah"}, 1, 2) == path{"blah"});
  CHECK(path_clip(path{"test/blah"}, 3, 2) == path{});
  CHECK(path_clip(path{"test/blah"}, 0, 2) == path{"test/blah"});
  CHECK(path_clip(path{"test/blah"}, 0, 1) == path{"test"});
  CHECK(path_clip(path{"test/blah"}, 1, 1) == path{"blah"});
  CHECK(path_clip(path{"/test/blah"}, 0, 3) == path{"/test/blah"});
  CHECK(path_clip(path{"/test/blah"}, 1, 2) == path{"test/blah"});
  CHECK(path_clip(path{"/test/blah"}, 2, 1) == path{"blah"});
  CHECK(path_clip(path{"/test/blah"}, 0, 2) == path{"/test"});
  CHECK(path_clip(path{"/test/blah"}, 0, 1) == path{"/"});
  CHECK(path_clip(path{"/test/blah"}, 0, 0) == path{""});

#ifdef _WIN32
  CHECK(path_clip(path{R"(test\blah)"}, 0, 2) == path{R"(test\blah)"});
  CHECK(path_clip(path{R"(test\blah)"}, 0, 1) == path{R"(test)"});
  CHECK(path_clip(path{R"(c:\test\blah)"}, 0, 4) == path{R"(c:\test\blah)"});
  CHECK(path_clip(path{R"(c:\test\blah)"}, 0, 3) == path{R"(c:\test)"});
  CHECK(path_clip(path{R"(c:\test\blah)"}, 1, 3) == path{R"(\test\blah)"});
  CHECK(path_clip(path{R"(test\blah)"}, 1, 1) == path{R"(blah)"});
#endif
}

TEST_CASE("path_pop_front")
{
  CHECK(path_pop_front(path{}) == path{});
  CHECK(path_pop_front(path{""}) == path{});
  CHECK(path_pop_front(path{"/"}) == path{});
  CHECK(path_pop_front(path{"/asdf"}) == path{"asdf"});
  CHECK(path_pop_front(path{"/asdf/blah"}) == path{"asdf/blah"});
  CHECK(path_pop_front(path{"asdf"}) == path{});
  CHECK(path_pop_front(path{"asdf/blah"}) == path{"blah"});
}

TEST_CASE("path_add_extension")
{
  CHECK(path_add_extension(path{}, path{}) == path{});
  CHECK(path_add_extension(path{}, path{".ext"}) == path{".ext"});
  CHECK(path_add_extension(path{"asdf"}, path{".ext"}) == path{"asdf.ext"});
  CHECK(path_add_extension(path{"asdf.xyz"}, path{".ext"}) == path{"asdf.xyz.ext"});
  CHECK(path_add_extension(path{"/"}, path{".ext"}) == path{"/.ext"});
  CHECK(path_add_extension(path{"/asdf"}, path{".ext"}) == path{"/asdf.ext"});
  CHECK(path_add_extension(path{"/asdf.xyz"}, path{".ext"}) == path{"/asdf.xyz.ext"});
}

TEST_CASE("path_remove_extension")
{
  CHECK(path_remove_extension(path{}) == path{});
  CHECK(path_remove_extension(path{".ext"}) == path{".ext"});
  CHECK(path_remove_extension(path{"asdf.ext"}) == path{"asdf"});
  CHECK(path_remove_extension(path{"asdf.xyz.ext"}) == path{"asdf.xyz"});
  CHECK(path_remove_extension(path{"/.ext"}) == path{"/.ext"});
  CHECK(path_remove_extension(path{"/asdf.ext"}) == path{"/asdf"});
  CHECK(path_remove_extension(path{"/asdf.xyz.ext"}) == path{"/asdf.xyz"});
}

TEST_CASE("path_replace_extension")
{
  CHECK(path_replace_extension(path{}, path{".new"}) == path{".new"});
  CHECK(path_replace_extension(path{"asdf"}, path{".new"}) == path{"asdf.new"});
  CHECK(path_replace_extension(path{"asdf.xyz"}, path{".new"}) == path{"asdf.new"});
  CHECK(path_replace_extension(path{"/"}, path{".new"}) == path{"/.new"});
  CHECK(path_replace_extension(path{"/asdf"}, path{".new"}) == path{"/asdf.new"});
  CHECK(path_replace_extension(path{"/asdf.xyz"}, path{".new"}) == path{"/asdf.new"});

  CHECK(path_replace_extension(path{".ext"}, path{".new"}) == path{".ext.new"});
  CHECK(path_replace_extension(path{"asdf.ext"}, path{".new"}) == path{"asdf.new"});
  CHECK(
    path_replace_extension(path{"asdf.xyz.ext"}, path{".new"}) == path{"asdf.xyz.new"});
  CHECK(path_replace_extension(path{"/.ext"}, path{".new"}) == path{"/.ext.new"});
  CHECK(path_replace_extension(path{"/asdf.ext"}, path{".new"}) == path{"/asdf.new"});
  CHECK(
    path_replace_extension(path{"/asdf.xyz.ext"}, path{".new"}) == path{"/asdf.xyz.new"});

  CHECK(path_replace_extension(path{}, path{}) == path{});
  CHECK(path_replace_extension(path{".ext"}, path{}) == path{".ext"});
  CHECK(path_replace_extension(path{"asdf.ext"}, path{}) == path{"asdf"});
  CHECK(path_replace_extension(path{"asdf.xyz.ext"}, path{}) == path{"asdf.xyz"});
  CHECK(path_replace_extension(path{"/.ext"}, path{}) == path{"/.ext"});
  CHECK(path_replace_extension(path{"/asdf.ext"}, path{}) == path{"/asdf"});
  CHECK(path_replace_extension(path{"/asdf.xyz.ext"}, path{}) == path{"/asdf.xyz"});
}

} // namespace kdl
