/*
 Copyright (C) 2016 Eric Wasylishen
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Macros.h"
#include "StringUtils.h"

namespace TrenchBroom {
    namespace Ensure {
        TEST(EnsureTest, successfulEnsure) {
            EXPECT_NO_THROW(ensure(true, "this shouldn't fail"));
        }
        
        TEST(EnsureTest, failingEnsure) {
            EXPECT_ANY_THROW(ensure(false, "this should fail"));
            EXPECT_THROW(ensure(false, "this should fail"), TrenchBroom::ConditionFailedException);
        }
        
        TEST(EnsureTest, failingEnsureMessage) {
            bool caught = false;
            int lineNumber;
            
            try {
                lineNumber = __LINE__; ensure(1 + 1 == 3, "this should fail");
            } catch (TrenchBroom::ConditionFailedException &exception) {
                String message = exception.what();
                
                EXPECT_TRUE(message.find("something not in the exception message") == String::npos);
                
                EXPECT_FALSE(message.find("this should fail") == String::npos);
                EXPECT_FALSE(message.find("1 + 1 == 3") == String::npos);
                
                StringStream fileLineNumberStream;
                fileLineNumberStream << __FILE__ << ":" << lineNumber;
                EXPECT_FALSE(message.find(fileLineNumberStream.str()) == String::npos);
                
                caught = true;
            }
            
            EXPECT_TRUE(caught);
        }
    }
}
