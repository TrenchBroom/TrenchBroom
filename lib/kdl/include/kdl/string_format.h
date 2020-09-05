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

#ifndef KDL_STRING_FORMAT_H
#define KDL_STRING_FORMAT_H

#include <cassert>
#include <sstream>
#include <string>
#include <string_view>

namespace kdl {
    /**
     * A string containing all characters which are considered whitespace.
     */
    constexpr auto Whitespace = " \n\t\r";

    /**
     * The default character to be used for escaping.
     */
    constexpr auto EscapeChar = '\\';

    /**
     * Selects one of the two given strings depending on the given predicate.
     *
     * @param predicate the predicate
     * @param positive the string to return if the predicate is true
     * @param negative the string to return if the predicate is false
     * @return the string
     */
    inline std::string str_select(const bool predicate, const std::string_view positive, const std::string_view negative) {
        return std::string(predicate ? positive : negative);
    }

    /**
     * Returns either of the given strings depending on the given count.
     *
     * @tparam C the type of the count parameter
     * @param count the count which determines the string to return
     * @param singular the string to return if count == 1
     * @param plural the string to return otherwise
     * @return one of the given strings depending on the given count
     */
    template <typename C>
    inline std::string str_plural(const C count, const std::string_view singular, const std::string_view plural) {
        return str_select(count == static_cast<C>(1), singular, plural);
    }

    /**
     * Returns either of the given strings depending on the given count. The returned string is prefixed with the given
     * prefix and suffixed with the given suffix.
     *
     * @tparam C the type of the count parameter
     * @param prefix the prefix
     * @param count the count which determines the string to return
     * @param singular the string to return if count == T(1)
     * @param plural the string to return otherwise
     * @param suffix the suffix
     * @return one of the given strings depending on the given count, with the given prefix and suffix
     */
    template <typename C>
    std::string str_plural(const std::string_view prefix, const C count, const std::string_view singular, const std::string_view plural, const std::string_view suffix = "") {
        std::stringstream result;
        result << prefix << str_plural(count, singular, plural) << suffix;
        return result.str();
    }

    /**
     * Trims the longest prefix and the longest suffix consisting only of the given characters from the given string.
     *
     * @param s the string to trim
     * @param chars the characters that should be trimmed from the start and from the end of the given string
     * @return the trimmed string
     */
    inline std::string str_trim(const std::string_view s, const std::string_view chars = Whitespace) {
        if (s.empty() || chars.empty()) {
            return std::string(s);
        }

        const auto first = s.find_first_not_of(chars);
        if (first == std::string::npos) {
            return "";
        }

        const auto last = s.find_last_not_of(chars);
        if (first > last) {
            return "";
        }

        return std::string(s.substr(first, last - first + 1));
    }

    /**
     * Convers the given ASCII character to lowercase.
     *
     * @param c the character to convert
     * @return the converted character
     */
    inline char str_to_lower(const char c) {
        if (c < 'A' || c > 'Z') {
            return c;
        } else {
            return static_cast<char>(c + 'a' - 'A');
        }
    }

    /**
     * Convers the given ASCII character to uppercase.
     *
     * @param c the character to convert
     * @return the converted character
     */
    inline char str_to_upper(const char c) {
        if (c < 'a' || c > 'z') {
            return c;
        } else {
            return static_cast<char>(c - 'a' + 'A');
        }
    }

    /**
     * Convers the given string to lowercase (only supports ASCII).
     *
     * @param str the string to convert
     * @return the converted string
     */
    inline std::string str_to_lower(const std::string_view str) {
        auto result = std::string();
        result.reserve(str.size());

        for (const auto c : str) {
            result.push_back(str_to_lower(c));
        }

        return result;
    }

    /**
     * Convers the given string to uppercase (only supports ASCII).
     *
     * @param str the string to convert
     * @return the converted string
     */
    inline std::string str_to_upper(const std::string_view str) {
        auto result = std::string();
        result.reserve(str.size());

        for (const auto c : str) {
            result.push_back(str_to_upper(c));
        }

        return result;
    }

    /**
     * Converts the first character and any character following one (or multiple) of the given delimiters to upper case.
     *
     * For example, calling capitalize("by the power of greyscull!", " ") would result in the string "By The Power Of
     * Greyscull!"
     *
     * @param str the string to capitalize
     * @param delims the delimiters, defaults to whitespace
     * @return the capitalized string
     */
    inline std::string str_capitalize(const std::string_view str, const std::string_view delims = Whitespace) {
        auto result = std::string();
        result.reserve(str.size());

        bool initial = true;
        for (const auto c : str) {
            if (delims.find(c) != std::string::npos) {
                initial = true;
                result.push_back(c);
            } else if (initial) {
                result.push_back(str_to_upper(c));
                initial = false;
            } else {
                result.push_back(c);
                initial = false;
            }
        }

        return result;
    }

    /**
     * Returns a string where the given characters are escaped by the given escape char, which defaults to a single
     * backslash.
     *
     * @param str the string to escape
     * @param chars the characters to escape if encountered in the string
     * @param esc the escape character
     * @return the escaped string
     */
    inline std::string str_escape(const std::string_view str, const std::string_view chars, char esc = EscapeChar) {
        if (str.empty()) {
            return "";
        }

        std::stringstream buffer;
        for (const auto c : str) {
            if (c == esc || chars.find_first_of(c) != std::string::npos) {
                buffer << esc;
            }
            buffer << c;
        }
        return buffer.str();
    }

    /**
     * Returns a string where the given characters are escaped by the given escape char, which defaults to a single
     * backslash. This function checks whether a character is already escaped in the given string before escaping it,
     * and a character will only be escaped if it needs to be.
     *
     * Precondition: chars does not contain the escape character
     *
     * @param str the string to escape
     * @param chars the characters to escape if encountered in the string, this must not include the escape character
     * @param esc the escape character
     * @return the escaped string
     */
    inline std::string str_escape_if_necessary(const std::string_view str, const std::string_view chars, char esc = EscapeChar) {
        assert(chars.find(esc) == std::string::npos);

        if (str.empty()) {
            return "";
        }

        std::stringstream buffer;
        auto escaped = false;
        for (const auto c : str) {
            const auto needsEscaping = (chars.find(c) != std::string::npos);
            if (needsEscaping) {
                // if 'c' is not prefixed by 'esc', insert an 'esc'
                if (!escaped) {
                    buffer << esc;
                }
            }

            if (c == esc) {
                escaped = !escaped;
            } else {
                escaped = false;
            }
            buffer << c;
        }
        return buffer.str();
    }

    /**
     * Unescapes any escaped characters in the given string. An escaped character is unescaped only if it is one of the
     * given chars.
     *
     * @param str the string to unescape
     * @param chars the characters to unescape if encountered escaped in the given string
     * @param esc the escape character
     * @return the unescaped string
     */
    inline std::string str_unescape(const std::string_view str, const std::string_view chars, char esc = EscapeChar) {
        if (str.empty()) {
            return "";
        }

        std::stringstream buffer;
        auto escaped = false;
        for (const auto c : str) {
            if (c == esc) {
                if (escaped) {
                    buffer << c;
                }
                escaped = !escaped;
            } else {
                if (escaped && chars.find_first_of(c) == std::string::npos) {
                    buffer << '\\';
                }
                buffer << c;
                escaped = false;
            }
        }

        if (escaped) {
            buffer << '\\';
        }

        return buffer.str();
    }

    /**
     * Checks whether the given string consists of only whitespace.
     *
     * @param str the string to check
     * @param whitespace a string of characters which should be considered whitespace
     * @return true if the given string consists of only whitespace characters and false otherwise
     */
    inline bool str_is_blank(const std::string_view str, const std::string_view whitespace = Whitespace) {
        return str.find_first_not_of(whitespace) == std::string::npos;
    }

    /**
     * Checks whether the given string consists of only numeric characters, i.e. '0', '1', ..., '9'. Note that the
     * empty string is considered to be numeric!
     *
     * @param str the string to check
     * @return true if the given string consists of only numeric characters, and false otherwise.
     */
    inline bool str_is_numeric(const std::string_view str) {
        for (const auto c : str) {
            if (c < '0' || c > '9') {
                return false;
            }
        }
        return true;
    }
}

#endif //KDL_STRING_FORMAT_H
