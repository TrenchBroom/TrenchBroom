/*
 Copyright (C) 2026 Kristian Duske

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

#include "kd/memory_utils.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

struct Base
{
  virtual ~Base() = default;
};

struct Derived : public Base
{
  int value = 0;
};

} // namespace

TEST_CASE("memory_utils")
{
  SECTION("static_pointer_cast")
  {
    SECTION("null pointer")
    {
      CHECK(static_pointer_cast<Derived>(std::unique_ptr<Base>{}) == nullptr);
    }

    SECTION("non-null pointer")
    {
      auto derived = std::make_unique<Derived>();
      derived->value = 7;
      auto* pointee = derived.get();

      auto base = std::unique_ptr<Base>{std::move(derived)};
      auto result = static_pointer_cast<Derived>(std::move(base));

      CHECK(base == nullptr);
      CHECK(result.get() == pointee);
      CHECK(result->value == 7);
    }
  }
}

} // namespace kdl
