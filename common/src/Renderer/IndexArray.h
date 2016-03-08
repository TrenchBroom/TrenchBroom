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

#ifndef IndexArray_h
#define IndexArray_h

#include "CollectionUtils.h"
#include "SharedPointer.h"
#include "Renderer/GL.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class IndexArray {
        private:
            class BaseHolder {
            public:
                typedef std::tr1::shared_ptr<BaseHolder> Ptr;
                virtual ~BaseHolder() {}
                
                virtual size_t indexCount() const = 0;
                virtual size_t sizeInBytes() const = 0;
                
                virtual void prepare(Vbo& vbo) = 0;
            public:
                void render(PrimType primType, size_t offset, size_t count) const;
                
                virtual size_t indexOffset() const = 0;
            private:
                virtual void doRender(PrimType primType, size_t offset, size_t count) const = 0;
            };
            
            template <typename Index>
            class Holder : public BaseHolder {
            protected:
                typedef std::vector<Index> IndexList;
            private:
                VboBlock* m_block;
                size_t m_indexCount;
            public:
                size_t indexCount() const {
                    return m_indexCount;
                }
                
                size_t sizeInBytes() const {
                    return sizeof(Index) * m_indexCount;
                }
                
                virtual void prepare(Vbo& vbo) {
                    if (m_indexCount > 0 && m_block == NULL) {
                        ActivateVbo activate(vbo);
                        m_block = vbo.allocateBlock(sizeInBytes());
                        
                        MapVboBlock map(m_block);
                        m_block->writeBuffer(0, doGetIndices());
                    }
                }
            protected:
                Holder(const size_t indexCount) :
                m_block(NULL),
                m_indexCount(indexCount) {}
                
                virtual ~Holder() {
                    if (m_block != NULL) {
                        m_block->free();
                        m_block = NULL;
                    }
                }
            private:
                size_t indexOffset() const {
                    if (m_indexCount == 0)
                        return 0;
                    assert(m_block != NULL);
                    return m_block->offset();
                    
                }

                void doRender(PrimType primType, size_t offset, size_t count) const {
                    const GLsizei renderCount  = static_cast<GLsizei>(count);
                    const GLenum indexType     = glType<Index>();
                    const GLvoid* renderOffset = reinterpret_cast<GLvoid*>(indexOffset() + sizeof(Index) * offset);

                    glAssert(glDrawElements(primType, renderCount, indexType, renderOffset));
                }
            private:
                virtual const IndexList& doGetIndices() const = 0;
            };
            
            template <typename Index>
            class CopyHolder : public Holder<Index> {
            public:
                typedef typename Holder<Index>::IndexList IndexList;
            private:
                IndexList m_indices;
            public:
                CopyHolder(const IndexList& indices) :
                Holder<Index>(indices.size()),
                m_indices(indices) {}
                
                void prepare(Vbo& vbo) {
                    Holder<Index>::prepare(vbo);
                    VectorUtils::clearToZero(m_indices);
                }
            private:
                const IndexList& doGetIndices() const {
                    return m_indices;
                }
            };
            
            template <typename Index>
            class SwapHolder : public Holder<Index> {
            public:
                typedef typename Holder<Index>::IndexList IndexList;
            private:
                IndexList m_indices;
            public:
                SwapHolder(IndexList& indices) :
                Holder<Index>(indices.size()),
                m_indices(0) {
                    using std::swap;
                    swap(m_indices, indices);
                }
                
                void prepare(Vbo& vbo) {
                    Holder<Index>::prepare(vbo);
                    VectorUtils::clearToZero(m_indices);
                }
            private:
                const IndexList& doGetIndices() const {
                    return m_indices;
                }
            };
            
            template <typename Index>
            class RefHolder : public Holder<Index> {
            public:
                typedef typename Holder<Index>::IndexList IndexList;
            private:
                const IndexList& m_indices;
            public:
                RefHolder(const IndexList& indices) :
                Holder<Index>(indices.size()),
                m_indices(indices) {}
            private:
                const IndexList& doGetIndices() const {
                    return m_indices;
                }
            };
        private:
            BaseHolder::Ptr m_holder;
            bool m_prepared;
        public:
            explicit IndexArray();
            
            template <typename Index>
            static IndexArray copy(const std::vector<Index>& indices) {
                return IndexArray(BaseHolder::Ptr(new CopyHolder<Index>(indices)));
            }
            
            template <typename Index>
            static IndexArray swap(std::vector<Index>& indices) {
                return IndexArray(BaseHolder::Ptr(new SwapHolder<Index>(indices)));
            }
            
            template <typename Index>
            static IndexArray ref(const std::vector<Index>& indices) {
                return IndexArray(BaseHolder::Ptr(new RefHolder<Index>(indices)));
            }

            IndexArray& operator=(IndexArray other);
            friend void swap(IndexArray& left, IndexArray& right);
            
            bool empty() const;
            size_t sizeInBytes() const;
            size_t indexCount() const;
            
            bool prepared() const;
            void prepare(Vbo& vbo);
            
            void render(PrimType primType, size_t offset, size_t count) const;
        private:
            IndexArray(BaseHolder::Ptr holder);
        };
    }
}

#endif /* IndexArray_h */
