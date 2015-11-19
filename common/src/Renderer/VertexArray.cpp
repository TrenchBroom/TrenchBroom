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

#include "VertexArray.h"

#include <algorithm>
#include <cassert>
#include <limits>

namespace TrenchBroom {
    namespace Renderer {
        VertexArray::RenderSpec::IndicesAndCounts::IndicesAndCounts() :
        indices(0),
        counts(0) {}

        VertexArray::RenderSpec::IndicesAndCounts::IndicesAndCounts(const GLint index, const GLsizei count) :
        indices(1, index),
        counts(1, count) {}
        
        VertexArray::RenderSpec::IndicesAndCounts::IndicesAndCounts(const IndexArray& i_indices, const CountArray& i_counts) :
        indices(i_indices),
        counts(i_counts) {
            assert(indices.size() == counts.size());
        }
        
        VertexArray::RenderSpec::IndicesAndCounts::IndicesAndCounts(IndexArray& i_indices, CountArray& i_counts) :
        indices(0),
        counts(0) {
            assert(i_indices.size() == i_counts.size());
            
            using std::swap;
            swap(indices, i_indices);
            swap(counts, i_counts);
        }

        size_t VertexArray::RenderSpec::IndicesAndCounts::size() const {
            return indices.size();
        }

        void VertexArray::RenderSpec::IndicesAndCounts::merge(PrimType primType, const IndicesAndCounts& other) {
            switch (primType) {
                case PT_Points:
                case PT_Lines:
                case PT_Triangles:
                case PT_Quads: {
                    if (size() == 1 && other.size() == 1) {
                        const GLint myIndex = indices.front();
                        GLsizei& myCount = counts.front();
                        const GLint theirIndex = other.indices.front();
                        const GLsizei theirCount = other.counts.front();
                        
                        if (theirIndex == myIndex + myCount) {
                            myCount += theirCount;
                            break;
                        }
                    }
                }
                case PT_LineStrips:
                case PT_LineLoops:
                case PT_TriangleFans:
                case PT_TriangleStrips:
                case PT_QuadStrips:
                case PT_Polygons:
                    VectorUtils::append(indices, other.indices);
                    VectorUtils::append(counts, other.counts);
                    break;
            }
        }
        
        VertexArray::RenderSpec::RenderSpec(const PrimType primType, const GLint index, const GLsizei count) {
            m_data.insert(std::make_pair(primType, IndicesAndCounts(index, count)));
        }

        VertexArray::RenderSpec::RenderSpec(const PrimType primType, const IndexArray& indices, const CountArray& counts) {
            m_data.insert(std::make_pair(primType, IndicesAndCounts(indices, counts)));
        }
        
        VertexArray::RenderSpec::RenderSpec(const PrimType primType, IndexArray& indices, CountArray& counts) {
            m_data.insert(std::make_pair(primType, IndicesAndCounts(indices, counts)));
        }

        void VertexArray::RenderSpec::merge(const RenderSpec& other) {
            Data::const_iterator it, end;
            for (it = other.m_data.begin(), end = other.m_data.end(); it != end; ++it) {
                const PrimType primType = it->first;
                const IndicesAndCounts& theirIndicesAndCounts = it->second;
                
                Data::iterator my = MapUtils::findOrInsert(m_data, primType, IndicesAndCounts());
                IndicesAndCounts& myIndicesAndCounts = my->second;
                myIndicesAndCounts.merge(primType, theirIndicesAndCounts);
            }
        }

        void VertexArray::RenderSpec::render() const {
            Data::const_iterator it, end;
            for (it = m_data.begin(), end = m_data.end(); it != end; ++it) {
                const PrimType primType = it->first;
                const IndicesAndCounts& indicesAndCounts = it->second;
                
                const GLint primCount = static_cast<GLint>(indicesAndCounts.size());
                const GLint* indexArray   = indicesAndCounts.indices.data();
                const GLsizei* countArray = indicesAndCounts.counts.data();
                glMultiDrawArrays(primType, indexArray, countArray, primCount);
            }
        }

        VertexArray::VertexArray() :
        m_prepared(false),
        m_setup(false) {}
        
        VertexArray& VertexArray::operator= (VertexArray other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(VertexArray& left, VertexArray& right) {
            using std::swap;
            swap(left.m_holder, right.m_holder);
            swap(left.m_prepared, right.m_prepared);
            swap(left.m_setup, right.m_setup);
        }

        size_t VertexArray::size() const {
            return m_holder == NULL ? 0 : m_holder->size();
        }

        size_t VertexArray::vertexCount() const {
            return m_holder == NULL ? 0 : m_holder->vertexCount();
        }

        bool VertexArray::prepared() const {
            return m_prepared;
        }

        void VertexArray::prepare(Vbo& vbo) {
            if (!m_prepared && m_holder != NULL && m_holder->vertexCount() > 0)
                m_holder->prepare(vbo);
            m_prepared = true;
        }

        void VertexArray::setup() {
            assert(m_prepared);
            assert(!m_setup);
            
            if (m_holder != NULL && m_holder->vertexCount() > 0)
                m_holder->setup();
            m_setup = true;
        }
        
        void VertexArray::cleanup() {
            assert(m_setup);
            if (m_holder != NULL && m_holder->vertexCount() > 0)
                m_holder->cleanup();
            m_setup = false;
        }

        void VertexArray::render(const RenderSpec& spec) {
            assert(m_setup);
            spec.render();
        }

        VertexArray::VertexArray(const GLenum primType, BaseHolder::Ptr holder) :
        m_holder(holder),
        m_prepared(false),
        m_setup(false) {}
    }
}
