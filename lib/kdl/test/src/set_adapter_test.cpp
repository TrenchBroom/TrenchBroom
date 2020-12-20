/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "kdl/set_adapter.h"

#include <vector>

#include "GTestCompat.h"

#include <catch2/catch.hpp>

namespace kdl {
    TEST_CASE("const_set_adapter_test.wrap_set", "[const_set_adapter_test]") {
        const auto v = std::vector<int>({1, 2, 3, 4});
        const auto s = wrap_set(v);

        ASSERT_EQ(v, s.get_data());
    }


    TEST_CASE("const_set_adapter_test.iterators", "[const_set_adapter_test]") {
        const auto v = std::vector<int>({1, 2, 3, 4});
        const auto s = wrap_set(v);

        auto it = std::begin(s);
        for (std::size_t i = 0u; i < v.size(); ++i) {
            ASSERT_EQ(v[i], *it);
            ++it;
        }
        ASSERT_EQ(std::end(s), it);
    }

    TEST_CASE("const_set_adapter_test.reverse_iterators", "[const_set_adapter_test]") {
        const auto v = std::vector<int>({1, 2, 3, 4});
        const auto s = wrap_set(v);

        auto it = std::rbegin(s);
        for (std::size_t i = 0u; i < v.size(); ++i) {
            ASSERT_EQ(v[v.size() - i - 1u] , *it);
            ++it;
        }
        ASSERT_EQ(std::rend(s), it);
    }

    TEST_CASE("const_set_adapter_test.empty", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        ASSERT_TRUE(wrap_set(v1).empty());

        const auto v2 = std::vector<int>({ 1 });
        ASSERT_FALSE(wrap_set(v2).empty());
    }

    TEST_CASE("const_set_adapter_test.col_total_size", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        ASSERT_EQ(0u, wrap_set(v1).size());

        const auto v2 = std::vector<int>({ 1, 2 });
        ASSERT_EQ(2u, wrap_set(v2).size());
    }

    TEST_CASE("const_set_adapter_test.max_size", "[const_set_adapter_test]") {
        const auto v = std::vector<int>();
        ASSERT_EQ(v.max_size(), wrap_set(v).max_size());
    }

    TEST_CASE("const_set_adapter_test.count", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        ASSERT_EQ(0u, wrap_set(v1).count(1));

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        ASSERT_EQ(0u, wrap_set(v2).count(0));
        ASSERT_EQ(1u, wrap_set(v2).count(1));
        ASSERT_EQ(1u, wrap_set(v2).count(2));
        ASSERT_EQ(1u, wrap_set(v2).count(3));
        ASSERT_EQ(0u, wrap_set(v2).count(4));
    }

    TEST_CASE("const_set_adapter_test.find", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        const auto s1 = wrap_set(v1);
        ASSERT_EQ(std::end(s1), s1.find(1));

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto s2 = wrap_set(v2);
        ASSERT_EQ(std::end(s2), s2.find(0));
        ASSERT_EQ(std::next(std::begin(s2), 0), s2.find(1));
        ASSERT_EQ(std::next(std::begin(s2), 1), s2.find(2));
        ASSERT_EQ(std::next(std::begin(s2), 2), s2.find(3));
        ASSERT_EQ(std::end(s2), s2.find(4));
    }

    TEST_CASE("const_set_adapter_test.equal_range", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        const auto s1 = wrap_set(v1);
        ASSERT_EQ(std::make_pair(std::end(s1), std::end(s1)), s1.equal_range(1));

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto s2 = wrap_set(v2);
        ASSERT_EQ(std::make_pair(std::next(std::begin(s2), 0), std::next(std::begin(s2), 0)), s2.equal_range(0));
        ASSERT_EQ(std::make_pair(std::next(std::begin(s2), 0), std::next(std::begin(s2), 1)), s2.equal_range(1));
        ASSERT_EQ(std::make_pair(std::next(std::begin(s2), 1), std::next(std::begin(s2), 2)), s2.equal_range(2));
        ASSERT_EQ(std::make_pair(std::next(std::begin(s2), 2), std::next(std::begin(s2), 3)), s2.equal_range(3));
        ASSERT_EQ(std::make_pair(std::next(std::begin(s2), 3), std::next(std::begin(s2), 3)), s2.equal_range(4));
    }

    TEST_CASE("const_set_adapter_test.lower_bound", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        const auto s1 = wrap_set(v1);
        ASSERT_EQ(std::end(s1), s1.lower_bound(1));

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto s2 = wrap_set(v2);
        ASSERT_EQ(std::next(std::begin(s2), 0), s2.lower_bound(0));
        ASSERT_EQ(std::next(std::begin(s2), 0), s2.lower_bound(1));
        ASSERT_EQ(std::next(std::begin(s2), 1), s2.lower_bound(2));
        ASSERT_EQ(std::next(std::begin(s2), 2), s2.lower_bound(3));
        ASSERT_EQ(std::next(std::begin(s2), 3), s2.lower_bound(4));
    }

    TEST_CASE("const_set_adapter_test.upper_bound", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        const auto s1 = wrap_set(v1);
        ASSERT_EQ(std::end(s1), s1.upper_bound(1));

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto s2 = wrap_set(v2);
        ASSERT_EQ(std::next(std::begin(s2), 0), s2.upper_bound(0));
        ASSERT_EQ(std::next(std::begin(s2), 1), s2.upper_bound(1));
        ASSERT_EQ(std::next(std::begin(s2), 2), s2.upper_bound(2));
        ASSERT_EQ(std::next(std::begin(s2), 3), s2.upper_bound(3));
        ASSERT_EQ(std::next(std::begin(s2), 3), s2.upper_bound(4));
    }

    TEST_CASE("const_set_adapter_test.capacity", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        const auto s1 = wrap_set(v1);
        ASSERT_EQ(v1.capacity(), s1.capacity());
    }

    TEST_CASE("const_set_adapter_test.get_data", "[const_set_adapter_test]") {
        const auto v1 = std::vector<int>();
        const auto s1 = wrap_set(v1);
        const auto& d = s1.get_data();
        ASSERT_EQ(&v1, &d);
    }

    TEST_CASE("const_set_adapter_test.operator_equal", "[const_set_adapter_test]") {
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) == wrap_set(std::vector<int>({         })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) == wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({    2, 3 })) == wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({       3 })) == wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) == wrap_set(std::vector<int>({    2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) == wrap_set(std::vector<int>({       3 })));
    }

    TEST_CASE("const_set_adapter_test.operator_not_equal", "[const_set_adapter_test]") {
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) != wrap_set(std::vector<int>({         })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) != wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({    2, 3 })) != wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({       3 })) != wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) != wrap_set(std::vector<int>({    2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) != wrap_set(std::vector<int>({       3 })));
    }

    TEST_CASE("const_set_adapter_test.operator_less_than", "[const_set_adapter_test]") {
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) < wrap_set(std::vector<int>({         })));
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) < wrap_set(std::vector<int>({ 1       })));
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) < wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) < wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1       })) < wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2    })) < wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) < wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) < wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) < wrap_set(std::vector<int>({    2, 3 })));
    }

    TEST_CASE("const_set_adapter_test.operator_less_than_or_equal", "[const_set_adapter_test]") {
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) <= wrap_set(std::vector<int>({         })));
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) <= wrap_set(std::vector<int>({ 1       })));
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) <= wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) <= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1       })) <= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2    })) <= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) <= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) <= wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) <= wrap_set(std::vector<int>({    2, 3 })));
    }

    TEST_CASE("const_set_adapter_test.operator_greater_than", "[const_set_adapter_test]") {
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) > wrap_set(std::vector<int>({         })));
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) > wrap_set(std::vector<int>({ 1       })));
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) > wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) > wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1       })) > wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2    })) > wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) > wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) > wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) > wrap_set(std::vector<int>({    2, 3 })));
    }

    TEST_CASE("const_set_adapter_test.operator_greater_than_or_equal", "[const_set_adapter_test]") {
        ASSERT_TRUE (wrap_set(std::vector<int>({         })) >= wrap_set(std::vector<int>({         })));
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) >= wrap_set(std::vector<int>({ 1       })));
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) >= wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_FALSE(wrap_set(std::vector<int>({         })) >= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1       })) >= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2    })) >= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) >= wrap_set(std::vector<int>({ 1, 2, 3 })));
        ASSERT_TRUE (wrap_set(std::vector<int>({ 1, 2, 3 })) >= wrap_set(std::vector<int>({ 1, 2    })));
        ASSERT_FALSE(wrap_set(std::vector<int>({ 1, 2, 3 })) >= wrap_set(std::vector<int>({    2, 3 })));
    }

    TEST_CASE("set_adapter_test.wrap_set", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        auto s = wrap_set(v);

        ASSERT_EQ(v, s.get_data());
    }

    TEST_CASE("set_adapter_test.create_set", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3, 2, 5 });
        auto s = create_set(v);

        ASSERT_EQ(std::vector<int>({1, 2, 3, 5}), s.get_data());
    }

    TEST_CASE("set_adapter_test.operator_assign_with_initializer_list", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3, 2, 5 });
        auto s = create_set(v);

        s = { 5, 6, 7, 6, 3 };
        ASSERT_EQ(std::vector<int>({3, 5, 6, 7 }), s.get_data());
    }

    TEST_CASE("set_adapter_test.clear", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        auto s = wrap_set(v);

        s.clear();
        ASSERT_TRUE(s.empty());
        ASSERT_TRUE(v.empty());
    }

    TEST_CASE("set_adapter_test.insert_with_value", "[set_adapter_test]") {
        auto v = std::vector<int>();
        auto s = wrap_set(v);

        auto [it, success] = s.insert(2);
        ASSERT_TRUE(success);
        ASSERT_EQ(std::begin(s), it);

        std::tie(it, success) = s.insert(2);
        ASSERT_FALSE(success);
        ASSERT_EQ(std::begin(s), it);

        std::tie(it, success) = s.insert(1);
        ASSERT_TRUE(success);
        ASSERT_EQ(std::begin(s), it);

        std::tie(it, success) = s.insert(2);
        ASSERT_FALSE(success);
        ASSERT_EQ(std::next(std::begin(s), 1), it);

        std::tie(it, success) = s.insert(3);
        ASSERT_TRUE(success);
        ASSERT_EQ(std::next(std::begin(s), 2), it);

        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), v);
    }

    TEST_CASE("set_adapter_test.insert_with_value_and_hint", "[set_adapter_test]") {
        auto v = std::vector<int>();
        auto s = wrap_set(v);

        auto it = s.insert(std::end(s), 2);
        ASSERT_EQ(std::begin(s), it);

        it = s.insert(s.upper_bound(1), 1);
        ASSERT_EQ(std::begin(s), it);

        it = s.insert(s.upper_bound(2), 2);
        ASSERT_EQ(std::next(std::begin(s), 1), it);

        it = s.insert(s.begin(), 3); // wrong hint, must still work
        ASSERT_EQ(std::next(std::begin(s), 2), it);

        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), v);
    }

    TEST_CASE("set_adapter_test.insert_with_range", "[set_adapter_test]") {
        auto v = std::vector<int>();
        auto s = wrap_set(v);

        const auto r = std::vector<int>({ 4, 2, 2, 3, 4, 1 });
        s.insert(std::begin(r), std::end(r));

        ASSERT_EQ(std::vector<int>({ 1, 2, 3, 4 }), v);
    }

    TEST_CASE("set_adapter_test.insert_with_range_and_count", "[set_adapter_test]") {
        auto v = std::vector<int>();
        auto s = wrap_set(v);

        const auto r = std::vector<int>({ 4, 2, 2, 3, 4, 1 });
        s.insert(r.size(), std::begin(r), std::end(r));

        ASSERT_EQ(std::vector<int>({ 1, 2, 3, 4 }), v);
    }


    TEST_CASE("set_adapter_test.emplace", "[set_adapter_test]") {
        auto v = std::vector<int>();
        auto s = wrap_set(v);

        // emplace must create the value in any case for comparison, so there is no point in checking whether or
        // not a value was created

        auto [it, success] = s.emplace(2);
        ASSERT_TRUE(success);
        ASSERT_EQ(std::begin(s), it);

        std::tie(it, success) = s.emplace(2);
        ASSERT_FALSE(success);
        ASSERT_EQ(std::begin(s), it);

        std::tie(it, success) = s.emplace(1);
        ASSERT_TRUE(success);
        ASSERT_EQ(std::begin(s), it);

        std::tie(it, success) = s.emplace(2);
        ASSERT_FALSE(success);
        ASSERT_EQ(std::next(std::begin(s), 1), it);

        std::tie(it, success) = s.emplace(3);
        ASSERT_TRUE(success);
        ASSERT_EQ(std::next(std::begin(s), 2), it);

        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), v);
    }

    TEST_CASE("set_adapter_test.emplace_hint", "[set_adapter_test]") {
        auto v = std::vector<int>();
        auto s = wrap_set(v);

        // emplace must create the value in any case for comparison, so there is no point in checking whether or
        // not a value was created

        auto it = s.emplace_hint(std::end(s), 2);
        ASSERT_EQ(std::begin(s), it);

        it = s.emplace_hint(s.upper_bound(1), 1);
        ASSERT_EQ(std::begin(s), it);

        it = s.emplace_hint(s.upper_bound(2), 2);
        ASSERT_EQ(std::next(std::begin(s), 1), it);

        it = s.emplace_hint(s.begin(), 3); // wrong hint, must still work
        ASSERT_EQ(std::next(std::begin(s), 2), it);

        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), v);
    }

    TEST_CASE("set_adapter_test.erase_with_iterator", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        auto s = wrap_set(v);

        s.erase(std::next(std::begin(s), 1));
        ASSERT_EQ(std::vector<int>({ 1, 3 }), v);

        s.erase(std::next(std::begin(s), 1));
        ASSERT_EQ(std::vector<int>({ 1 }), v);

        s.erase(std::next(std::begin(s), 0));
        ASSERT_EQ(std::vector<int>({}), v);
    }

    TEST_CASE("set_adapter_test.erase_with_range", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        auto s = wrap_set(v);

        auto it = s.erase(std::next(std::begin(s), 0), std::next(std::begin(s), 2));
        ASSERT_EQ(std::next(std::begin(s), 0), it);
        ASSERT_EQ(std::vector<int>({ 3 }), v);

        it = s.erase(std::next(std::begin(s), 0), std::next(std::begin(s), 1));
        ASSERT_EQ(std::end(s), it);
        ASSERT_EQ(std::vector<int>({}), v);
    }

    TEST_CASE("set_adapter_test.erase_with_value", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        auto s = wrap_set(v);

        ASSERT_EQ(0u, s.erase(4));
        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), v);

        ASSERT_EQ(1u, s.erase(2));
        ASSERT_EQ(std::vector<int>({ 1, 3 }), v);

        ASSERT_EQ(1u, s.erase(3));
        ASSERT_EQ(std::vector<int>({ 1 }), v);

        ASSERT_EQ(1u, s.erase(1));
        ASSERT_EQ(std::vector<int>({}), v);

        ASSERT_EQ(0u, s.erase(1));
        ASSERT_EQ(std::vector<int>({}), v);
    }

    TEST_CASE("set_adapter_test.swap", "[set_adapter_test]") {
        // swap only works if the underlying collection is stored by value
        auto s = set_adapter<std::vector<int>, std::less<int>>(std::vector<int>({ 1, 2, 3 }));
        auto t = set_adapter<std::vector<int>, std::less<int>>(std::vector<int>({ 4, 5 }));

        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), s.get_data());
        ASSERT_EQ(std::vector<int>({ 4, 5 }), t.get_data());

        using std::swap;
        swap(s, t);

        ASSERT_EQ(std::vector<int>({ 4, 5 }), s.get_data());
        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), t.get_data());
    }

    TEST_CASE("set_adapter_test.release_data", "[set_adapter_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        auto s = wrap_set(v);

        auto w = s.release_data();
        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), w);
        ASSERT_TRUE(s.empty());
        ASSERT_TRUE(v.empty());
    }
}
