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

#ifndef VertexRenderSpec_h
#define VertexRenderSpec_h

#include "CollectionUtils.h"
#include "Renderer/GL.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        
        template <typename Key = int>
        class VertexRenderSpec {
        public:
            typedef enum {
                PT_Points           = GL_POINTS,
                PT_Lines            = GL_LINES,
                PT_LineStrips       = GL_LINE_STRIP,
                PT_LineLoops        = GL_LINE_LOOP,
                PT_Triangles        = GL_TRIANGLES,
                PT_TriangleFans     = GL_TRIANGLE_FAN,
                PT_TriangleStrips   = GL_TRIANGLE_STRIP,
                PT_Quads            = GL_QUADS,
                PT_QuadStrips       = GL_QUAD_STRIP,
                PT_Polygons =       GL_POLYGON
            } PrimType;
            
            typedef VertexArray::IndexArray IndexArray;
            typedef VertexArray::CountArray CountArray;
        private:
            struct IndicesAndCounts {
                IndexArray indices;
                CountArray counts;
                
                IndicesAndCounts(const size_t capacity) :
                indices(0),
                counts(0) {
                    indices.reserve(capacity);
                    counts.reserve(capacity);
                }
                
                size_t size() const {
                    return indices.size();
                }
                
                void add(const PrimType primType, const GLint index, const GLsizei count) {
                    switch (primType) {
                        case PT_Points:
                        case PT_Lines:
                        case PT_Triangles:
                        case PT_Quads: {
                            if (size() == 1) {
                                const GLint myIndex = indices.front();
                                GLsizei& myCount = counts.front();
                                
                                if (index == myIndex + myCount) {
                                    myCount += count;
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
                            assert(indices.capacity() > indices.size());
                            indices.push_back(index);
                            counts.push_back(count);
                            break;
                    }
                }
            };
            
            typedef std::map<PrimType, IndicesAndCounts> PrimTypeIndexData;
            typedef std::map<Key, PrimTypeIndexData> KeyIndexData;
            
            KeyIndexData m_data;
            typename KeyIndexData::iterator m_current;
        public:
            class Size {
            private:
                typedef std::map<PrimType, size_t> PrimTypeSize;
                typedef std::map<Key, PrimTypeSize> KeySize;
                
                KeySize m_sizes;
                typename KeySize::iterator m_current;
            public:
                Size() : m_current(m_sizes.end()) {}
                
                void inc(const Key& key, const PrimType primType) {
                    if (!isCurrent(key))
                        m_current = MapUtils::findOrInsert(m_sizes, key, PrimTypeSize());
                    
                    PrimTypeSize& primTypeSize = m_current->second;
                    typename PrimTypeSize::iterator primIt = MapUtils::findOrInsert(primTypeSize, primType, 0);
                    ++primIt->second;
                }
            private:
                bool isCurrent(const Key& key) const {
                    if (m_current == m_sizes.end())
                        return false;
                    
                    typedef typename KeyIndexData::key_compare Cmp;
                    const Cmp& cmp = m_sizes.key_comp();
                    
                    const Key& currentKey = m_current->first;
                    if (cmp(key, currentKey) || cmp(currentKey, key))
                        return false;
                    return true;
                }
                
                void initialize(KeyIndexData& keyIndexData) const {
                    typename KeySize::const_iterator keyIt, keyEnd;
                    for (keyIt = m_sizes.begin(), keyEnd = m_sizes.end(); keyIt != keyEnd; ++keyIt) {
                        const Key& key = keyIt->first;
                        const PrimTypeSize& primTypeSize = keyIt->second;
                        PrimTypeIndexData& primTypeIndexData = keyIndexData[key];
                        
                        typename PrimTypeSize::const_iterator primIt, primEnd;
                        for (primIt = primTypeSize.begin(), primEnd = primTypeSize.end(); primIt != primEnd; ++primIt) {
                            const PrimType primType = primIt->first;
                            const size_t size = primIt->second;
                            primTypeIndexData.insert(std::make_pair(primType, IndicesAndCounts(size)));
                        }
                    }
                }
            };
        public:
            VertexRenderSpec(const Size& size) :
            m_current(m_data.end()) {
                size.initialize(m_data);
            }
            
            void add(const Key& key, const PrimType primType, const GLint index, const GLsizei count) {
                IndicesAndCounts& currentData = findData(key, primType);
                currentData.add(index, count);
            }
        private:
            IndicesAndCounts& findData(const Key& key, const PrimType primType) {
                if (!isCurrent(key))
                    m_current = m_data.find(key);
                assert(m_current != m_data.end());
                
                PrimTypeIndexData& primTypeIndexData = m_current->second;
                
                typename PrimTypeIndexData::iterator primIt = primTypeIndexData.find(primType);
                assert(primIt != primTypeIndexData.end());
                
                return primIt->second;
            }
            
            bool isCurrent(const Key& key) const {
                if (m_current == m_data.end())
                    return false;
                
                typedef typename KeyIndexData::key_compare Cmp;
                const Cmp& cmp = m_data.key_comp();
                
                const Key& currentKey = m_current->first;
                if (cmp(key, currentKey) || cmp(currentKey, key))
                    return false;
                return true;
            }
            
            void render(VertexArray& vertexArray) const {
                if (!vertexArray.setup())
                    return;
                
                typename KeyIndexData::const_iterator keyIt, keyEnd;
                for (keyIt = m_data.begin(), keyEnd = m_data.end(); keyIt != keyEnd; ++keyIt) {
                    const PrimTypeIndexData& keyData = keyIt->second;
                    
                    typename PrimTypeIndexData::const_iterator primIt, primEnd;
                    for (primIt = keyData.begin(), primEnd = keyData.end(); primIt != primEnd; ++primIt) {
                        const PrimType primType = primIt->first;
                        const IndicesAndCounts& indicesAndCounts = primIt->second;
                        vertexArray.render(primType, indicesAndCounts.indices, indicesAndCounts.counts);
                    }
                }
                
                vertexArray.cleanup();
            }
        };
    }
}

#endif /* VertexRenderSpec_h */
