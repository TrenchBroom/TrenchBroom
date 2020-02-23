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

#include <gtest/gtest.h>

#include "kdl/memory_utils.h"

#include <memory>

namespace kdl {
    class A {
    public:
        virtual ~A() = default;
    };
    
    class B : public A {};
    
    TEST(memory_utils_test, mem_static_pointer_cast) {
        std::unique_ptr<A> auPtr = std::make_unique<B>();
        std::unique_ptr<B> buPtr = mem_static_pointer_cast<B>(std::move(auPtr));
        
        std::shared_ptr<A> asPtr = std::make_shared<B>();
        std::shared_ptr<B> bsPtr = mem_static_pointer_cast<B>(asPtr);
    }
}
