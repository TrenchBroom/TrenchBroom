/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_AttributeSpec_h
#define TrenchBroom_AttributeSpec_h

#include "Macros.h"
#include "Renderer/GL.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace Renderer {
        /**
         * The values of this enum define the possible types of OpenGL vertex attributes.
         */
        enum class GLVertexAttributeTypeTag {
            User,
            Position,
            Normal,
            Color,
            TexCoord0,
            TexCoord1,
            TexCoord2,
            TexCoord3
        };

        /**
         * Base template to define a vertex attribute type. This should be unused; use to the partial specializations
         * to define actual vertex attribute types.
         *
         * @tparam T the type of the vertex attribute
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLVertexAttributeTypeTag T, GLenum D, size_t S>
        class GLVertexAttributeType {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            /**
             * Sets up a vertex buffer pointer for this attribute with the given index, stride, and offset.
             *
             * @param index the attribute's index (position in the vertices)
             * @param stride the stride for the vertex buffer pointer
             * @param offset the offset for the vertex buffer pointer
             */
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                unused(index);
                unused(stride);
                unused(offset);
            }
            static void cleanup(const size_t /* index */) {}

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for user defined vertex attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::User, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glAssert(glEnableVertexAttribArray(static_cast<GLuint>(index)))
                glAssert(glVertexAttribPointer(static_cast<GLuint>(index), static_cast<GLint>(S), D, 0, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t index) {
                glAssert(glDisableVertexAttribArray(static_cast<GLuint>(index)))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for vertex position attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::Position, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glEnableClientState(GL_VERTEX_ARRAY))
                glAssert(glVertexPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t /* index */) {
                glAssert(glDisableClientState(GL_VERTEX_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for vertex normal attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, const size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::Normal, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t /* index */, const size_t stride, const size_t offset) {
                assert(S == 3);
                glAssert(glEnableClientState(GL_NORMAL_ARRAY))
                glAssert(glNormalPointer(D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t /* index */) {
                glAssert(glDisableClientState(GL_NORMAL_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for vertex color attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::Color, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glEnableClientState(GL_COLOR_ARRAY))
                glAssert(glColorPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t /* index */) {
                glAssert(glDisableClientState(GL_COLOR_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for vertex texture coordinate (0) attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::TexCoord0, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glClientActiveTexture(GL_TEXTURE0))
                glAssert(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glTexCoordPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t /* index */) {
                glAssert(glClientActiveTexture(GL_TEXTURE0))
                glAssert(glDisableClientState(GL_TEXTURE_COORD_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for vertex texture coordinate (1) attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::TexCoord1, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glClientActiveTexture(GL_TEXTURE1))
                glAssert(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glTexCoordPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t /* index */) {
                glAssert(glClientActiveTexture(GL_TEXTURE1))
                glAssert(glDisableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glClientActiveTexture(GL_TEXTURE0))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for vertex texture coordinate (2) attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::TexCoord2, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glClientActiveTexture(GL_TEXTURE2))
                glAssert(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glTexCoordPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t /* index */) {
                glAssert(glClientActiveTexture(GL_TEXTURE2))
                glAssert(glDisableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glClientActiveTexture(GL_TEXTURE0))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        /**
         * Partial specialization for vertex texture coordinate (3) attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeType<GLVertexAttributeTypeTag::TexCoord3, D, S> {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glClientActiveTexture(GL_TEXTURE3))
                glAssert(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glTexCoordPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(const size_t /* index */) {
                glAssert(glClientActiveTexture(GL_TEXTURE3))
                glAssert(glDisableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glClientActiveTexture(GL_TEXTURE0))
            }

            // Non-instantiable
            GLVertexAttributeType() = delete;
            deleteCopyAndMove(GLVertexAttributeType)
        };

        namespace GLVertexAttributeTypes {
            using P2  = GLVertexAttributeType<GLVertexAttributeTypeTag::Position, GL_FLOAT, 2>;
            using P3  = GLVertexAttributeType<GLVertexAttributeTypeTag::Position, GL_FLOAT, 3>;
            using N   = GLVertexAttributeType<GLVertexAttributeTypeTag::Normal, GL_FLOAT, 3>;
            using T02 = GLVertexAttributeType<GLVertexAttributeTypeTag::TexCoord0, GL_FLOAT, 2>;
            using T12 = GLVertexAttributeType<GLVertexAttributeTypeTag::TexCoord1, GL_FLOAT, 2>;
            using C4  = GLVertexAttributeType<GLVertexAttributeTypeTag::Color, GL_FLOAT, 4>;
        }
    }
}

#endif
