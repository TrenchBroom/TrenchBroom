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

#ifndef TRENCHBROOM_STRING_FORMAT_H
#define TRENCHBROOM_STRING_FORMAT_H

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
    std::string str_select(bool predicate, const std::string_view& positive, const std::string_view& negative);

    /**
     * Returns either of the given strings depending on the given count.
     *
     * @param count the count which determines the string to return
     * @param singular the string to return if count == 1
     * @param plural the string to return otherwise
     * @return one of the given strings depending on the given count
     */
    std::string str_plural(int count, const std::string_view& singular, const std::string_view& plural);

    /**
     * Returns either of the given strings depending on the given count. The returned string is prefixed with the given
     * prefix and suffixed with the given suffix.
     *
     * @param prefix the prefix
     * @param count the count which determines the string to return
     * @param singular the string to return if count == T(1)
     * @param plural the string to return otherwise
     * @param suffix the suffix
     * @return one of the given strings depending on the given count, with the given prefix and suffix
     */
    std::string str_plural(const std::string_view& prefix, int count, const std::string_view& singular, const std::string_view& plural, const std::string_view& suffix = "");

    /**
     * Returns either of the given strings depending on the given count.
     *
     * @tparam T the type of the given count
     * @param count the count which determines the string to return
     * @param singular the string to return if count == T(1)
     * @param plural the string to return otherwise
     * @return one of the given strings depending on the given count
     */
    template <typename T>
    std::string str_plural(const T count, const std::string_view& singular, const std::string_view& plural) {
        return str_plural(static_cast<int>(count), singular, plural);
    }

    /**
     * Returns either of the given strings depending on the given count. The returned string is prefixed with the given
     * prefix and suffixed with the given suffix.
     *
     * @tparam T the type of the given count
     * @param prefix the prefix
     * @param count the count which determines the string to return
     * @param singular the string to return if count == T(1)
     * @param plural the string to return otherwise
     * @param suffix the suffix
     * @return one of the given strings depending on the given count, with the given prefix and suffix
     */
    template <typename T>
    std::string str_plural(const std::string_view& prefix, const T count, const std::string_view& singular, const std::string_view& plural, const std::string_view& suffix = "") {
        return str_plural(prefix, static_cast<int>(count), singular, plural, suffix);
    }

    /**
     * Trims the longest prefix and the longest suffix consisting only of the given characters from the given string.
     *
     * @param s the string to trim
     * @param chars the characters that should be trimmed from the start and from the end of the given string
     * @return the trimmed string
     */
    std::string str_trim(const std::string_view& s, const std::string_view& chars = Whitespace);

    /**
     * Convers the given string to lowercase using the C locale.
     *
     * @param str the string to convert
     * @return the converted string
     */
    std::string str_to_lower(const std::string_view& str);

    /**
     * Convers the given string to uppercase using the C locale.
     *
     * @param str the string to convert
     * @return the converted string
     */
    std::string str_to_upper(const std::string_view& str);

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
    std::string str_capitalize(const std::string_view& str, const std::string_view& delims = Whitespace);

    /**
     * Returns a string where the given characters are escaped by the given escape char, which defaults to a single
     * backslash.
     *
     * @param str the string to escape
     * @param chars the characters to escape if encountered in the string
     * @param esc the escape character
     * @return the escaped string
     */
    std::string str_escape(const std::string_view& str, const std::string_view& chars, char esc = EscapeChar);

    /**
     * Returns a string where the given characters are escaped by the given escape char, which defaults to a single
     * backslash. This function checks whether a character is already escaped in the given string before escaping it,
     * and a character will only be escaped if it needs to be.
     *
     * @param str the string to escape
     * @param chars the characters to escape if encountered in the string, this must not include the escape character
     * @param esc the escape character
     * @return the escaped string
     */
    std::string str_escape_if_necessary(const std::string_view& str, const std::string_view& chars, char esc = EscapeChar);

    /**
     * Unescapes any escaped characters in the given string. An escaped character is unescaped only if it is one of the
     * given chars.
     *
     * @param str the string to unescape
     * @param chars the characters to unescape if encountered escaped in the given string
     * @param esc the escape character
     * @return the unescaped string
     */
    std::string str_unescape(const std::string_view& str, const std::string_view& chars, char esc = EscapeChar);

    /**
     * Checks whether the given string consists of only whitespace.
     *
     * @param str the string to check
     * @param whitespace a string of characters which should be considered whitespace
     * @return true if the given string consists of only whitespace characters and false otherwise
     */
    bool str_is_blank(const std::string_view& str, const std::string_view& whitespace = Whitespace);

    /**
     * Checks whether the given string consists of only numeric characters, i.e. '0', '1', ..., '9'.
     *
     * @param str the string to check
     * @return true if the given string consists of only numeric characters, and false otherwise.
     */
    bool str_is_numeric(const std::string_view& str);
}

#endif //TRENCHBROOM_STRING_FORMAT_H
