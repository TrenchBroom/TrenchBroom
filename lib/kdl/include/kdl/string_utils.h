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

#ifndef TRENCHBROOM_STRING_UTILS_H
#define TRENCHBROOM_STRING_UTILS_H

#include <algorithm> // for std::mismatch, std::sort, std::search, std::equal
#include <string_view>

#include <kdl/collection_utils.h>

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
        return kdl::lexicographical_compare(s1, s2, char_compare);
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
     * Checks whether the given string in range [s_cur, s_end) matches the glob pattern in range [p_cur, p_end).
     * Characters are compared for equality using the given binary predicate.
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
     * @tparam I the iterator type
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param s_cur beginning of the string to match the pattern against
     * @param s_end end of the string to match the pattern against (past-the-end iterator)
     * @param p_cur beginning of the pattern
     * @param p_end end of the pattern (past-the-end iterator)
     * @param char_equal the binary predicate
     * @return true if the given pattern matches the given string
     */
    template <typename I, typename CharEqual>
    bool matches_glob(I s_cur, I s_end, I p_cur, I p_end, const CharEqual& char_equal) {
        if (s_cur == s_end && p_cur == p_end) {
            return true;
        }

        if (p_cur == p_end) {
            return false;
        }

        // Handle escaped characters in pattern.
        if (*p_cur == '\\' && (std::next(p_cur)) != p_end) {
            if (s_cur == s_end) {
                return false;
            }

            const auto n = *std::next(p_cur);
            if (n == '*' || n == '?' || n == '\\') {
                if (*s_cur != n) {
                    return false;
                }
                return matches_glob(std::next(s_cur), s_end, std::next(p_cur, 2), p_end, char_equal);
            } else {
                return false; // Invalid escape sequence.
            }
        }

        // If the pattern is a star and the string is consumed
        if (*p_cur == '*' && s_cur == s_end) {
            return matches_glob(s_cur, s_end, std::next(p_cur), p_end, char_equal);
        }

        // If the pattern is a '?' and the string is consumed
        if (*p_cur == '?' && s_cur == s_end) {
            return false;
        }

        // If the pattern is not consumed, and the current char is not a wildcard, and the pattern is not consumed.
        if (s_cur == s_end) {
            return false;
        }

        // If the pattern contains '?', or current characters of both strings match
        if (*p_cur == '?' || char_equal(*p_cur, *s_cur)) {
            return matches_glob(std::next(s_cur), s_end, std::next(p_cur), p_end, char_equal);
        }

        // If there is * in the pattern, then there are two possibilities
        // a) We consider the current character of the string.
        // b) We ignore the current character of the string.
        if (*p_cur == '*') {
            return (matches_glob(s_cur, s_end, std::next(p_cur), p_end, char_equal) ||
                    matches_glob(std::next(s_cur), s_end, p_cur, p_end, char_equal));
        }

        return false;
    }

    /**
     * Checks whether the given string in range [s_cur, s_end) matches the glob pattern in range [p_cur, p_end).
     * Characters are compared for equality using the given binary predicate.
     *
     * @see matches_glob(I, I, I, I, CharEqual)
     *
     * @tparam CharEqual the type of the binary predicate used to test characters for equality
     * @param s the string to match against
     * @param p the patterm
     * @param char_equal the binary predicate
     * @return true if the given pattern matches the given string
     */
    template <typename CharEqual>
    bool matches_glob(const std::string_view& s, const std::string_view& p, const CharEqual& char_equal) {
        return matches_glob(std::begin(s), std::end(s), std::begin(p), std::end(p), char_equal);
    }

    /**
     * Sorts the given collection of strings. Strings are compared using the given comparator.
     *
     * @tparam C the collection type
     * @tparam StringLess the type of the comparator used to compare strings
     * @param c the collection of strings
     * @param string_less the comparator
     */
    template <typename C, typename StringLess>
    void sort(C& c, const StringLess& string_less) {
        std::sort(std::begin(c), std::end(c), string_less);
    }

    /**
     * Contains functions for working with strings case sensitively.
     */
    namespace cs {
        struct char_less {
            bool operator()(const char& lhs, const char& rhs) const {
                return lhs < rhs;
            }
        };

        struct char_equal {
            bool operator()(const char& lhs, const char& rhs) const {
                return lhs == rhs;
            }
        };

        struct string_less {
            bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
                return std::lexicographical_compare(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), char_less());
            }
        };

        struct string_equal {
            bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
                // std::equal can determine size mismatch when given random access iterators
                return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), char_equal());
            }
        };

        /**
         * Returns the first position at which the given strings differ. Characters are compared with case sensitivity.
         *
         * If the given strings are identical, then the length of the strings is returned.
         *
         * @param s1 the first string
         * @param s2 the second string
         * @return the first position at which the given strings differ
         */
        inline std::size_t mismatch(const std::string_view& s1, const std::string_view& s2) {
            return kdl::mismatch(s1, s2, char_equal());
        }

        /**
         * Checks whether the first string contains the second string. Characters are compared with case sensitivity.
         *
         * @param haystack the string to search in
         * @param needle the string to search for
         * @return true if the first string contains the second string and false otherwise
         */
        inline bool contains(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::contains(haystack, needle, char_equal());
        }

        /**
         * Checks whether the second string is a prefix of the first string. Characters are compared with case
         * sensitivity.
         *
         * @param haystack the string to search in
         * @param needle the string to search for
         * @return true if needle is a prefix of haystack
         */
        inline bool is_prefix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_prefix(haystack, needle, char_equal());
        }

        /**
         * Checks whether the second string is a suffix of the first string. Characters are compared with case
         * sensitivity.
         *
         * @param haystack the string to search in
         * @param needle the string to search for
         * @return true if needle is a suffix of haystack
         */
        inline bool is_suffix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_suffix(haystack, needle, char_equal());
        }

        /**
         * Performs lexicographical comparison of the given strings s1 and s2. Returns -1 if the first string is less
         * than the second string, or +1 in the opposite case, or 0 if both strings are equal. Characters are compared
         * with case sensitivity.
         *
         * @param s1 the first string
         * @param s2 the second string
         * @return an int indicating the result of the comparison
         */
        inline int compare(const std::string_view& s1, const std::string_view& s2) {
            return kdl::compare(s1, s2, char_less());
        }

        /**
         * Checks whether the given strings are equal. Characters are compared with case sensitivity.
         *
         * @param s1 the first string
         * @param s2 the second string
         * @return true if the given strings are equal and false otherwise
         */
        inline bool is_equal(const std::string_view& s1, const std::string_view& s2) {
            return kdl::is_equal(s1, s2, char_equal());
        }

        /**
         * Checks whether the given string in range [s_cur, s_end) matches the glob pattern in range [p_cur, p_end).
         * Characters are compared with case sensitivity.
         *
         * @see kdl::matches_glob(I, I, I, I, CharEqual)
         *
         * @param s the string to match against
         * @param p the patterm
         * @return true if the given pattern matches the given string
         */
        inline bool matches_glob(const std::string_view& s, const std::string_view& p) {
            return kdl::matches_glob(s, p, char_equal());
        }

        /**
         * Sorts the given collection of strings. The strings are compared with case sensitivity.
         *
         * @tparam C the collection type
         * @param c the collection of strings
         */
        template <typename C>
        void sort(C& c) {
            kdl::sort(c, string_less());
        }
    }

    /**
     * Contains functions for working with strings case insensitively.
     */
    namespace ci {
        struct char_less {
            bool operator()(const char& lhs, const char& rhs) const {
                return std::tolower(lhs) < std::tolower(rhs);
            }
        };

        struct char_equal {
            bool operator()(const char& lhs, const char& rhs) const {
                return std::tolower(lhs) == std::tolower(rhs);
            }
        };

        struct string_less {
            bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
                return std::lexicographical_compare(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), char_less());
            }
        };

        struct string_equal {
            bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
                // std::equal can determine size mismatch when given random access iterators
                return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), char_equal());
            }
        };

        /**
         * Returns the first position at which the given strings differ. Characters are compared without case
         * sensitivity.
         *
         * If the given strings are identical, then the length of the strings is returned.
         *
         * @param s1 the first string
         * @param s2 the second string
         * @return the first position at which the given strings differ
         */
        inline std::size_t mismatch(const std::string_view& s1, const std::string_view& s2) {
            return kdl::mismatch(s1, s2, char_equal());
        }

        /**
         * Checks whether the first string contains the second string. Characters are compared without case sensitivity.
         *
         * @param haystack the string to search in
         * @param needle the string to search for
         * @return true if the first string contains the second string and false otherwise
         */
        inline bool contains(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::contains(haystack, needle, char_equal());
        }

        /**
         * Checks whether the second string is a prefix of the first string. Characters are compared without case
         * sensitivity.
         *
         * @param haystack the string to search in
         * @param needle the string to search for
         * @return true if needle is a prefix of haystack
         */
        inline bool is_prefix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_prefix(haystack, needle, char_equal());
        }

        /**
         * Checks whether the second string is a suffix of the first string. Characters are compared without case
         * sensitivity.
         *
         * @param haystack the string to search in
         * @param needle the string to search for
         * @return true if needle is a suffix of haystack
         */
        inline bool is_suffix(const std::string_view& haystack, const std::string_view& needle) {
            return kdl::is_suffix(haystack, needle, char_equal());
        }

        /**
         * Performs lexicographical comparison of the given strings s1 and s2. Returns -1 if the first string is less
         * than the second string, or +1 in the opposite case, or 0 if both strings are equal. Characters are compared
         * without case sensitivity.
         *
         * @param s1 the first string
         * @param s2 the second string
         * @return an int indicating the result of the comparison
         */
        inline int compare(const std::string_view& s1, const std::string_view& s2) {
            return kdl::compare(s1, s2, char_less());
        }

        /**
         * Checks whether the given strings are equal. Characters are compared without case sensitivity.
         *
         * @param s1 the first string
         * @param s2 the second string
         * @return true if the given strings are equal and false otherwise
         */
        inline bool is_equal(const std::string_view& s1, const std::string_view& s2) {
            return kdl::is_equal(s1, s2, char_equal());
        }

        /**
         * Checks whether the given string in range [s_cur, s_end) matches the glob pattern in range [p_cur, p_end).
         * Characters are compared without case sensitivity.
         *
         * @see kdl::matches_glob(I, I, I, I, CharEqual)
         *
         * @param s the string to match against
         * @param p the patterm
         * @return true if the given pattern matches the given string
         */
        inline bool matches_glob(const std::string_view& s, const std::string_view& p) {
            return kdl::matches_glob(s, p, char_equal());
        }

        /**
         * Sorts the given collection of strings. The strings are compared without case sensitivity.
         *
         * @tparam C the collection type
         * @param c the collection of strings
         */
        template <typename C>
        void sort(C& c) {
            kdl::sort(c, string_less());
        }
    }
}

#endif //TRENCHBROOM_STRING_UTILS_H
