/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(VboTest, Constructor) {
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            ASSERT_EQ(0xFFFF, vbo.capacity());
            ASSERT_EQ(0xFFFF, vbo.freeCapacity());
            ASSERT_EQ(VboState::Inactive, vbo.state());
        }
        
        TEST(VboTest, ActivateAndDeactivateVbo) {
            using namespace testing;
            
            GLMock = new CGLMock();
            Mock::AllowLeak(GLMock);
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            
            // activate for the first time
            EXPECT_CALL(*GLMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(*GLMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            {
                SetVboState activateVbo(vbo, VboState::Active);
                ASSERT_EQ(VboState::Active, vbo.state());
                
                // deactivate by leaving block
                EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            ASSERT_EQ(VboState::Inactive, vbo.state());
            
            // reactivate
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            {
                SetVboState activateVbo(vbo, VboState::Active);
                ASSERT_EQ(VboState::Active, vbo.state());
                
                // deactivate by leaving block
                EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            ASSERT_EQ(VboState::Inactive, vbo.state());

            // destroy vbo
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }

        TEST(VboTest, MapAndUnmapVbo) {
            using namespace testing;
            
            GLMock = new CGLMock();
            Mock::AllowLeak(GLMock);
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            
            unsigned char buffer[20];
            
            // activate and map for the first time
            EXPECT_CALL(*GLMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(*GLMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            EXPECT_CALL(*GLMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
            {
                SetVboState mapVbo(vbo, VboState::Mapped);
                ASSERT_EQ(VboState::Mapped, vbo.state());
                
                // deactivate and unmap by leaving block
                EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
                EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            ASSERT_EQ(VboState::Inactive, vbo.state());
            
            // reactivate
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            {
                SetVboState activateVbo(vbo, VboState::Active);
                ASSERT_EQ(VboState::Active, vbo.state());

                // map
                EXPECT_CALL(*GLMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
                {
                    SetVboState mapVbo(vbo, VboState::Mapped);
                    ASSERT_EQ(VboState::Mapped, vbo.state());
                    
                    // unmap by leaving block
                    EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
                }
                ASSERT_EQ(VboState::Active, vbo.state());
                
                // deactivate by leaving block
                EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            ASSERT_EQ(VboState::Inactive, vbo.state());
            
            // destroy vbo
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }
    }
}
