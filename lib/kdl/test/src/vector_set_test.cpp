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

#include "kdl/vector_set.h"

namespace kdl {
    using vec = std::vector<int>;
    using vset = vector_set<int>;

    vset create_vset_from_range(const vec& v);
    vset create_vset_from_range(const vset::size_type capacity, const vec& v);

    vset create_vset_from_list(std::initializer_list<int> l);
    vset create_vset_from_list(const vset::size_type capacity, std::initializer_list<int> l);

    void ASSERT_VSET_EQ(const vec& expected, const vset& actual);
    void ASSERT_VSET_EQ(const vset::size_type capacity, const vec& expected, const vset& actual);

    TEST(vector_set_test, constructor_default) {
        vset s;
        ASSERT_TRUE(s.empty());
        ASSERT_EQ(0u, s.size());
    }

    TEST(vector_set_test, constructor_default_with_capacity) {
        vset s(7u);
        ASSERT_TRUE(s.empty());
        ASSERT_EQ(0u, s.size());
        ASSERT_EQ(7u, s.capacity());
    }

    TEST(vector_set_test, constructor_with_range) {
        ASSERT_VSET_EQ({},          create_vset_from_range({}));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_range({ 1 }));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_range({ 1, 1 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_range({ 1, 2 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_range({ 2, 1 }));
        ASSERT_VSET_EQ({ 1, 2, 3 }, create_vset_from_range({ 2, 1, 3, 1, 2 }));
    }

    TEST(vector_set_test, constructor_with_range_and_capacity) {
        ASSERT_VSET_EQ(10u, {},          create_vset_from_range(10u, {}));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_range(10u, { 1 }));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_range(10u, { 1, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_range(10u, { 1, 2 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_range(10u, { 2, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2, 3 }, create_vset_from_range(10u, { 2, 1, 3, 1, 2 }));
    }

    TEST(vector_set_test, constructor_with_initializer_list) {
        ASSERT_VSET_EQ({},          create_vset_from_list({}));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_list({ 1 }));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_list({ 1, 1 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_list({ 1, 2 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_list({ 2, 1 }));
        ASSERT_VSET_EQ({ 1, 2, 3 }, create_vset_from_list({ 2, 1, 3, 1, 2 }));
    }

    TEST(vector_set_test, constructor_with_initializer_list_and_capacity) {
        ASSERT_VSET_EQ(10u, {},          create_vset_from_list(10u, {}));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_list(10u, { 1 }));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_list(10u, { 1, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_list(10u, { 1, 2 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_list(10u, { 2, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2, 3 }, create_vset_from_list(10u, { 2, 1, 3, 1, 2 }));
    }

    TEST(vector_set_test, assignment_from_initializer_list) {
        ASSERT_VSET_EQ({},          vset() = {});
        ASSERT_VSET_EQ({ 1 },       vset() = { 1 });
        ASSERT_VSET_EQ({ 1 },       vset() = { 1, 1 });
        ASSERT_VSET_EQ({ 1, 2 },    vset() = { 1, 2 });
        ASSERT_VSET_EQ({ 1, 2 },    vset() = { 2, 1 });
        ASSERT_VSET_EQ({ 1, 2, 3 }, vset() = { 2, 1, 3, 1, 2 });

        ASSERT_VSET_EQ({},          vset({ 7, 8, 9 }) = {});
        ASSERT_VSET_EQ({ 1 },       vset({ 7, 8, 9 }) = { 1 });
        ASSERT_VSET_EQ({ 1 },       vset({ 7, 8, 9 }) = { 1, 1 });
        ASSERT_VSET_EQ({ 1, 2 },    vset({ 7, 8, 9 }) = { 1, 2 });
        ASSERT_VSET_EQ({ 1, 2 },    vset({ 7, 8, 9 }) = { 2, 1 });
        ASSERT_VSET_EQ({ 1, 2, 3 }, vset({ 7, 8, 9 }) = { 2, 1, 3, 1, 2 });
    }

    TEST(vector_set_test, iterators) {
        vset v1;
        auto b = v1.begin();
        auto e = v1.end();
        ASSERT_EQ(b, e);

        vset v2({ 1 });
        b = v2.begin();
        e = v2.end();
        ASSERT_NE(b, e);
        ASSERT_EQ(1, *b);
        ASSERT_EQ(++b, e);

        vset v3({ 1, 2 });
        b = v3.begin();
        e = v3.end();
        ASSERT_NE(b, e);
        ASSERT_EQ(1, *b);
        ASSERT_NE(++b, e);
        ASSERT_EQ(2, *b);
        ASSERT_EQ(++b, e);
    }


    TEST(vector_set_test, reverse_iterators) {
        vset v1;
        auto b = v1.rbegin();
        auto e = v1.rend();
        ASSERT_EQ(b, e);

        vset v2({ 1 });
        b = v2.rbegin();
        e = v2.rend();
        ASSERT_NE(b, e);
        ASSERT_EQ(1, *b);
        ASSERT_EQ(++b, e);

        vset v3({ 1, 2 });
        b = v3.rbegin();
        e = v3.rend();
        ASSERT_NE(b, e);
        ASSERT_EQ(2, *b);
        ASSERT_NE(++b, e);
        ASSERT_EQ(1, *b);
        ASSERT_EQ(++b, e);
    }

    TEST(vector_set_test, empty) {
        ASSERT_TRUE(vec({}).empty());
        ASSERT_FALSE(vec({ 1 }).empty());
    }

    TEST(vector_set_test, size) {
        ASSERT_EQ(0u, vset({}).size());
        ASSERT_EQ(1u, vset({ 1 }).size());
        ASSERT_EQ(1u, vset({ 1, 1 }).size());
        ASSERT_EQ(2u, vset({ 1, 3 }).size());
    }

    TEST(vector_set_test, clear) {
        vset v1({});
        v1.clear();
        ASSERT_VSET_EQ({}, v1);

        vset v2({ 1 });
        v2.clear();
        ASSERT_VSET_EQ({}, v2);

        vset v3({ 1, 2 });
        v3.clear();
        ASSERT_VSET_EQ({}, v3);
    }

    vset create_vset_from_range(const vec& v) {
        return vset(std::begin(v), std::end(v));
    }

    vset create_vset_from_range(const vset::size_type capacity, const vec& v) {
        return vset(capacity, std::begin(v), std::end(v));
    }

    vset create_vset_from_list(std::initializer_list<int> l) {
        return vset(std::move(l));
    }

    vset create_vset_from_list(const vset::size_type capacity, std::initializer_list<int> l) {
        return vset(capacity, std::move(l));
    }

    void ASSERT_VSET_EQ(const vec& expected, const vset& actual) {
        ASSERT_EQ(create_vset_from_range(expected), actual);
    }

    void ASSERT_VSET_EQ(const vset::size_type capacity, const vec& expected, const vset& actual) {
        ASSERT_EQ(capacity, actual.capacity());
        ASSERT_EQ(create_vset_from_range(expected), actual);
    }
}
