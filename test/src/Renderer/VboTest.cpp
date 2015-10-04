/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include <gmock/gmock.h>

#include "GL/GLMock.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"

#include <limits>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        TEST(VboTest, constructor) {
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            ASSERT_FALSE(vbo.active());
        }
        
        TEST(VboTest, activateAndDeactivateVbo) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            GLMock glMock;
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            
            // activate for the first time
            EXPECT_CALL(glMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(glMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            {
                ActivateVbo activate(vbo);
                ASSERT_TRUE(vbo.active());
                
                // deactivate by leaving block
                EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            ASSERT_FALSE(vbo.active());
            
            // reactivate
            EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            {
                ActivateVbo activate(vbo);
                ASSERT_TRUE(vbo.active());
                
                // deactivate by leaving block
                EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            ASSERT_FALSE(vbo.active());

            // destroy vbo
            EXPECT_CALL(glMock, DeleteBuffers(1, Pointee(13)));
        }
        
        TEST(VboTest, allocateBlocks) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            GLMock glMock;
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            
            unsigned char buffer[0xFFFF];
            
            // activate for the first time
            EXPECT_CALL(glMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(glMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            {
                ActivateVbo activate(vbo);
                
                VboBlock* block1 = vbo.allocateBlock(124);
                ASSERT_EQ(124u, block1->capacity());
                
                VboBlock* block2 = vbo.allocateBlock(646);
                ASSERT_EQ(646u, block2->capacity());
                
                const size_t block3Capacity = 0xFFFF - block1->capacity() - block2->capacity();
                VboBlock* block3 = vbo.allocateBlock(block3Capacity);
                ASSERT_EQ(block3Capacity, block3->capacity());
                
                // buffer reallocation
                EXPECT_CALL(glMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
                EXPECT_CALL(glMock, UnmapBuffer(GL_ARRAY_BUFFER));
                EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 0));
                EXPECT_CALL(glMock, DeleteBuffers(1, Pointee(13)));
                EXPECT_CALL(glMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(14));
                EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 14));
                EXPECT_CALL(glMock, BufferData(GL_ARRAY_BUFFER, 0x17FFE, NULL, GL_DYNAMIC_DRAW));
                EXPECT_CALL(glMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
                EXPECT_CALL(glMock, UnmapBuffer(GL_ARRAY_BUFFER));

                VboBlock* block4 = vbo.allocateBlock(373);
                ASSERT_EQ(373u, block4->capacity());

                // deactivate buffer
                EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            
            // destroy vbo
            EXPECT_CALL(glMock, DeleteBuffers(1, Pointee(14)));
        }

        TEST(VboTest, allocateBlockAndWriteBuffer) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            typedef std::vector<unsigned char> Buf;
            
            GLMock glMock;
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            
            // activate for the first time
            EXPECT_CALL(glMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(glMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            {
                ActivateVbo activate(vbo);
                
                VboBlock* block1 = vbo.allocateBlock(124);
                ASSERT_EQ(124u, block1->capacity());
                
                Buf writeBuffer;
                for (unsigned char i = 0; i < 124; i++)
                    writeBuffer.push_back(i);
                
                EXPECT_CALL(glMock, BufferSubData(GL_ARRAY_BUFFER, 0, 124, _));

                MapVboBlock map(block1);
                const size_t offset = block1->writeBuffer(0, writeBuffer);
                ASSERT_EQ(124u, offset);
                
                // deactivate buffer by leaving block
                EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            
            // destroy vbo
            EXPECT_CALL(glMock, DeleteBuffers(1, Pointee(13)));
        }
        
        TEST(VboTest, deallocateBlock) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            GLMock glMock;
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            
            // activate for the first time
            EXPECT_CALL(glMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(glMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            {
                ActivateVbo activate(vbo);
                
                // allocate and free a block
                VboBlock* block = vbo.allocateBlock(300);
                block->free();
                
                // deactivate by leaving block
                EXPECT_CALL(glMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            }
            
            // destroy vbo
            EXPECT_CALL(glMock, DeleteBuffers(1, Pointee(13)));
        }
    }
}
