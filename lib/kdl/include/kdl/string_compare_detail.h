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

#ifndef TRENCHBROOM_STRING_COMPARE_DETAIL_H
#define TRENCHBROOM_STRING_COMPARE_DETAIL_H

#include <algorithm> // for std::mismatch, std::sort, std::search, std::equal
#include <string_view>

#include "collection_utils.h"

namespace kdl {
    /**
     * Returns the first position at which the given strings differ. Characters are compared for equality using the
     * given binary predicate.
     *
     * If the given strings are identical, then the length of the strings is returned.
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param s1 the first string
     * @param s2 the second string
     * @param char_equal the binary predicate
     * @return the first position at which the given strings differ
     */
    template <typename CharEqual>
    std::size_t mismatch(const std::string_view& s1, const std::string_view& s2, const CharEqual& char_equal) {
        const auto mis = std::mismatch(std::begin(s1), std::end(s1), std::begin(s2), std::end(s2), char_equal);
        return static_cast<std::size_t>(std::distance(std::begin(s1), mis.first));
    }

    /**
     * Checks whether the first string contains the second string. Characters are compared for equality using the given
     * binary predicate.
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param haystack the string to search in
     * @param needle the string to search for
     * @param char_equal the binary predicate
     * @return true if the first string contains the second string and false otherwise
     */
    template <typename CharEqual>
    bool contains(const std::string_view& haystack, const std::string_view& needle, const CharEqual& char_equal) {
        return std::search(std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle), char_equal) != std::end(haystack);
    }

    /**
     * Checks whether the second string is a prefix of the first string. Characters are compared for equality using the
     * given binary predicate.
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param haystack the string to search in
     * @param needle the string to search for
     * @param char_equal the binary predicate
     * @return true if needle is a prefix of haystack
     */
    template <typename CharEqual>
    bool is_prefix(const std::string_view& haystack, const std::string_view& needle, const CharEqual& char_equal) {
        return std::mismatch(std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle), char_equal).second == std::end(needle);
    }

    /**
     * Checks whether the second string is a suffix of the first string. Characters are compared for equality using the
     * given binary predicate.
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param haystack the string to search in
     * @param needle the string to search for
     * @param char_equal the binary predicate
     * @return true if needle is a suffix of haystack
     */
    template <typename CharEqual>
    bool is_suffix(const std::string_view& haystack, const std::string_view& needle, const CharEqual& char_equal) {
        return std::mismatch(std::rbegin(haystack), std::rend(haystack), std::rbegin(needle), std::rend(needle), char_equal).second == std::rend(needle);
    }

    /**
     * Performs lexicographical comparison of the given collections c1 and c2 using the given comparator. Returns -1 if
     * the first collection is less than the second collection, or +1 in the opposite case, or 0 if both collections are
     * equivalent.
     *
     * @tparam C1 the type of the first collection
     * @tparam C2 the type of the second collection
     * @tparam Compare the comparator type, defaults to std::less<T>, where T both C1::value_type and C2::value_type
     * must be convertible to T
     * @param c1 the first collection
     * @param c2 the second collection
     * @param cmp the comparator to use
     * @return an int indicating the result of the comparison
     */

    /**
     * Performs lexicographical comparison of the given strings s1 and s2 using the given comparator. Returns -1 if
     * the first string is less than the second string, or +1 in the opposite case, or 0 if both strings are equal.
     *
     * @tparam CharCompare the type of the comparator to use for comparing characters
     * @param s1 the first string
     * @param s2 the second string
     * @param char_compare the comparator
     * @return an int indicating the result of the comparison
     */
    template <typename CharCompare>
    int compare(const std::string_view& s1, const std::string_view& s2, const CharCompare& char_compare) {
        return kdl::col_lexicographical_compare(s1, s2, char_compare);
    }

    /**
     * Checks whether the given strings are equal. Characters are compared for equality using the given binary
     * predicate.
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param s1 the first string
     * @param s2 the second string
     * @param char_equal the binary predicate
     * @return true if the given strings are equal and false otherwise
     */
    template <typename CharEqual>
    bool is_equal(const std::string_view& s1, const std::string_view& s2, const CharEqual& char_equal) {
        return std::equal(std::begin(s1), std::end(s1), std::begin(s2), std::end(s2), char_equal);
    }

    /**
     * Checks whether the given string matches the given glob pattern. Characters are compared for equality using the
     * given binary predicate.
     *
     * A glob pattern is a string that has the following special characters:
     * - ? matches any character one time
     * - * matches any character any number of times, including 0
     * - \? matches a literal '?' character
     * - \* matches a literal '*' character
     * - \\ matches a literal '\' character
     *
     * Consider the following examples:
     * - ?o? matches 'god' and 'dog', but not 'dug'
     * - he*o matches 'hello' and 'hero', but not 'hera' nor 'hiro'
     * - wh*\? matches 'what?' and 'why?'
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param s the string to match the pattern against
     * @param p the pattern
     * @param char_equal the binary predicate
     * @return true if the given pattern matches the given string
     */
    template <typename CharEqual>
    bool matches_glob(const std::string_view& s, const std::string_view& p, const CharEqual& char_equal) {
        // If both the string and the pattern are exhausted, we have a successful match.
        if (s.empty() && p.empty()) {
            return true;
        }

        // If the pattern is exhausted but the string is not, there cannot be a match.
        if (p.empty()) {
            return false;
        }

        // Handle escaped characters in pattern.
        if (p[0] == '\\' && p.size() > 1u) {
            // If the string is exhausted, there cannot be a match.
            if (s.empty()) {
                return false;
            }

            // Look ahead at the next character.
            const auto& n = p[1u];
            if (n == '*' || n == '?' || n == '\\') {
                if (s[0] != n) {
                    return false;
                }

                return matches_glob(s.substr(1u), p.substr(2u), char_equal);
            } else {
                return false; // Invalid escape sequence.
            }
        }

        // If the pattern is a star and the string is consumed, continue matching at the next char in the pattern.
        if (p[0] == '*' && s.empty()) {
            return matches_glob(s, p.substr(1u), char_equal);
        }

        // If the pattern is a '?' and the string is consumed, there cannot be a match.
        if (p[0] == '?' && s.empty()) {
            return false;
        }

        // If the pattern is not consumed, and the current char is not a wildcard, and the pattern is not consumed,
        // there cannot be a match.
        if (s.empty()) {
            return false;
        }

        // If the pattern contains '?', or current characters of both strings match, advance both the string and the
        // pattern and continue to match.
        if (p[0] == '?' || char_equal(p[0], s[0])) {
            return matches_glob(s.substr(1u), p.substr(1u), char_equal);
        }

        // If there is * in the pattern, then there are two possibilities
        // a) We consider the current character of the string.
        // b) We ignore the current character of the string.
        if (p[0] == '*') {
            return matches_glob(s, p.substr(1u), char_equal) ||
                   matches_glob(s.substr(1u), p, char_equal);
        }

        // All other possibilities are exhausted, the current characters of the string and the pattern do not match.
        return false;
    }
}

#endif //TRENCHBROOM_STRING_COMPARE_DETAIL_H
