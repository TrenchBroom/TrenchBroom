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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "test_utils.h"

#include "kdl/vector_utils.h"

#include <set>
#include <vector>

namespace kdl {
    TEST_CASE("vector_utils_test.vec_at", "[vector_utils_test]") {
        const auto cv = std::vector<int>({ 1, 2, 3 });
        for (std::size_t i = 0u; i < cv.size(); ++i) {
            ASSERT_EQ(cv[i], vec_at(cv, static_cast<int>(i)));
        }

        auto mv = std::vector<int>({ 1, 2, 3 });
        vec_at(mv, 2) = 4;
        ASSERT_EQ(4, mv[2]);
    }

    TEST_CASE("vector_utils_test.vec_pop_back", "[vector_utils_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        ASSERT_EQ(3, vec_pop_back(v));
        ASSERT_EQ(std::vector<int>({ 1, 2 }), v);
        ASSERT_EQ(2, vec_pop_back(v));
        ASSERT_EQ(std::vector<int>({ 1 }), v);
        ASSERT_EQ(1, vec_pop_back(v));
        ASSERT_EQ(std::vector<int>({}), v);
    }

    struct base {
        virtual ~base();
    };

    base::~base() = default;

    struct derived : public base {
        ~derived() override;
    };

    derived::~derived() = default;

    TEST_CASE("vector_utils_test.vec_element_cast", "[vector_utils_test]") {
        auto vd = std::vector<derived*>({ new derived(), new derived() });
        auto vb = vec_element_cast<base*>(vd);

        ASSERT_EQ(vd.size(), vb.size());
        for (std::size_t i = 0u; i < vd.size(); ++i) {
            ASSERT_EQ(vd[i], vb[i]);
        }

        auto vbd = vec_element_cast<derived*>(vb);
        ASSERT_EQ(vb.size(), vbd.size());
        for (std::size_t i = 0u; i < vb.size(); ++i) {
            ASSERT_EQ(vb[i], vbd[i]);
        }

        vec_clear_and_delete(vd);
    }

    TEST_CASE("vector_utils_test.vec_index_of", "[vector_utils_test]") {
        using vec = std::vector<int>;

        ASSERT_EQ(0u, vec_index_of(vec({}), 1));
        ASSERT_EQ(1u, vec_index_of(vec({ 2 }), 1));
        ASSERT_EQ(0u, vec_index_of(vec({ 1 }), 1));
        ASSERT_EQ(0u, vec_index_of(vec({ 1, 2, 3 }), 1));
        ASSERT_EQ(1u, vec_index_of(vec({ 1, 2, 3 }), 2));
        ASSERT_EQ(2u, vec_index_of(vec({ 1, 2, 3 }), 3));
        ASSERT_EQ(3u, vec_index_of(vec({ 1, 2, 3 }), 4));
    }

    TEST_CASE("vector_utils_test.vec_contains", "[vector_utils_test]") {
        using vec = std::vector<int>;

        ASSERT_FALSE(vec_contains(vec({}), 1));
        ASSERT_FALSE(vec_contains(vec({ 2 }), 1));
        ASSERT_TRUE(vec_contains(vec({ 1 }), 1));
        ASSERT_TRUE(vec_contains(vec({ 1, 2, 3 }), 1));
        ASSERT_TRUE(vec_contains(vec({ 1, 2, 3 }), 2));
        ASSERT_TRUE(vec_contains(vec({ 1, 2, 3 }), 3));
        ASSERT_FALSE(vec_contains(vec({ 1, 2, 3 }), 4));
    }

    template <typename T, typename... Args>
    void test_append(const std::vector<T>& exp, std::vector<T> into, Args&&... args) {
        vec_append(into, std::forward<Args>(args)...);
        ASSERT_EQ(exp, into);
    }

    TEST_CASE("vector_utils_test.vec_append", "[vector_utils_test]") {
        using vec = std::vector<int>;

        test_append<int>({}, {});
        test_append<int>({}, {}, vec{});
        test_append<int>({ 1 }, { 1 });
        test_append<int>({ 1, 2, 3 }, { 1 }, vec{ 2 }, vec{ 3 });
    }

    TEST_CASE("vector_utils_test.vec_concat", "[vector_utils_test]") {
        using vec = std::vector<int>;

        ASSERT_EQ(vec({}), vec_concat(vec({})));
        ASSERT_EQ(vec({}), vec_concat(vec({}), vec({})));
        ASSERT_EQ(vec({ 1 }), vec_concat(vec({ 1 })));
        ASSERT_EQ(vec({ 1, 2 }), vec_concat(vec({ 1 }), vec({ 2 })));
    }

    TEST_CASE("vector_utils_test.vec_slice", "[vector_utils_test]") {
        using vec = std::vector<int>;

        ASSERT_EQ(vec({}), vec_slice(vec({}), 0, 0));
        ASSERT_EQ(vec({}), vec_slice(vec({ 1, 2, 3 }), 0, 0));
        ASSERT_EQ(vec({}), vec_slice(vec({ 1, 2, 3 }), 1, 0));
        ASSERT_EQ(vec({}), vec_slice(vec({ 1, 2, 3 }), 2, 0));
        ASSERT_EQ(vec({}), vec_slice(vec({ 1, 2, 3 }), 3, 0));
        ASSERT_EQ(vec({ 1 }), vec_slice(vec({ 1, 2, 3 }), 0, 1));
        ASSERT_EQ(vec({ 2 }), vec_slice(vec({ 1, 2, 3 }), 1, 1));
        ASSERT_EQ(vec({ 3 }), vec_slice(vec({ 1, 2, 3 }), 2, 1));
        ASSERT_EQ(vec({ 1, 2 }), vec_slice(vec({ 1, 2, 3 }), 0, 2));
        ASSERT_EQ(vec({ 2, 3 }), vec_slice(vec({ 1, 2, 3 }), 1, 2));
        ASSERT_EQ(vec({ 1, 2, 3 }), vec_slice(vec({ 1, 2, 3 }), 0, 3));
    }

    TEST_CASE("vector_utils_test.vec_slice_prefix", "[vector_utils_test]") {
        using vec = std::vector<int>;

        ASSERT_EQ(vec({}), vec_slice_prefix(vec({}), 0));
        ASSERT_EQ(vec({ 1 }), vec_slice_prefix(vec({ 1 }), 1));
        ASSERT_EQ(vec({}), vec_slice_prefix(vec({ 1 }), 0));
        ASSERT_EQ(vec({ 1, 2, 3 }), vec_slice_prefix(vec({ 1, 2, 3 }), 3));
        ASSERT_EQ(vec({ 1, 2 }), vec_slice_prefix(vec({ 1, 2, 3 }), 2));
        ASSERT_EQ(vec({ 1 }), vec_slice_prefix(vec({ 1, 2, 3 }), 1));
        ASSERT_EQ(vec({}), vec_slice_prefix(vec({ 1, 2, 3 }), 0));
    }

    TEST_CASE("vector_utils_test.vec_slice_suffix", "[vector_utils_test]") {
        using vec = std::vector<int>;

        ASSERT_EQ(vec({}), vec_slice_suffix(vec({}), 0));
        ASSERT_EQ(vec({}), vec_slice_suffix(vec({ 1 }), 0));
        ASSERT_EQ(vec({ 1 }), vec_slice_suffix(vec({ 1 }), 1));
        ASSERT_EQ(vec({}), vec_slice_suffix(vec({ 1, 2, 3 }), 0));
        ASSERT_EQ(vec({ 3 }), vec_slice_suffix(vec({ 1, 2, 3 }), 1));
        ASSERT_EQ(vec({ 2, 3 }), vec_slice_suffix(vec({ 1, 2, 3 }), 2));
        ASSERT_EQ(vec({ 1, 2, 3 }), vec_slice_suffix(vec({ 1, 2, 3 }), 3));
    }

    template <typename T>
    void test_erase(const std::vector<T>& exp, std::vector<T> from, const T& x) {
        vec_erase(from, x);
        ASSERT_EQ(exp, from);
    }

    TEST_CASE("vector_utils_test.vec_erase", "[vector_utils_test]") {
        test_erase<int>({}, {}, 1);
        test_erase<int>({}, { 1 }, 1);
        test_erase<int>({ 1 }, { 1 }, 2);
        test_erase<int>({ 1, 1 }, { 1, 2, 1 }, 2);
        test_erase<int>({ 2 }, { 1, 2, 1 }, 1);
    }

    template <typename T, typename P>
    void test_erase_if(const std::vector<T>& exp, std::vector<T> from, const P& pred) {
        vec_erase_if(from, pred);
        ASSERT_EQ(exp, from);
    }

    TEST_CASE("vector_utils_test.vec_erase_if", "[vector_utils_test]") {
        const auto pred = [](const int n) { return n % 2 == 0; };

        test_erase_if<int>({}, {}, pred);
        test_erase_if<int>({ 1 }, { 1 }, pred);
        test_erase_if<int>({ 1, 1 }, { 1, 2, 1 }, pred);
        test_erase_if<int>({ 1 }, { 2, 1, 2 }, pred);
    }

    template <typename T>
    void test_erase_at(const std::vector<T>& exp, std::vector<T> from, const std::size_t i) {
        vec_erase_at(from, i);
        ASSERT_EQ(exp, from);
    }

    TEST_CASE("vector_utils_test.vec_erase_at", "[vector_utils_test]") {
        test_erase_at<int>({}, { 1 }, 0u);
        test_erase_at<int>({ 1, 1 }, { 1, 2, 1 }, 1u);
        test_erase_at<int>({ 1, 2 }, { 2, 1, 2 }, 0u);
    }

    template <typename T>
    void test_erase_all(const std::vector<T>& exp, std::vector<T> from, const std::vector<T>& which) {
        vec_erase_all(from, which);
        ASSERT_EQ(exp, from);
    }

    TEST_CASE("vector_utils_test.vec_erase_all", "[vector_utils_test]") {
        test_erase_all<int>({}, {}, {});
        test_erase_all<int>({ 1, 2, 3 }, { 1, 2, 3 }, {});
        test_erase_all<int>({ 2, 3 }, { 1, 2, 3 }, { 1 });
        test_erase_all<int>({ 3 }, { 1, 2, 3 }, { 1, 2 });
        test_erase_all<int>({}, { 1, 2, 3 }, { 1, 2, 3 });
        test_erase_all<int>({ 1, 3 }, { 1, 2, 2, 3 }, { 2 });
    }

    TEST_CASE("vector_utils_test.vec_sort", "[vector_utils_test]") {
        // just a smoke test since we're just forwarding to std::sort
        auto v = std::vector<int>({ 2, 3, 2, 1 });
        vec_sort(v);
        ASSERT_EQ(std::vector<int>({ 1, 2, 2, 3 }), v);
    }

    TEST_CASE("vector_utils_test.vec_sort_and_remove_duplicates", "[vector_utils_test]") {
        // just a smoke test since we're just forwarding to std::sort and std::unique
        auto v = std::vector<int>({ 2, 3, 2, 1 });
        vec_sort_and_remove_duplicates(v);
        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), v);
    }

    TEST_CASE("vector_utils_test.vec_filter", "[vector_utils_test]") {
        ASSERT_EQ(std::vector<int>({}), vec_filter(std::vector<int>({}), [](auto) { return false; }));
        ASSERT_EQ(std::vector<int>({}), vec_filter(std::vector<int>({ 1, 2, 3 }), [](auto) { return false; }));
        ASSERT_EQ(std::vector<int>({ 1, 2, 3}), vec_filter(std::vector<int>({ 1, 2, 3 }), [](auto) { return true; }));
        ASSERT_EQ(std::vector<int>({ 2 }), vec_filter(std::vector<int>({ 1, 2, 3 }), [](auto x) { return x % 2 == 0; }));
    }

    struct MoveOnly {
        MoveOnly() = default;

        MoveOnly(const MoveOnly& other) = delete;
        MoveOnly& operator=(const MoveOnly& other) = delete;

        MoveOnly(MoveOnly&& other) noexcept = default;
        MoveOnly& operator=(MoveOnly&& other) = default;
    };

    TEST_CASE("vector_utils_test.vec_filter_rvalue", "[vector_utils_test]") {
        auto vec = std::vector<MoveOnly>{};
        vec.emplace_back();
        vec.emplace_back();
        ASSERT_EQ(2u, vec_filter(std::move(vec), [](const auto&) { return true; }).size());
    }
    
    TEST_CASE("vector_utils_test.vec_transform", "[vector_utils_test]") {
        ASSERT_EQ(std::vector<int>({}), vec_transform(std::vector<int>({}), [](auto x) { return x + 10; }));
        ASSERT_EQ(std::vector<int>({ 11, 12, 13 }),
            vec_transform(std::vector<int>({ 1, 2, 3 }), [](auto x) { return x + 10; }));
        ASSERT_EQ(std::vector<double>({ 11.0, 12.0, 13.0 }),
            vec_transform(std::vector<int>({ 1, 2, 3 }), [](auto x) { return x + 10.0; }));
    }

    struct X {};

    TEST_CASE("vector_utils_test.vec_transform_rvalue", "[vector_utils_test]") {
        ASSERT_EQ(1u, vec_transform(std::vector<X>{ X() }, [](X&& x) { return std::move(x); }).size());
    }

    TEST_CASE("vector_utils_test.set_difference", "[vector_utils_test]") {
        using vec = std::vector<int>;
        using set = std::set<int>;
        ASSERT_EQ(vec({}), set_difference(set({}), set({})));
        ASSERT_EQ(vec({}), set_difference(set({}), set({ 1, 2 })));
        ASSERT_EQ(vec({}), set_difference(set({ 1 }), set({ 1, 2 })));
        ASSERT_EQ(vec({}), set_difference(set({ 1, 2 }), set({ 1, 2 })));
        ASSERT_EQ(vec({}), set_difference(set({ 1, 2 }), set({ 1, 2, 3, 4 })));
        ASSERT_EQ(vec({ 3 }), set_difference(set({ 1, 2, 3 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 3 }), set_difference(set({ 1, 2, 3 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 3 }), set_difference(set({ 1, 2, 3 }), set({ 2 })));
    }

    TEST_CASE("vector_utils_test.set_union", "[vector_utils_test]") {
        using vec = std::vector<int>;
        using set = std::set<int>;
        ASSERT_EQ(vec({}), set_union(set({}), set({})));
        ASSERT_EQ(vec({ 1, 2 }), set_union(set({}), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2 }), set_union(set({ 1 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2 }), set_union(set({ 1, 2 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2, 3, 4}), set_union(set({ 1, 2 }), set({ 1, 2, 3, 4 })));
        ASSERT_EQ(vec({ 1, 2, 3, 4 }), set_union(set({ 1, 2, 3 }), set({ 2, 4 })));
    }

    TEST_CASE("vector_utils_test.set_intersection", "[vector_utils_test]") {
        using vec = std::vector<int>;
        using set = std::set<int>;
        ASSERT_EQ(vec({}), set_intersection(set({}), set({})));
        ASSERT_EQ(vec({}), set_intersection(set({}), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1 }), set_intersection(set({ 1 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2 }), set_intersection(set({ 1, 2 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2 }), set_intersection(set({ 1, 2 }), set({ 1, 2, 3, 4 })));
        ASSERT_EQ(vec({ 1, 2 }), set_intersection(set({ 1, 2, 3 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 3 }), set_intersection(set({ 1, 2, 3, 4 }), set({ 1, 3, 5 })));
    }

    TEST_CASE("vector_utils_test.vec_clear_to_zero", "[vector_utils_test]") {
        auto v = std::vector<int>({ 1, 2, 3 });
        ASSERT_LT(0u, v.capacity());

        vec_clear_to_zero(v);
        ASSERT_TRUE(v.empty());
        ASSERT_EQ(0u, v.capacity());
    }

    TEST_CASE("vector_utils_test.vec_clear_and_delete", "[vector_utils_test]") {
        bool d1 = false;
        bool d2 = false;
        bool d3 = false;
        auto v = std::vector<deletable*>({
            new deletable(d1),
            new deletable(d2),
            new deletable(d3),
        });

        vec_clear_and_delete(v);
        ASSERT_TRUE(v.empty());
        ASSERT_TRUE(d1);
        ASSERT_TRUE(d2);
        ASSERT_TRUE(d3);
    }
}
