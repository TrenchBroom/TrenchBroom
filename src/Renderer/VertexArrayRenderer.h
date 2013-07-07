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

#ifndef __TrenchBroom__VertexArrayRenderer__
#define __TrenchBroom__VertexArrayRenderer__

#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class AttributeSpec {
        public:
            typedef enum {
                User,
                Position,
                Normal,
                Color,
                TexCoord0,
                TexCoord1,
                TexCoord2,
                TexCoord3
            } AttributeType;
        private:
            AttributeType m_attributeType;
            GLenum m_dataType;
            GLint m_size;
        public:
            AttributeSpec(const AttributeType attributeType, const GLenum dataType, const GLint size);
            
            static const AttributeSpec& P2();
            static const AttributeSpec& P3();
            static const AttributeSpec& N3();
            static const AttributeSpec& C4();
            static const AttributeSpec& T02();

            AttributeType attributeType() const;
            GLenum dataType() const;
            GLint size() const;
            size_t sizeInBytes() const;
            
            void setup(const size_t index, const size_t stride, const size_t offset) const;
            void cleanup(const size_t index) const;
        };
        
        class VertexSpec {
        private:
            typedef std::vector<AttributeSpec> AttributeSpecList;
            AttributeSpecList m_attributeSpecs;
            size_t m_totalSize;
        public:
            VertexSpec();
            VertexSpec(const AttributeSpec& attributeSpec1);
            VertexSpec(const AttributeSpec& attributeSpec1, const AttributeSpec& attributeSpec2);
            VertexSpec(const AttributeSpec& attributeSpec1, const AttributeSpec& attributeSpec2, const AttributeSpec& attributeSpec3);
            VertexSpec(const AttributeSpec& attributeSpec1, const AttributeSpec& attributeSpec2, const AttributeSpec& attributeSpec3, const AttributeSpec& attributeSpec4);
            
            static const VertexSpec P3();
            static const VertexSpec P3C4();
            static const VertexSpec P3T2();
            static const VertexSpec P3N3T2();
            
            const AttributeSpec& operator[] (const size_t index) const;
            size_t size() const;
            
            void setup(const size_t baseOffset) const;
            void cleanup() const;
        };
        
        class VertexArrayRenderer {
        public:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;
        private:
            VertexSpec m_vertexSpec;
            GLenum m_primType;
            VertexArray m_vertexArray;
            IndexArray m_indices;
            CountArray m_counts;
        public:
            VertexArrayRenderer(const VertexSpec& vertexSpec, const GLenum primType);
            VertexArrayRenderer(const VertexSpec& vertexSpec, const GLenum primType, VertexArray& vertexArray);
            VertexArrayRenderer(const VertexSpec& vertexSpec, const GLenum primType, VertexArray& vertexArray, const IndexArray& indices, const CountArray& counts);
            VertexArrayRenderer(VertexArrayRenderer& other);
            
            VertexArrayRenderer& operator= (VertexArrayRenderer other);
            inline friend void swap(VertexArrayRenderer& left, VertexArrayRenderer& right) {
                using std::swap;
                swap(left.m_vertexSpec, right.m_vertexSpec);
                swap(left.m_primType, right.m_primType);
                swap(left.m_vertexArray, right.m_vertexArray);
                swap(left.m_indices, right.m_indices);
                swap(left.m_counts, right.m_counts);
            }
            
            void render();
        };
    }
}

#endif /* defined(__TrenchBroom__VertexArrayRenderer__) */
