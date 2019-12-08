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

#include "string_compare.h"

#include "string_compare_detail.h"

#include <algorithm> // for std::mismatch, std::sort, std::search, std::equal
#include <cctype> // for std::tolower
#include <string_view>

namespace kdl {
    namespace cs {
        bool char_less::operator()(const char& lhs, const char& rhs) const {
            return lhs < rhs;
        }

        bool char_equal::operator()(const char& lhs, const char& rhs) const {
            return lhs == rhs;
        }

        bool string_less::operator()(const std::string_view& lhs, const std::string_view& rhs) const {
            return std::lexicographical_compare(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), char_less());
        }

        bool string_equal::operator()(const std::string_view& lhs, const std::string_view& rhs) const {
            // std::equal can determine size mismatch when given random access iterators
            return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), char_equal());
        }

        std::size_t mismatch(const std::string_view& s1, const std::string_view& s2) {
            return kdl::mismatch(s1, s2, char_equal());
        }

        bool contains(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::contains(haystack, needle, char_equal());
        }

        bool is_prefix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_prefix(haystack, needle, char_equal());
        }

        bool is_suffix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_suffix(haystack, needle, char_equal());
        }

        int compare(const std::string_view& s1, const std::string_view& s2) {
            return kdl::compare(s1, s2, char_less());
        }

        bool is_equal(const std::string_view& s1, const std::string_view& s2) {
            return kdl::is_equal(s1, s2, char_equal());
        }

        bool matches_glob(const std::string_view& s, const std::string_view& p) {
            return kdl::matches_glob(s, p, char_equal());
        }
    }

    namespace ci {
        bool char_less::operator()(const char& lhs, const char& rhs) const {
            return std::tolower(lhs) < std::tolower(rhs);
        }

        bool char_equal::operator()(const char& lhs, const char& rhs) const {
            return std::tolower(lhs) == std::tolower(rhs);
        }

        bool string_less::operator()(const std::string_view& lhs, const std::string_view& rhs) const {
            return std::lexicographical_compare(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), char_less());
        }

        bool string_equal::operator()(const std::string_view& lhs, const std::string_view& rhs) const {
            // std::equal can determine size mismatch when given random access iterators
            return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), char_equal());
        }

        std::size_t mismatch(const std::string_view& s1, const std::string_view& s2) {
            return kdl::mismatch(s1, s2, char_equal());
        }

        bool contains(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::contains(haystack, needle, char_equal());
        }

        bool is_prefix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_prefix(haystack, needle, char_equal());
        }

        bool is_suffix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_suffix(haystack, needle, char_equal());
        }

        int compare(const std::string_view& s1, const std::string_view& s2) {
            return kdl::compare(s1, s2, char_less());
        }

        bool is_equal(const std::string_view& s1, const std::string_view& s2) {
            return kdl::is_equal(s1, s2, char_equal());
        }

        bool matches_glob(const std::string_view& s, const std::string_view& p) {
            return kdl::matches_glob(s, p, char_equal());
        }
    }
}
