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

#include "VertexArray.h"

#include "Renderer/PrimType.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        VertexArray::BaseHolder::~BaseHolder() = default;

        VertexArray::VertexArray() :
        m_prepared(false),
        m_setup(false) {}

        bool VertexArray::empty() const {
            return vertexCount() == 0;
        }

        size_t VertexArray::sizeInBytes() const {
            return m_holder.get() == nullptr ? 0 : m_holder->sizeInBytes();
        }

        size_t VertexArray::vertexCount() const {
            return m_holder.get() == nullptr ? 0 : m_holder->vertexCount();
        }

        bool VertexArray::prepared() const {
            return m_prepared;
        }

        void VertexArray::prepare(VboManager& vboManager) {
            if (!prepared() && !empty()) {
                m_holder->prepare(vboManager);
            }
            m_prepared = true;
        }

        bool VertexArray::setup() {
            if (empty())
                return false;

            assert(prepared());
            assert(!m_setup);


            m_holder->setup();
            m_setup = true;
            return true;
        }

        void VertexArray::cleanup() {
            assert(m_setup);
            assert(!empty());
            m_holder->cleanup();
            m_setup = false;
        }

        void VertexArray::render(const PrimType primType) {
            render(primType, 0, static_cast<GLsizei>(vertexCount()));
        }

        void VertexArray::render(const PrimType primType, const GLint index, const GLsizei count) {
            assert(prepared());
            if (!m_setup) {
                if (setup()) {
                    glAssert(glDrawArrays(toGL(primType), index, count));
                    cleanup();
                }
            } else {
                glAssert(glDrawArrays(toGL(primType), index, count));
            }
        }

        void VertexArray::render(const PrimType primType, const GLIndices& indices, const GLCounts& counts, const GLint primCount) {
            assert(prepared());
            if (!m_setup) {
                if (setup()) {
                    const auto* indexArray = indices.data();
                    const auto* countArray = counts.data();
                    glAssert(glMultiDrawArrays(toGL(primType), indexArray, countArray, primCount));
                    cleanup();
                }
            } else {
                const auto* indexArray = indices.data();
                const auto* countArray = counts.data();
                glAssert(glMultiDrawArrays(toGL(primType), indexArray, countArray, primCount));
            }

        }

        void VertexArray::render(const PrimType primType, const GLIndices& indices, const GLsizei count) {
            assert(prepared());
            if (!m_setup) {
                if (setup()) {
                    const auto* indexArray = indices.data();
                    glAssert(glDrawElements(toGL(primType), count, GL_UNSIGNED_INT, indexArray));
                    cleanup();
                }
            } else {
                const auto* indexArray = indices.data();
                glAssert(glDrawElements(toGL(primType), count, GL_UNSIGNED_INT, indexArray));
            }
        }

        VertexArray::VertexArray(std::shared_ptr<BaseHolder> holder) :
        m_holder(std::move(holder)),
        m_prepared(false),
        m_setup(false) {}
    }
}
