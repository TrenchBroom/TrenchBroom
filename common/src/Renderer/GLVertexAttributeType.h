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

#include "Ensure.h"
#include "Macros.h"
#include "Renderer/GL.h"
#include "Renderer/ShaderProgram.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace Renderer {
        /**
         * User defined vertex attribute types.
         *
         * @tparam A class containing the attribute name in a `static inline const std::string` member called `name`
         *           e.g. `struct Whatever { static inline const std::string name{"arrowPosition"}; };`
         * @tparam D the vertex component type
         * @tparam S the number of components
         * @tparam N whether to normalize signed integer types to [-1..1] and unsigned to [0..1]
         */
        template <class A, GLenum D, size_t S, bool N>
        class GLVertexAttributeUser {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);
            static const bool Normalize = N;

            static void setup(ShaderProgram* program, const size_t /* index */, const size_t stride, const size_t offset) {
                ensure(program != nullptr, "must have a program bound to use generic attributes");

                const GLint attributeIndex = program->findAttributeLocation(A::name);
                glAssert(glEnableVertexAttribArray(static_cast<GLuint>(attributeIndex)))
                glAssert(glVertexAttribPointer(static_cast<GLuint>(attributeIndex), static_cast<GLint>(S), D, Normalize ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(ShaderProgram* program, const size_t /* index */) {
                ensure(program != nullptr, "must have a program bound to use generic attributes");

                const GLint attributeIndex = program->findAttributeLocation(A::name);
                glAssert(glDisableVertexAttribArray(static_cast<GLuint>(attributeIndex)))
            }

            // Non-instantiable
            GLVertexAttributeUser() = delete;
            deleteCopyAndMove(GLVertexAttributeUser)
        };

        /**
         * Vertex position attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributePosition {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(ShaderProgram* /* program */, const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glEnableClientState(GL_VERTEX_ARRAY))
                glAssert(glVertexPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(ShaderProgram* /* program */, const size_t /* index */) {
                glAssert(glDisableClientState(GL_VERTEX_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributePosition() = delete;
            deleteCopyAndMove(GLVertexAttributePosition)
        };

        /**
         * Vertex normal attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, const size_t S>
        class GLVertexAttributeNormal {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(ShaderProgram* /* program */, const size_t /* index */, const size_t stride, const size_t offset) {
                assert(S == 3);
                glAssert(glEnableClientState(GL_NORMAL_ARRAY))
                glAssert(glNormalPointer(D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(ShaderProgram* /* program */, const size_t /* index */) {
                glAssert(glDisableClientState(GL_NORMAL_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributeNormal() = delete;
            deleteCopyAndMove(GLVertexAttributeNormal)
        };

        /**
         * Vertex color attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeColor {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(ShaderProgram* /* program */, const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glEnableClientState(GL_COLOR_ARRAY))
                glAssert(glColorPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(ShaderProgram* /* program */, const size_t /* index */) {
                glAssert(glDisableClientState(GL_COLOR_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributeColor() = delete;
            deleteCopyAndMove(GLVertexAttributeColor)
        };

        /**
         * Vertex texture coordinate (0) attribute types.
         *
         * @tparam D the vertex component type
         * @tparam S the number of components
         */
        template <GLenum D, size_t S>
        class GLVertexAttributeTexCoord0 {
        public:
            using ComponentType = typename GLType<D>::Type;
            using ElementType = vm::vec<ComponentType,S>;
            static const size_t Size = sizeof(ElementType);

            static void setup(ShaderProgram* /* program */, const size_t /* index */, const size_t stride, const size_t offset) {
                glAssert(glClientActiveTexture(GL_TEXTURE0))
                glAssert(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
                glAssert(glTexCoordPointer(static_cast<GLint>(S), D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset)))
            }

            static void cleanup(ShaderProgram* /* program */, const size_t /* index */) {
                glAssert(glClientActiveTexture(GL_TEXTURE0))
                glAssert(glDisableClientState(GL_TEXTURE_COORD_ARRAY))
            }

            // Non-instantiable
            GLVertexAttributeTexCoord0() = delete;
            deleteCopyAndMove(GLVertexAttributeTexCoord0)
        };

        namespace GLVertexAttributeTypes {
            using P2  = GLVertexAttributePosition<GL_FLOAT, 2>;
            using P3  = GLVertexAttributePosition<GL_FLOAT, 3>;
            using N   = GLVertexAttributeNormal<GL_FLOAT, 3>;
            using T02 = GLVertexAttributeTexCoord0<GL_FLOAT, 2>;
            using C4  = GLVertexAttributeColor<GL_FLOAT, 4>;
        }
    }
}

#endif
