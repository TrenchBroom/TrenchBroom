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

#include "TrenchBroom.h"
#include "VecMath.h"

#include "Renderer/VertexArray.h"
#include "Renderer/VertexArrayRenderer.h"
#include "Renderer/Vbo.h"

#include <cstdlib>
#include <ctime>

namespace TrenchBroom {
    namespace Renderer {
        TEST(VertexArrayRendererTest, renderSerialP3Array) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            std::srand(static_cast<unsigned int>(std::time(NULL)));

            GLMock = new CGLMock();
            Mock::AllowLeak(GLMock);
            
            Vec3f::List vertexPositions;
            VP3::List vertices;
            
            for (size_t i = 0; i < 22; ++i) {
                Vec3f v;
                for (size_t j = 0; j < 3; ++j)
                    v[j] = static_cast<float>(std::rand());
                vertexPositions.push_back(v);
                vertices.push_back(VP3(v));
            }
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            unsigned char buffer[0xFFFF];
            
            EXPECT_CALL(*GLMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(*GLMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            EXPECT_CALL(*GLMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
            SetVboState setVboState(vbo);
            setVboState.mapped();

            VertexArray vertexArray(vbo, vertices);
            VertexArrayRenderer renderer(VertexSpec::P3(), GL_TRIANGLES, vertexArray);

            EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
            setVboState.active();
            
            EXPECT_CALL(*GLMock, EnableClientState(GL_VERTEX_ARRAY));
            EXPECT_CALL(*GLMock, VertexPointer(3, GL_FLOAT, 12, 0));
            EXPECT_CALL(*GLMock, DrawArrays(GL_TRIANGLES, 0, 22));
            EXPECT_CALL(*GLMock, DisableClientState(GL_VERTEX_ARRAY));
            renderer.render();
            
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }

        TEST(VertexArrayRendererTest, renderSerialP3N3T2Array) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            std::srand(static_cast<unsigned int>(std::time(NULL)));
            
            GLMock = new CGLMock();
            Mock::AllowLeak(GLMock);

            VP3N3T2::List vertices;
            
            for (size_t i = 0; i < 22; ++i) {
                Vec3f v;
                Vec3f n;
                Vec2f t;
                for (size_t j = 0; j < 3; ++j) {
                    v[j] = static_cast<float>(std::rand());
                    n[j] = static_cast<float>(std::rand());
                }
                for (size_t j = 0; j < 2; ++j)
                    t[j] = static_cast<float>(std::rand());
                vertices.push_back(VP3N3T2(v, n, t));
            }
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            unsigned char buffer[0xFFFF];
            
            EXPECT_CALL(*GLMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(*GLMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            EXPECT_CALL(*GLMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
            SetVboState setVboState(vbo);
            setVboState.mapped();
            
            VertexArray vertexArray(vbo, vertices);
            VertexArrayRenderer renderer(VertexSpec::P3N3T2(), GL_TRIANGLES, vertexArray);
            
            EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
            setVboState.active();
            
            EXPECT_CALL(*GLMock, EnableClientState(GL_VERTEX_ARRAY));
            EXPECT_CALL(*GLMock, VertexPointer(3, GL_FLOAT, (3 + 3 + 2) * 4, 0));
            EXPECT_CALL(*GLMock, EnableClientState(GL_NORMAL_ARRAY));
            EXPECT_CALL(*GLMock, NormalPointer(GL_FLOAT, (3 + 3 + 2) * 4, reinterpret_cast<const GLvoid*>(3 * 4)));
            EXPECT_CALL(*GLMock, ClientActiveTexture(GL_TEXTURE0));
            EXPECT_CALL(*GLMock, EnableClientState(GL_TEXTURE_COORD_ARRAY));
            EXPECT_CALL(*GLMock, TexCoordPointer(2, GL_FLOAT, (3 + 3 + 2) * 4, reinterpret_cast<const GLvoid*>(6 * 4)));
            EXPECT_CALL(*GLMock, DrawArrays(GL_TRIANGLES, 0, 22));
            EXPECT_CALL(*GLMock, DisableClientState(GL_VERTEX_ARRAY));
            EXPECT_CALL(*GLMock, DisableClientState(GL_NORMAL_ARRAY));
            EXPECT_CALL(*GLMock, ClientActiveTexture(GL_TEXTURE0));
            EXPECT_CALL(*GLMock, DisableClientState(GL_TEXTURE_COORD_ARRAY));
            renderer.render();
            
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }
    }
}
