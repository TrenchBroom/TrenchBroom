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
#include "Renderer/Vbo.h"

#include <cstdlib>
#include <ctime>

namespace TrenchBroom {
    namespace Renderer {
        TEST(VertexArrayTest, vertex1Array) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            std::srand(static_cast<unsigned int>(std::time(NULL)));
            
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
            
            VertexArray attrArray(vbo, vertices);
            attrArray.prepare();
            
            for (size_t i = 0; i < vertexPositions.size(); i++) {
                const float* x = reinterpret_cast<const float*>(buffer + i * 3 * sizeof(float) + 0 * sizeof(float));
                const float* y = reinterpret_cast<const float*>(buffer + i * 3 * sizeof(float) + 1 * sizeof(float));
                const float* z = reinterpret_cast<const float*>(buffer + i * 3 * sizeof(float) + 2 * sizeof(float));
                ASSERT_FLOAT_EQ(vertexPositions[i].x(), *x);
                ASSERT_FLOAT_EQ(vertexPositions[i].y(), *y);
                ASSERT_FLOAT_EQ(vertexPositions[i].z(), *z);
            }

            EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }

        TEST(VertexArrayTest, vertex2Array) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            std::srand(static_cast<unsigned int>(std::time(NULL)));
            
            Vec3f::List vertexPositions;
            Vec2f::List textureCoords;
            VP3T2::List vertices;
            
            for (size_t i = 0; i < 22; ++i) {
                Vec3f v;
                Vec2f t;
                for (size_t j = 0; j < 3; ++j)
                    v[j] = static_cast<float>(std::rand());
                for (size_t j = 0; j < 2; ++j)
                    t[j] = static_cast<float>(std::rand());
                vertexPositions.push_back(v);
                textureCoords.push_back(t);
                vertices.push_back(VP3T2(v, t));
            }
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            unsigned char buffer[0xFFFF];
            
            EXPECT_CALL(*GLMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(*GLMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            EXPECT_CALL(*GLMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
            SetVboState setVboState(vbo);
            setVboState.mapped();
            
            VertexArray attrArray(vbo, vertices);
            attrArray.prepare();
            
            for (size_t i = 0; i < vertexPositions.size(); i++) {
                const float* x = reinterpret_cast<const float*>(buffer + i * 5 * sizeof(float) + 0 * sizeof(float));
                const float* y = reinterpret_cast<const float*>(buffer + i * 5 * sizeof(float) + 1 * sizeof(float));
                const float* z = reinterpret_cast<const float*>(buffer + i * 5 * sizeof(float) + 2 * sizeof(float));
                const float* s = reinterpret_cast<const float*>(buffer + i * 5 * sizeof(float) + 3 * sizeof(float));
                const float* t = reinterpret_cast<const float*>(buffer + i * 5 * sizeof(float) + 4 * sizeof(float));
                ASSERT_FLOAT_EQ(vertexPositions[i].x(), *x);
                ASSERT_FLOAT_EQ(vertexPositions[i].y(), *y);
                ASSERT_FLOAT_EQ(vertexPositions[i].z(), *z);
                ASSERT_FLOAT_EQ(textureCoords[i].x(), *s);
                ASSERT_FLOAT_EQ(textureCoords[i].y(), *t);
            }
            
            EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }
        
        TEST(VertexArrayTest, vertex3Array) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            std::srand(static_cast<unsigned int>(std::time(NULL)));
            
            Vec3f::List vertexPositions;
            Vec3f::List normals;
            Vec2f::List textureCoords;
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
                vertexPositions.push_back(v);
                normals.push_back(n);
                textureCoords.push_back(t);
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
            
            VertexArray attrArray(vbo, vertices);
            attrArray.prepare();
            
            for (size_t i = 0; i < vertexPositions.size(); i++) {
                const float* vx = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 0 * sizeof(float));
                const float* vy = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 1 * sizeof(float));
                const float* vz = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 2 * sizeof(float));
                const float* nx = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 3 * sizeof(float));
                const float* ny = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 4 * sizeof(float));
                const float* nz = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 5 * sizeof(float));
                const float* ts = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 6 * sizeof(float));
                const float* tt = reinterpret_cast<const float*>(buffer + i * 8 * sizeof(float) + 7 * sizeof(float));
                ASSERT_FLOAT_EQ(vertexPositions[i].x(), *vx);
                ASSERT_FLOAT_EQ(vertexPositions[i].y(), *vy);
                ASSERT_FLOAT_EQ(vertexPositions[i].z(), *vz);
                ASSERT_FLOAT_EQ(normals[i].x(), *nx);
                ASSERT_FLOAT_EQ(normals[i].y(), *ny);
                ASSERT_FLOAT_EQ(normals[i].z(), *nz);
                ASSERT_FLOAT_EQ(textureCoords[i].x(), *ts);
                ASSERT_FLOAT_EQ(textureCoords[i].y(), *tt);
            }
            
            EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }

        TEST(VertexArrayTest, indexedVertex1Array) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            std::srand(static_cast<unsigned int>(std::time(NULL)));
            
            Vec3f::List vertexPositions;
            IndexedVertexList<VP3> indexedVertices;
            
            for (size_t i = 0; i < 7; ++i) {
                for (size_t j = 0; j < 22; ++j) {
                    Vec3f v;
                    for (size_t k = 0; k < 3; ++k)
                        v[k] = static_cast<float>(std::rand());
                    vertexPositions.push_back(v);
                    indexedVertices.addVertex(VP3(v));
                }
                indexedVertices.endPrimitive();
            }
            
            Vbo vbo(0xFFFF, GL_ARRAY_BUFFER);
            unsigned char buffer[0xFFFF];
            
            EXPECT_CALL(*GLMock, GenBuffers(1,_)).WillOnce(SetArgumentPointee<1>(13));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 13));
            EXPECT_CALL(*GLMock, BufferData(GL_ARRAY_BUFFER, 0xFFFF, NULL, GL_DYNAMIC_DRAW));
            EXPECT_CALL(*GLMock, MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)).WillOnce(Return(buffer));
            SetVboState setVboState(vbo);
            setVboState.mapped();
            
            VertexArray attrArray(vbo, indexedVertices.vertices());
            attrArray.prepare();
            
            for (size_t i = 0; i < vertexPositions.size(); i++) {
                const float* x = reinterpret_cast<const float*>(buffer + i * 3 * sizeof(float) + 0 * sizeof(float));
                const float* y = reinterpret_cast<const float*>(buffer + i * 3 * sizeof(float) + 1 * sizeof(float));
                const float* z = reinterpret_cast<const float*>(buffer + i * 3 * sizeof(float) + 2 * sizeof(float));
                ASSERT_FLOAT_EQ(vertexPositions[i].x(), *x);
                ASSERT_FLOAT_EQ(vertexPositions[i].y(), *y);
                ASSERT_FLOAT_EQ(vertexPositions[i].z(), *z);
            }
            
            EXPECT_CALL(*GLMock, UnmapBuffer(GL_ARRAY_BUFFER));
            EXPECT_CALL(*GLMock, BindBuffer(GL_ARRAY_BUFFER, 0));
            EXPECT_CALL(*GLMock, DeleteBuffers(1, Pointee(13)));
        }
    }
}
