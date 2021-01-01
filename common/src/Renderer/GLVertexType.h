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

#pragma once

#include "Renderer/GLVertexAttributeType.h"
#include "Renderer/GLVertex.h"

#include <vecmath/forward.h>

namespace TrenchBroom {
    namespace Renderer {
        class ShaderProgram;

        /**
         * Defines the type of a vertex by the types of the vertex attributes. Enables to set up and clean up the
         * corresponding vertex buffer pointers.
         *
         * @tparam AttrTypes the vertex attribute types
         */
        template <typename... AttrTypes>
        struct GLVertexType;

        /**
         * Template specialization of the GLVertexType template for the case of multiple vertex attributes. The first
         * attribute is handled in this template, while the remaining attributes are delegated recursively.
         *
         * @tparam AttrType the type of the first vertex attribute
         * @tparam AttrTypeRest the types of the remaining vertex attributes
         */
        template <typename AttrType, typename... AttrTypeRest>
        struct GLVertexType<AttrType, AttrTypeRest...> {
            using Vertex = GLVertex<AttrType, AttrTypeRest...>;
            static const size_t Size = sizeof(Vertex);

            /**
             * Sets up the vertex buffer pointers for the attribute types of this vertex type.
             *
             * @param program current shader program
             * @param baseOffset the base offset into the corresponding vertex buffer
             */
            static void setup(ShaderProgram* program, const size_t baseOffset) {
                doSetup(program, 0, Size, baseOffset);
            }

            /**
             * Cleans up the vertex buffer pointers for the attributes of this vertex type.
             *
             * @param program current shader program
             */
            static void cleanup(ShaderProgram* program) {
                doCleanup(program, 0);
            }

            /**
             * Sets up the vertex buffer pointer for the first vertex attribute type and delegates the call for the
             * remaining attribute types. Do not call this directly, use the setup method instead.
             *
             * @param program current shader program
             * @param index the index of the attribute to be set up here
             * @param stride the stride of the vertex buffer pointer to be set up here
             * @param offset the offset of the vertex buffer pointer to be set up here
             */
            static void doSetup(ShaderProgram* program, const size_t index, const size_t stride, const size_t offset) {
                AttrType::setup(program, index, stride, offset);
                GLVertexType<AttrTypeRest...>::doSetup(program, index + 1, stride, offset + AttrType::Size);
            }

            /**
             * Cleans up the vertex buffer pointer for the first vertex attribute type and delegates the call for the
             * remaining attribute types. Do not call this directly, use the cleanup method instead.
             *
             * Note that the pointers are cleaned up in reverse order (last attribute first).
             *
             * @param program current shader program
             * @param index the index of the attribute to be cleaned up here
             */
            static void doCleanup(ShaderProgram* program, const size_t index) {
                GLVertexType<AttrTypeRest...>::doCleanup(program, index + 1);
                AttrType::cleanup(program, index);
            }

            // Non-instantiable
            GLVertexType() = delete;
            deleteCopyAndMove(GLVertexType)
        };

        /**
         * Template specialization of the GLVertexType template for the case of a single vertex attributes. This is
         * the base case for the recursive calls in the multi-attribute GLVertexType specialization above.
         *
         * @tparam AttrType the type of the vertex attribute
         */
        template <typename AttrType>
        struct GLVertexType<AttrType> {
            using Vertex = GLVertex<AttrType>;
            static const size_t Size = sizeof(Vertex);

            /**
             * Sets up the vertex buffer pointer for the attribute type of this vertex type.
             *
             * @param program current shader program
             * @param baseOffset the base offset into the corresponding vertex buffer
             */
            static void setup(ShaderProgram* program, const size_t baseOffset) {
                doSetup(program, 0, Size, baseOffset);
            }

            /**
             * Cleans up the vertex buffer pointer for the attribute of this vertex type.
             *
             * @param program current shader program
             */
            static void cleanup(ShaderProgram* program) {
                doCleanup(program, 0);
            }

            /**
             * Sets up the vertex buffer pointer for the vertex attribute type. Do not call this directly, use the setup
             * method instead.
             *
             * @param program current shader program
             * @param index the index of the attribute to be set up here
             * @param stride the stride of the vertex buffer pointer to be set up here
             * @param offset the offset of the vertex buffer pointer to be set up here
             */
            static void doSetup(ShaderProgram* program, const size_t index, const size_t stride, const size_t offset) {
                AttrType::setup(program, index, stride, offset);
            }

            /**
             * Cleans up the vertex buffer pointer for the vertex attribute type. Do not call this directly, use the
             * cleanup method instead.
             *
             * @param program current shader program
             * @param index the index of the attribute to be cleaned up here
             */
            static void doCleanup(ShaderProgram* program, const size_t index) {
                AttrType::cleanup(program, index);
            }

            // Non-instantiable
            GLVertexType() = delete;
            deleteCopyAndMove(GLVertexType)
        };

        namespace GLVertexTypes {
            using P2     = GLVertexType<GLVertexAttributeTypes::P2>;
            using P3     = GLVertexType<GLVertexAttributeTypes::P3>;
            using P2C4   = GLVertexType<GLVertexAttributeTypes::P2, GLVertexAttributeTypes::C4>;
            using P3C4   = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::C4>;
            using P2T2   = GLVertexType<GLVertexAttributeTypes::P2, GLVertexAttributeTypes::T02>;
            using P3T2   = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::T02>;
            using P2T2C4 = GLVertexType<GLVertexAttributeTypes::P2, GLVertexAttributeTypes::T02, GLVertexAttributeTypes::C4>;
            using P3T2C4 = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::T02, GLVertexAttributeTypes::C4>;
            using P3N    = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::N>;
            using P3NC4  = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::N, GLVertexAttributeTypes::C4>;
            using P3NT2  = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::N, GLVertexAttributeTypes::T02>;
        }
    }
}

