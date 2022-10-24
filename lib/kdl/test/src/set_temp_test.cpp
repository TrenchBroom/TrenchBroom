/*
 Copyright 2010-2019 Kristian Duske

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

#include "kdl/set_temp.h"

#include <catch2/catch.hpp>

namespace kdl
{
TEST_CASE("set_temp_test.set_unset", "[set_temp_test]")
{
  int value = 0;
  {
    set_temp s(value, 1);
    CHECK(value == 1);
  }
  CHECK(value == 0);
}

TEST_CASE("set_temp_test.set_unset_bool", "[set_temp_test]")
{
  bool value = false;
  {
    set_temp s(value, true);
    CHECK(value);
  }
  CHECK_FALSE(value);

  {
    set_temp s(value);
    CHECK(value);

    {
      set_temp t(value, false);
      CHECK_FALSE(value);
    }
    CHECK(value);
  }
  CHECK_FALSE(value);
}

TEST_CASE("set_later_test.set", "[set_later_test]")
{
  int value = 0;

  {
    set_later s(value, 1);
    CHECK(value == 0);
  }
  CHECK(value == 1);
}

TEST_CASE("inc_temp.inc_dec", "[inc_temp]")
{
  int value = 0;

  {
    inc_temp i(value);
    CHECK(value == 1);
  }
  CHECK(value == 0);
}

TEST_CASE("dec_temp.dec_inc", "[dec_temp]")
{
  int value = 0;
  {
    dec_temp d(value);
    CHECK(value == -1);
  }
  CHECK(value == 0);
}
} // namespace kdl
