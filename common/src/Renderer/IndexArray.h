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

#include "SharedPointer.h"
#include "Renderer/GL.h"
#include "Renderer/VertexArray.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class IndexArray {
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
                PT_Polygons         = GL_POLYGON
            } PrimType;
        private:
            struct IndicesAndCounts {
                VertexArray::IndexArray indices;
                VertexArray::CountArray counts;
                
                IndicesAndCounts(size_t capacity);
                IndicesAndCounts(GLint index, GLsizei count);
                size_t size() const;
                void add(PrimType primType, GLint index, GLsizei count);
            };
            
            typedef std::map<PrimType, IndicesAndCounts> PrimTypeToIndexData;
            typedef std::tr1::shared_ptr<PrimTypeToIndexData> PrimTypeToIndexDataPtr;
        public:
            class Size {
            private:
                friend class IndexArray;
                
                typedef std::map<PrimType, size_t> PrimTypeToSize;
                PrimTypeToSize m_sizes;
            public:
                void inc(const PrimType primType, size_t count = 1);
            private:
                void initialize(PrimTypeToIndexData& data) const;
            };
        private:
            PrimTypeToIndexDataPtr m_data;
        public:
            IndexArray(const Size& size);
            IndexArray(PrimType primType, GLint index, GLsizei count);
            void add(PrimType primType, GLint index, GLsizei count);
            
            void render(const VertexArray& vertexArray) const;
        };
    }
}

#endif /* IndexArray_p */
