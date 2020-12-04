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

#pragma once

#include <algorithm> // for std::mismatch, std::sort, std::search, std::equal
#include <string_view>
#include <vector> // used in str_matches_glob

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
    std::size_t str_mismatch(const std::string_view s1, const std::string_view s2, const CharEqual& char_equal) {
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
    bool str_contains(const std::string_view haystack, const std::string_view needle, const CharEqual& char_equal) {
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
    bool str_is_prefix(const std::string_view haystack, const std::string_view needle, const CharEqual& char_equal) {
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
    bool str_is_suffix(const std::string_view haystack, const std::string_view needle, const CharEqual& char_equal) {
        return std::mismatch(std::rbegin(haystack), std::rend(haystack), std::rbegin(needle), std::rend(needle), char_equal).second == std::rend(needle);
    }

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
    int str_compare(const std::string_view s1, const std::string_view s2, const CharCompare& char_compare) {
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
    bool str_is_equal(const std::string_view s1, const std::string_view s2, const CharEqual& char_equal) {
        return std::equal(std::begin(s1), std::end(s1), std::begin(s2), std::end(s2), char_equal);
    }

    /**
     * Checks whether the given string matches the given glob pattern. Characters are compared for equality using the
     * given binary predicate.
     *
     * A glob pattern is a string that has the following special characters:
     * - ? matches any character one time
     * - * matches any character any number of times, including 0
     * - % matches any digit one time
     * - %* matches any digit any number of times, including 0
     * - \? matches a literal '?' character
     * - \* matches a literal '*' character
     * - \% matches a literal '%' character
     * - \\ matches a literal '\' character
     *
     * Consider the following examples:
     * - ?o? matches 'god' and 'dog', but not 'dug'
     * - he*o matches 'hello' and 'hero', but not 'hera' nor 'hiro'
     * - wh*\? matches 'what?' and 'why?'
     * - wh%% matches 'wh34'
     * - wh%* matches 'wh343433'
     * - wh%* matches 'wh'
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param str the string to match the pattern against
     * @param pattern the pattern
     * @param char_equal the binary predicate
     * @return true if the given pattern matches the given string
     */
    template <typename CharEqual>
    bool str_matches_glob(const std::string_view str, const std::string_view pattern, const CharEqual& char_equal) {
        using match_task = std::pair<std::size_t, std::size_t>;
        std::vector<match_task> match_tasks({{ 0u, 0u }});

        while (!match_tasks.empty()) {
            const auto [s_i, p_i] = match_tasks.back();
            match_tasks.pop_back();

            if (s_i == str.length() && p_i == pattern.length()) {
                // both the string and the pattern are consumed, so we have a match
                return true;
            }

            if (p_i == pattern.length()) {
                // the pattern is consumed but the string is not, so we cannot have a match
                continue;
            }

            if (pattern[p_i] == '\\' && p_i < pattern.length() - 1u) {
                // handle escaped characters in the pattern
                if (s_i < str.length()) {
                    // check the next character in the pattern against the next character in the string
                    const auto& n = pattern[p_i + 1u];
                    if (n == '*' || n == '?' || n == '%' || n == '\\') {
                        if (str[s_i] == n) {
                            // the string matches the escaped character, continue to match
                            match_tasks.emplace_back(s_i + 1u, p_i + 2u);
                        }
                    } else {
                        // invalid escape sequence in pattern
                        return false;
                    }
                }
            } else if (pattern[p_i] == '*') {
                // handle '*' in the pattern
                if (p_i == pattern.length() - 1u) {
                    // the pattern is consumed after the '*', it doesn't matter what the rest of the string looks like
                    return true;
                }

                if (s_i == str.length()) {
                    // the string is consumed, continue matching at the next char in the pattern
                    match_tasks.emplace_back(s_i, p_i + 1u);
                } else {
                    // '*' matches any character
                    // consume the '*' and continue matching at the current character of the string
                    match_tasks.emplace_back(s_i, p_i + 1u);
                    // consume the current character of the string and continue matching at '*'
                    match_tasks.emplace_back(s_i + 1u, p_i);
                }
            } else if (pattern[p_i] == '?') {
                // handle '?' in the pattern
                if (s_i < str.length()) {
                    // '?' matches any character, continue at the next chars in both the pattern and the string
                    match_tasks.emplace_back(s_i + 1u, p_i + 1u);
                }
            } else if (pattern[p_i] == '%') {
                // handle '%' in the pattern
                if (p_i < pattern.length() - 1u && pattern[p_i + 1u] == '*') {
                    // handle "%*" in the pattern
                    // try to continue matching after "%*"
                    match_tasks.emplace_back(s_i, p_i + 2u);
                    if (s_i < str.length() && str[s_i] >= '0' && str[s_i] <= '9') {
                        // try to match more digits
                        match_tasks.emplace_back(s_i + 1u, p_i);
                    }
                } else if (s_i < str.length()) {
                    // handle '%' in the pattern (not followed by '*')
                    if (str[s_i] >= '0' && str[s_i] <= '9') {
                        // continue matching after the digit
                        match_tasks.emplace_back(s_i + 1u, p_i + 1u);
                    }
                }
            } else if (s_i < str.length() && char_equal(pattern[p_i], str[s_i])) {
                // handle a regular character in the pattern
                match_tasks.emplace_back(s_i + 1u, p_i + 1u);
            }
        }

        return false;
    }
}


