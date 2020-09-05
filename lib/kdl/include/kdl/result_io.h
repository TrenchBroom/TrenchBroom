/*
 Copyright 2020 Kristian Duske

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

#ifndef result_h
#define result_h

#include "kdl/overload.h"
#include "kdl/result.h"

#include <iostream>

namespace kdl {
    template <typename Value, typename... Errors>
    std::ostream& operator<<(std::ostream& str, const result<Value, Errors...>& result) {
        result.visit([&](const auto& x) { str << x; });
        return str;
    }

    template <typename... Errors>
    std::ostream& operator<<(std::ostream& str, const result<void, Errors...>& result) {
        result.visit(kdl::overload {
            []()               { str << "void"; },
            [&](const auto& e) { str << e; }
        });
        return str;
    }
}

#endif /* result_h */
