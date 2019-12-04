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

#include <gtest/gtest.h>

#include "kdl/vector_utils.h"

#include <set>
#include <vector>

namespace kdl {
    struct base {
        virtual ~base();
    };

    base::~base() = default;

    struct derived : public base {
        ~derived() override;
    };

    derived::~derived() = default;

    TEST(vector_utils_test, cast) {
        auto vd = std::vector<derived*>({ new derived(), new derived() });
        auto vb = cast<base*>(vd);

        ASSERT_EQ(vd.size(), vb.size());
        for (std::size_t i = 0u; i < vd.size(); ++i) {
            ASSERT_EQ(vd[i], vb[i]);
        }

        auto vbd = cast<derived*>(vb);
        ASSERT_EQ(vb.size(), vbd.size());
        for (std::size_t i = 0u; i < vb.size(); ++i) {
            ASSERT_EQ(vb[i], vbd[i]);
        }

        clear_and_delete(vd);
    }

    TEST(vector_utils_test, index_of) {
        using vec = std::vector<int>;

        ASSERT_EQ(0u, index_of(vec({}), 1));
        ASSERT_EQ(1u, index_of(vec({ 2 }), 1));
        ASSERT_EQ(0u, index_of(vec({ 1 }), 1));
        ASSERT_EQ(0u, index_of(vec({ 1, 2, 3 }), 1));
        ASSERT_EQ(1u, index_of(vec({ 1, 2, 3 }), 2));
        ASSERT_EQ(2u, index_of(vec({ 1, 2, 3 }), 3));
        ASSERT_EQ(3u, index_of(vec({ 1, 2, 3 }), 4));
    }

    TEST(vector_utils_test, contains) {
        using vec = std::vector<int>;

        ASSERT_FALSE(contains(vec({}), 1));
        ASSERT_FALSE(contains(vec({ 2 }), 1));
        ASSERT_TRUE(contains(vec({ 1 }), 1));
        ASSERT_TRUE(contains(vec({ 1, 2, 3 }), 1));
        ASSERT_TRUE(contains(vec({ 1, 2, 3 }), 2));
        ASSERT_TRUE(contains(vec({ 1, 2, 3 }), 3));
        ASSERT_FALSE(contains(vec({ 1, 2, 3 }), 4));
    }

    template <typename T, typename... Args>
    void test_append(const std::vector<T>& exp, std::vector<T> into, Args&&... args) {
        append(into, std::forward<Args>(args)...);
        ASSERT_EQ(exp, into);
    }

    TEST(vector_utils_test, append) {
        using vec = std::vector<int>;

        test_append<int>({}, {});
        test_append<int>({}, {}, vec{});
        test_append<int>({ 1 }, { 1 });
        test_append<int>({ 1, 2, 3 }, { 1 }, vec{ 2 }, vec{ 3 });
    }

    TEST(vector_utils_test, concat) {
        using vec = std::vector<int>;

        ASSERT_EQ(vec({}), concat(vec({})));
        ASSERT_EQ(vec({}), concat(vec({}), vec({})));
        ASSERT_EQ(vec({ 1 }), concat(vec({ 1 })));
        ASSERT_EQ(vec({ 1, 2 }), concat(vec({ 1 }), vec({ 2 })));
    }

    template <typename T>
    void test_erase(const std::vector<T>& exp, std::vector<T> from, const T& x) {
        erase(from, x);
        ASSERT_EQ(exp, from);
    }

    TEST(vector_utils_test, erase) {
        test_erase<int>({}, {}, 1);
        test_erase<int>({}, { 1 }, 1);
        test_erase<int>({ 1 }, { 1 }, 2);
        test_erase<int>({ 1, 1 }, { 1, 2, 1 }, 2);
        test_erase<int>({ 2 }, { 1, 2, 1 }, 1);
    }

    template <typename T, typename P>
    void test_erase_if(const std::vector<T>& exp, std::vector<T> from, const P& pred) {
        erase_if(from, pred);
        ASSERT_EQ(exp, from);
    }

    TEST(vector_utils_test, erase_if) {
        const auto pred = [](const int n) { return n % 2 == 0; };

        test_erase_if<int>({}, {}, pred);
        test_erase_if<int>({ 1 }, { 1 }, pred);
        test_erase_if<int>({ 1, 1 }, { 1, 2, 1 }, pred);
        test_erase_if<int>({ 1 }, { 2, 1, 2 }, pred);
    }

    template <typename T>
    void test_erase_at(const std::vector<T>& exp, std::vector<T> from, const std::size_t i) {
        erase_at(from, i);
        ASSERT_EQ(exp, from);
    }

    TEST(vector_utils_test, erase_at) {
        test_erase_at<int>({}, { 1 }, 0u);
        test_erase_at<int>({ 1, 1 }, { 1, 2, 1 }, 1u);
        test_erase_at<int>({ 1, 2 }, { 2, 1, 2 }, 0u);
    }

    template <typename T>
    void test_erase_all(const std::vector<T>& exp, std::vector<T> from, const std::vector<T>& which) {
        erase_all(from, which);
        ASSERT_EQ(exp, from);
    }

    TEST(vector_utils_test, erase_all) {
        test_erase_all<int>({}, {}, {});
        test_erase_all<int>({ 1, 2, 3 }, { 1, 2, 3 }, {});
        test_erase_all<int>({ 2, 3 }, { 1, 2, 3 }, { 1 });
        test_erase_all<int>({ 3 }, { 1, 2, 3 }, { 1, 2 });
        test_erase_all<int>({}, { 1, 2, 3 }, { 1, 2, 3 });
        test_erase_all<int>({ 1, 3 }, { 1, 2, 2, 3 }, { 2 });
    }

    TEST(vector_utils_test, sort) {
        // just a smoke test since we're just forwarding to std::sort
        auto v = std::vector<int>({ 2, 3, 2, 1 });
        sort(v);
        ASSERT_EQ(std::vector<int>({ 1, 2, 2, 3 }), v);
    }

    TEST(vector_utils_test, sort_and_make_unique) {
        // just a smoke test since we're just forwarding to std::sort and std::unique
        auto v = std::vector<int>({ 2, 3, 2, 1 });
        sort_and_make_unique(v);
        ASSERT_EQ(std::vector<int>({ 1, 2, 3 }), v);
    }

    TEST(vector_utils_test, transform) {
        ASSERT_EQ(std::vector<int>({}), transform(std::vector<int>({}), [](auto x) { return x + 10; }));
        ASSERT_EQ(std::vector<int>({ 11, 12, 13 }), transform(std::vector<int>({ 1, 2, 3 }), [](auto x) { return x + 10; }));
        ASSERT_EQ(std::vector<double>({ 11.0, 12.0, 13.0 }), transform(std::vector<int>({ 1, 2, 3 }), [](auto x) { return x + 10.0; }));
    }

    TEST(vector_utils_test, set_difference) {
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

    TEST(vector_utils_test, set_union) {
        using vec = std::vector<int>;
        using set = std::set<int>;
        ASSERT_EQ(vec({}), set_union(set({}), set({})));
        ASSERT_EQ(vec({ 1, 2 }), set_union(set({}), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2 }), set_union(set({ 1 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2 }), set_union(set({ 1, 2 }), set({ 1, 2 })));
        ASSERT_EQ(vec({ 1, 2, 3, 4}), set_union(set({ 1, 2 }), set({ 1, 2, 3, 4 })));
        ASSERT_EQ(vec({ 1, 2, 3, 4 }), set_union(set({ 1, 2, 3 }), set({ 2, 4 })));
    }

    TEST(vector_utils_test, set_intersection) {
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

    TEST(vector_utils_test, clear_to_zero) {
        auto v = std::vector<int>({ 1, 2, 3 });
        ASSERT_LT(0u, v.capacity());

        clear_to_zero(v);
        ASSERT_TRUE(v.empty());
        ASSERT_EQ(0u, v.capacity());
    }

    struct deletable {
        bool& deleted;

        deletable(bool& i_deleted) : deleted(i_deleted) { deleted = false; }
        ~deletable() { deleted = true; }
    };

    TEST(vector_utils_test, clear_and_delete) {
        bool d1, d2, d3;
        auto v = std::vector<deletable*>({
            new deletable(d1),
            new deletable(d2),
            new deletable(d3),
        });

        clear_and_delete(v);
        ASSERT_TRUE(v.empty());
        ASSERT_TRUE(d1);
        ASSERT_TRUE(d2);
        ASSERT_TRUE(d3);
    }
}
