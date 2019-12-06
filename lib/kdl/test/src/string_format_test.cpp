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

#include <kdl/string_format.h>

namespace kdl {
    TEST(string_format_test, str_select) {
        ASSERT_EQ("yes", str_select(true, "yes", "no"));
        ASSERT_EQ("no", str_select(false, "yes", "no"));
    }

    TEST(string_format_test, str_plural) {
        ASSERT_EQ("many", str_plural(0, "one", "many"));
        ASSERT_EQ("one", str_plural(1, "one", "many"));
        ASSERT_EQ("many", str_plural(2, "one", "many"));
    }

    TEST(string_format_test, str_plural_with_prefix_suffix) {
        ASSERT_EQ("prefix many suffix", str_plural("prefix ", 0, "one", "many", " suffix"));
        ASSERT_EQ("prefix one suffix", str_plural("prefix ", 1, "one", "many", " suffix"));
        ASSERT_EQ("prefix many suffix", str_plural("prefix ", 2, "one", "many", " suffix"));
    }

    TEST(string_format_test, str_escape) {
        ASSERT_EQ("", str_escape("", ""));
        ASSERT_EQ("", str_escape("", ";"));
        ASSERT_EQ("asdf", str_escape("asdf", ""));
        ASSERT_EQ("\\\\", str_escape("\\", ""));

        ASSERT_EQ("c:\\\\blah\\\\fasel\\\\test.jpg", str_escape("c:\\blah\\fasel\\test.jpg", "\\"));
        ASSERT_EQ("c\\:\\\\blah\\\\fasel\\\\test\\.jpg", str_escape("c:\\blah\\fasel\\test.jpg", "\\:."));
        ASSERT_EQ("\\asdf", str_escape("asdf", "a"));
        ASSERT_EQ("asd\\f", str_escape("asdf", "f"));
    }

    TEST(string_format_test, str_escape_if_necessary) {
        ASSERT_EQ("this \\# should be escaped, but not this \\#; this \\\\\\# however, should!", str_escape_if_necessary("this # should be escaped, but not this \\#; this \\\\# however, should!", "#"));
    }

    TEST(string_format_test, str_unescape) {
        ASSERT_EQ("", str_unescape("", ""));
        ASSERT_EQ("", str_unescape("", ";"));
        ASSERT_EQ("asdf", str_unescape("asdf", ""));

        ASSERT_EQ("c:\\blah\\fasel\\test.jpg", str_unescape("c:\\\\blah\\\\fasel\\\\test.jpg", "\\"));
        ASSERT_EQ("c:\\blah\\fasel\\test.jpg", str_unescape("c\\:\\\\blah\\\\fasel\\\\test\\.jpg", "\\:."));
        ASSERT_EQ("asdf", str_unescape("\\asdf", "a"));
        ASSERT_EQ("asdf", str_unescape("asd\\f", "f"));
        ASSERT_EQ("asdf", str_unescape("\\asdf", "a"));
        ASSERT_EQ("asdf\\", str_unescape("asdf\\", ""));
        ASSERT_EQ("asdf\\", str_unescape("asdf\\\\", ""));
        ASSERT_EQ("asdf\\\\", str_unescape("asdf\\\\\\\\", ""));
    }
}
