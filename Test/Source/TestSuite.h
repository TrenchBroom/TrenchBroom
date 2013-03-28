/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_TestSuite_h
#define TrenchBroom_TestSuite_h

#include <functional>
#include <vector>

namespace TrenchBroom {
    template <class SubClass>
    class TestSuite {
    private:
        typedef std::mem_fun_t<void, SubClass> TestCase;
        typedef std::vector<TestCase> TestCaseList;
        TestCaseList m_testCases;
    protected:
        inline void registerTestCase(TestCase testCase) {
            m_testCases.push_back(testCase);
        }
        
        inline void registerTestCase(void (SubClass::*f)()) {
            registerTestCase(std::mem_fun(f));
        }
        
        virtual void registerTestCases() {};
        virtual void setup() {}
        virtual void teardown() {}
    public:
        virtual ~TestSuite() {}

        inline void run() {
            registerTestCases();
            
            typename TestCaseList::iterator it, end;
            for (it = m_testCases.begin(), end = m_testCases.end(); it != end; ++it) {
                TestCase& testCase = *it;
                setup();
                testCase(static_cast<SubClass*>(this));
                teardown();
            }
        }
    };
}

#endif
