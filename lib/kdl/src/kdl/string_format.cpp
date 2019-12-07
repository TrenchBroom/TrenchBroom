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

#include "string_format.h"

#include "string_compare.h"

#include <cassert>
#include <cctype> // for std::tolower
#include <sstream>

namespace kdl {
    std::string str_select(const bool predicate, const std::string_view& positive, const std::string_view& negative) {
        return std::string(predicate ? positive : negative);
    }

    std::string str_plural(const int count, const std::string_view& singular, const std::string_view& plural) {
        return std::string(count == 1 ? singular : plural);
    }

    std::string str_plural(const std::string_view& prefix, const int count, const std::string_view& singular, const std::string_view& plural, const std::string_view& suffix) {
        std::stringstream result;
        result << prefix << str_plural(count, singular, plural) << suffix;
        return result.str();
    }

    std::string str_trim(const std::string_view& s, const std::string_view& chars) {
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

    std::string str_to_lower(const std::string_view& str) {
        auto result = std::string();
        result.reserve(str.size());

        for (const auto c : str) {
            result.push_back(static_cast<std::string::value_type>(std::tolower(c)));
        }

        return result;
    }

    std::string str_to_upper(const std::string_view& str) {
        auto result = std::string();
        result.reserve(str.size());

        for (const auto c : str) {
            result.push_back(static_cast<std::string::value_type>(std::toupper(c)));
        }

        return result;
    }

    std::string str_capitalize(const std::string_view& str, const std::string_view& delims) {
        auto result = std::string();
        result.reserve(str.size());

        bool initial = true;
        for (const auto c : str) {
            if (delims.find(c) != std::string::npos) {
                initial = true;
                result.push_back(c);
            } else if (initial) {
                result.push_back(static_cast<std::string::value_type>(std::toupper(c)));
                initial = false;
            } else {
                result.push_back(c);
                initial = false;
            }
        }

        return result;
    }

    std::string str_escape(const std::string_view& str, const std::string_view& chars, char esc) {
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

    std::string str_escape_if_necessary(const std::string_view& str, const std::string_view& chars, char esc) {
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

    std::string str_unescape(const std::string_view& str, const std::string_view& chars, char esc) {
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

    bool str_is_blank(const std::string_view& str, const std::string_view& whitespace) {
        return str.find_first_not_of(whitespace) == std::string::npos;
    }

    bool str_is_numeric(const std::string_view& str) {
        for (const auto c : str) {
            if (c < '0' || c > '9') {
                return false;
            }
        }
        return true;
    }
}
