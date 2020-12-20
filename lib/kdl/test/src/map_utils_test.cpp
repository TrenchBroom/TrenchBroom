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

#include "kdl/map_utils.h"

#include <map>
#include <string>
#include <vector>

#include "test_utils.h"

#include "GTestCompat.h"

#include <catch2/catch.hpp>

namespace kdl {
    template<typename K, typename V>
    void test_map_keys(const std::vector<K>& keys, const std::map<K, V>& map) {
        ASSERT_EQ(keys, map_keys(map));
    }

    TEST_CASE("map_utils_test.map_keys", "[map_utils_test]") {
        test_map_keys<int, int>({}, {});
        test_map_keys<int, std::string>({ 1, 2, 3 }, {
            { 1, "one" },
            { 2, "two" },
            { 3, "three " }
        });
    }

    template<typename K, typename V>
    void test_map_values(const std::vector<V>& values, const std::map<K, V>& map) {
        ASSERT_EQ(values, map_values(map));
    }

    TEST_CASE("map_utils_test.map_values", "[map_utils_test]") {
        test_map_values<int, int>({}, {});
        test_map_values<int, std::string>({ "one", "two", "three" }, {
            { 1, "one" },
            { 2, "two" },
            { 3, "three" }
        });
    }

    template<typename K, typename V>
    void test_map_lexicographical_compare(const int exp, const std::map<K, V>& lhs, const std::map<K, V>& rhs) {
        ASSERT_EQ(exp, map_lexicographical_compare(lhs, rhs));
    }

    TEST_CASE("map_utils_test.map_lexicographical_compare", "[map_utils_test]") {
        test_map_lexicographical_compare<int, int>(0, {}, {});
        test_map_lexicographical_compare<int, int>(0, {
            { 1, 2 },
            { 2, 3 }
        }, {
            { 1, 2 },
            { 2, 3 }
        });
        test_map_lexicographical_compare<int, int>(-1, {
            { 1, 2 },
            { 2, 3 }
        }, {
            { 1, 2 },
            { 3, 3 }
        });
        test_map_lexicographical_compare<int, int>(+1, {
            { 1, 2 },
            { 3, 3 }
        }, {
            { 1, 2 },
            { 2, 3 }
        });
        test_map_lexicographical_compare<int, int>(-1, {
            { 1, 2 },
            { 3, 3 }
        }, {
            { 2, 2 },
            { 3, 3 }
        });
        test_map_lexicographical_compare<int, int>(+1, {
            { 1, 2 },
            { 2, 3 },
            { 3, 4 },
        }, {
            { 1, 2 },
            { 2, 3 }
        });
        test_map_lexicographical_compare<int, int>(-1, {
            { 1, 2 },
            { 2, 3 }
        }, {
            { 1, 2 },
            { 2, 3 },
            { 3, 4 },
        });
    }

    template<typename K, typename V>
    void test_map_is_equivalent(const bool exp, const std::map<K, V>& lhs, const std::map<K, V>& rhs) {
        ASSERT_EQ(exp, map_is_equivalent(lhs, rhs));
    }

    TEST_CASE("map_utils_test.map_is_equivalent", "[map_utils_test]") {
        test_map_is_equivalent<int, int>(true, {}, {});
        test_map_is_equivalent<int, int>(true, {
            { 1, 2 },
            { 2, 3 }
        }, {
            { 1, 2 },
            { 2, 3 }
        });
        test_map_is_equivalent<int, int>(false, {
            { 1, 2 },
            { 2, 3 }
        }, {
            { 1, 2 },
            { 3, 3 }
        });
        test_map_is_equivalent<int, int>(false, {
            { 1, 2 },
            { 3, 3 }
        }, {
            { 1, 2 },
            { 2, 3 }
        });
        test_map_is_equivalent<int, int>(false, {
            { 1, 2 },
            { 3, 3 }
        }, {
            { 2, 2 },
            { 3, 3 }
        });
        test_map_is_equivalent<int, int>(false, {
            { 1, 2 },
            { 2, 3 },
            { 3, 4 },
        }, {
            { 1, 2 },
            { 2, 3 }
        });
        test_map_is_equivalent<int, int>(false, {
            { 1, 2 },
            { 2, 3 }
        }, {
            { 1, 2 },
            { 2, 3 },
            { 3, 4 },
        });
    }

    template <typename K, typename V>
    void test_map_find_or_default(const V& exp, const std::map<K, V>& m, const K& key, const V& defaultValue) {
        ASSERT_EQ(exp, map_find_or_default(m, key, defaultValue));
    }

    TEST_CASE("map_utils_test.map_find_or_default", "[map_utils_test]") {
        test_map_find_or_default<int, std::string>("default", {}, 1, "default");
        test_map_find_or_default<int, std::string>("value", {{ 1, "value" }}, 1, "default");
    }

    template <typename K, typename V>
    void test_map_union(const std::map<K, V>& exp, const std::map<K, V>& m1, const std::map<K, V>& m2) {
        ASSERT_EQ(exp, map_union(m1, m2));
    }

    TEST_CASE("map_utils_test.map_union", "[map_utils_test]") {
        test_map_union<int, int>({},                   {},         {});
        test_map_union<int, int>({{ 1, 2 }},           {{ 1, 2 }}, {});
        test_map_union<int, int>({{ 1, 2 }},           {},         {{ 1, 2 }});
        test_map_union<int, int>({{ 1, 2 }},           {{ 1, 2 }}, {{ 1, 2 }});
        test_map_union<int, int>({{ 1, 2 }, { 2, 3 }}, {},         {{ 1, 2 }, { 2, 3 }});
        test_map_union<int, int>({{ 1, 2 }, { 2, 3 }}, {{ 1, 2 }}, {{ 2, 3 }});
        test_map_union<int, int>({{ 1, 3 }},           {{ 1, 2 }}, {{ 1, 3 }});
    }

    template <typename K, typename V>
    void test_map_merge(const std::map<K, std::vector<V>>& exp, const std::map<K, std::vector<V>>& m1, const std::map<K, std::vector<V>>& m2) {
        ASSERT_EQ(exp, map_merge(m1, m2));
    }

    TEST_CASE("map_utils_test.map_merge", "[map_utils_test]") {
        test_map_merge<int, int>({}, {}, {});
        test_map_merge<int, int>({{ 1, { 1, 2 }}}, {{ 1, { 1, 2 }}}, {});
        test_map_merge<int, int>({{ 1, { 1, 2 }}}, {}, {{ 1, { 1, 2 }}});
        test_map_merge<int, int>({{ 1, { 1, 2 }},
                                  { 2, { 3, 4 }}}, {{ 1, { 1, 2 }}}, {{ 2, { 3, 4 }}});
        test_map_merge<int, int>({{ 1, { 1, 2, 3, 4 }}}, {{ 1, { 1, 2 }}}, {{ 1, { 3, 4 }}});
    }

    TEST_CASE("map_utils_test.map_clear_and_delete", "[map_utils_test]") {
        bool d1 = false;
        bool d2 = false;
        bool d3 = false;
        bool d4 = false;

        auto m = std::map<int, std::vector<deletable*>>({
            { 1, {} },
            { 2, { new deletable(d1), new deletable(d2) } },
            { 3, {} },
            { 4, { new deletable(d3)} },
            { 5, { new deletable(d4)} }
        });

        map_clear_and_delete(m);
        ASSERT_TRUE(m.empty());
        ASSERT_TRUE(d1);
        ASSERT_TRUE(d2);
        ASSERT_TRUE(d3);
        ASSERT_TRUE(d4);
    }
}
