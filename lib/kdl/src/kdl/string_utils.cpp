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

#include "string_utils.h"
#include "string_format.h"

#include <algorithm> // for std::search
#include <cassert>

namespace kdl {
    std::vector<std::string> str_split(const std::string_view& str, const std::string_view& delims) {
        if (str.empty()) {
            return {};
        }

        const auto first = str.find_first_not_of(delims);
        if (first == std::string::npos) {
            return {};
        }

        const auto last = str.find_last_not_of(delims);
        assert(last != std::string::npos);
        assert(first <= last);

        std::vector<std::string> result;

        auto lastPos = first;
        auto pos = lastPos;
        while ((pos = str.find_first_of(delims, pos)) < last) {
            auto part = str_trim(str.substr(lastPos, pos - lastPos));
            if (!part.empty()) {
                result.push_back(std::move(part));
            }
            lastPos = ++pos;
        }

        if (lastPos <= last) {
            auto part = str_trim(str.substr(lastPos, last - lastPos + 1));
            if (!part.empty()) {
                result.push_back(std::move(part));
            }
        }

        return result;
    }

    std::string str_replace_every(const std::string_view& haystack, const std::string_view& needle, const std::string_view& replacement) {
        if (haystack.empty() || needle.empty() || needle == replacement) {
            return std::string(haystack);
        }

        std::string result(haystack);

        using diff_type = std::string::iterator::difference_type;
        auto it = std::search(std::begin(result), std::end(result), std::begin(needle), std::end(needle));
        while (it != std::end(result)) {
            // remember the position where the search will continue after replacement
            const auto next_offset = std::distance(std::begin(result), it) + static_cast<diff_type>(replacement.size());
            result.replace(it, std::next(it, static_cast<diff_type>(needle.size())), replacement); // invalidates it

            it = std::search(std::next(std::begin(result), next_offset), std::end(result), std::begin(needle), std::end(needle));
        }
        return result;
    }
}
