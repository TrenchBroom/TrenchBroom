/*
 Copyright 2021 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "kdl/deref_iterator.h"

#include <memory>
#include <vector>

#include <catch2/catch.hpp>

namespace kdl {
TEST_CASE("deref_iterator type members") {
  SECTION("non-const pointer") {
    using I = std::vector<int*>::iterator;
    using D = deref_iterator<I>;
    static_assert(
      std::is_same_v<D::iterator_category, I::iterator_category>, "same iterator category");
    static_assert(std::is_same_v<D::difference_type, I::difference_type>, "same difference type");
    static_assert(std::is_same_v<D::value_type, int>, "correct value type");
    static_assert(std::is_same_v<D::reference, int&>, "correct reference type");
    static_assert(std::is_same_v<D::pointer, int*>, "correct pointer type");
  }

  SECTION("const pointer") {
    using I = std::vector<const int*>::iterator;
    using D = deref_iterator<I>;
    static_assert(
      std::is_same_v<D::iterator_category, I::iterator_category>, "same iterator category");
    static_assert(std::is_same_v<D::difference_type, I::difference_type>, "same difference type");
    static_assert(std::is_same_v<D::value_type, const int>, "correct value type");
    static_assert(std::is_same_v<D::reference, const int&>, "correct reference type");
    static_assert(std::is_same_v<D::pointer, const int*>, "correct pointer type");
  }

  SECTION("unique_ptr") {
    using I = std::vector<std::unique_ptr<int>>::iterator;
    using D = deref_iterator<I>;
    static_assert(
      std::is_same_v<D::iterator_category, I::iterator_category>, "same iterator category");
    static_assert(std::is_same_v<D::difference_type, I::difference_type>, "same difference type");
    static_assert(std::is_same_v<D::value_type, int>, "correct value type");
    static_assert(std::is_same_v<D::reference, int&>, "correct reference type");
    static_assert(std::is_same_v<D::pointer, int*>, "correct pointer type");
  }
}

TEST_CASE("deref_iterator.operator*") {
  int a = 1;
  int b = 2;
  int c = 3;

  SECTION("non const") {
    const auto v = std::vector<int*>{&a, &b, &c};
    auto d = deref_iterator{v.begin()};

    CHECK(*d == 1);
    ++d;
    CHECK(*d == 2);
  }

  SECTION("const") {
    const auto v = std::vector<const int*>{&a, &b, &c};
    auto d = deref_iterator{v.begin()};

    CHECK(*d == 1);
    ++d;
    CHECK(*d == 2);
  }
}

struct A {
  void doIt() {}
  void doItConst() const {}
};

TEST_CASE("deref_iterator.operator->") {
  auto a = A{};
  auto b = A{};
  auto c = A{};

  const auto v = std::vector<const A*>{&a, &b, &c};
  auto d = deref_iterator{v.begin()};

  // must notcompile
  // d->doIt();
  d->doItConst();
}
} // namespace kdl
