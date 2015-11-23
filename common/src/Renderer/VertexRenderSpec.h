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
        public:
            virtual ~VertexRenderSpec();
        };
        
        class SimpleVertexRenderSpec : public VertexRenderSpec {
        private:
            struct IndicesAndCounts {
                IndexArray indices;
                CountArray counts;
                
                IndicesAndCounts(size_t capacity);
                size_t size() const;
                void add(PrimType primType, GLint index, GLsizei count);
            };

            typedef std::map<PrimType, IndicesAndCounts> PrimTypeToIndexData;
        public:
            class Size {
            private:
                friend class SimpleVertexRenderSpec;
                
                typedef std::map<PrimType, size_t> PrimTypeToSize;
                PrimTypeToSize m_sizes;
            public:
                void inc(const PrimType primType);
            private:
                void initialize(PrimTypeToIndexData& data) const;
            };
        private:
            PrimTypeToIndexData m_data;
        public:
            SimpleVertexRenderSpec(const Size& size);

            void add(PrimType primType, GLint index, GLsizei count);
            
            void render(VertexArray& vertexArray) const;
            void doRender(VertexArray& vertexArray) const;
        private:
            SimpleVertexRenderSpec(const SimpleVertexRenderSpec& other);
            SimpleVertexRenderSpec& operator=(const SimpleVertexRenderSpec& other);
        };
        
        template <typename Key>
        class KeyedVertexRenderSpec : public VertexRenderSpec {
        public:
            class KeyFunc {
            public:
                virtual ~KeyFunc() {}
                virtual void before(const Key& key) const = 0;
                virtual void after(const Key& key) const = 0;
            };
        private:
            typedef std::map<Key, SimpleVertexRenderSpec> KeyToRenderSpec;
        public:
            class Size {
            private:
                friend class KeyedVertexRenderSpec;
                
                typedef std::map<Key, SimpleVertexRenderSpec::Size> KeyToSize;
                KeyToSize m_sizes;
                typename KeyToSize::iterator m_current;
            public:
                Size() : m_current(m_sizes.end()) {}
                
                void inc(const Key& key, const PrimType primType) {
                    SimpleVertexRenderSpec::Size& sizeForKey = findCurrent(key);
                    sizeForKey.inc(primType);
                }
            private:
                SimpleVertexRenderSpec::Size& findCurrent(const Key& key) {
                    if (!isCurrent(key))
                        m_current = MapUtils::findOrInsert(m_sizes, key, SimpleVertexRenderSpec::Size());
                    return m_current->second;
                }
                
                bool isCurrent(const Key& key) const {
                    if (m_current == m_sizes.end())
                        return false;
                    
                    typedef typename KeyToSize::key_compare Cmp;
                    const Cmp& cmp = m_sizes.key_comp();
                    
                    const Key& currentKey = m_current->first;
                    if (cmp(key, currentKey) || cmp(currentKey, key))
                        return false;
                    return true;
                }
            };
        private:
            KeyToRenderSpec m_data;
            typename KeyToRenderSpec::iterator m_current;
        public:
            KeyedVertexRenderSpec(const Size& size) :
            m_current(m_data.end()) {}

            void add(const Key& key, const PrimType primType, const GLint index, const GLsizei count) {
                SimpleVertexRenderSpec& current = findCurrent(key);
                current.add(primType, index, count);
            }

            void render(VertexArray& vertexArray, const KeyFunc& keyFunc) const {
                typename KeyToRenderSpec::const_iterator keyIt, keyEnd;
                for (keyIt = m_data.begin(), keyEnd = m_data.end(); keyIt != keyEnd; ++keyIt) {
                    const Key& key = keyIt->first;
                    const SimpleVertexRenderSpec& spec = keyIt->second;
                    
                    keyFunc.before(key);
                    spec.doRender(vertexArray);
                    keyFunc.after(key);
                }
            }
        private:
            SimpleVertexRenderSpec& findCurrent(const Key& key) {
                if (!isCurrent(key))
                    m_current = m_data.find(key);
                assert(m_current != m_data.end());
                return m_current->second;
            }

            bool isCurrent(const Key& key) const {
                if (m_current == m_data.end())
                    return false;
                
                typedef typename KeyToRenderSpec::key_compare Cmp;
                const Cmp& cmp = m_data.key_comp();
                
                const Key& currentKey = m_current->first;
                if (cmp(key, currentKey) || cmp(currentKey, key))
                    return false;
                return true;
            }
        private:
            KeyedVertexRenderSpec(const KeyedVertexRenderSpec& other);
            KeyedVertexRenderSpec& operator=(const KeyedVertexRenderSpec& other);
        };
    }
}

#endif /* VertexRenderSpec_h */
