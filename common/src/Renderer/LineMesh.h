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

#ifndef TrenchBroom_LineMesh_h
#define TrenchBroom_LineMesh_h

#include "Renderer/IndexedVertexList.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        
        struct LineMeshRenderData {
            VertexArray lines;
            VertexArray lineStrips;
            VertexArray lineLoops;
        };
        
        template <class VertexSpec>
        class LineMesh {
        private:
            typedef enum {
                LineType_Lines,
                LineType_Strip,
                LineType_Loop,
                LineType_Unset
            } LineType;

            typedef typename VertexSpec::Vertex::List VertexList;
            typedef IndexedVertexList<VertexSpec> IndexedList;
            
            VertexList m_lines;
            IndexedList m_lineStrips;
            IndexedList m_lineLoops;
            LineType m_currentType;
        public:
            LineMesh() : m_currentType(LineType_Unset) {}
            
            LineMeshRenderData renderData() {
                LineMeshRenderData result;
                if (!m_lines.empty())
                    result.lines = VertexArray::swap(GL_LINES, m_lines);
                if (!m_lineStrips.empty())
                    result.lineStrips = VertexArray::swap(GL_LINE_STRIP, m_lineStrips.vertices(), m_lineStrips.indices(), m_lineStrips.counts());
                if (!m_lineLoops.empty())
                    result.lineLoops = VertexArray::swap(GL_LINE_LOOP, m_lineLoops.vertices(), m_lineLoops.indices(), m_lineLoops.counts());
                return result;
            }
            
            void beginLines() {
                begin(LineType_Lines);
            }
            
            void endLines() {
                assert(m_currentType == LineType_Lines);
                end();
            }
            
            void addLine(const typename VertexSpec::Vertex& v1, const typename VertexSpec::Vertex& v2) {
                assert(m_currentType == LineType_Lines);
                m_lines.push_back(v1);
                m_lines.push_back(v2);
            }
            
            void addLines(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == LineType_Lines);
                assert(vertices.size() % 2 == 0);
                VectorUtils::append(m_lines, vertices);
            }
            
            void addLineStrips(const IndexedList& strips) {
                beginLineStrip();
                m_lineStrips.addPrimitives(strips);
                endLineStrip();
            }

            void beginLineStrip() {
                begin(LineType_Strip);
            }
            
            void addVertexToStrip(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == LineType_Strip);
                m_lineStrips.addVertex(v);
            }
            
            void addVerticesToStrip(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == LineType_Strip);
                m_lineStrips.addVertices(vertices);
            }
            
            void addLineStrip(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == LineType_Strip);
                m_lineStrips.addPrimitive(vertices);
            }
            
            void endLineStrip() {
                assert(m_currentType == LineType_Strip);
                m_lineStrips.endPrimitive();
                end();
            }

            void addLineLoops(const IndexedList& strips) {
                beginLineLoop();
                m_lineLoops.addPrimitives(strips);
                endLineLoop();
            }
            
            void beginLineLoop() {
                begin(LineType_Loop);
            }
            
            void addVertexToLoop(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == LineType_Loop);
                m_lineLoops.addVertex(v);
            }
            
            void addVerticesToLoop(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == LineType_Loop);
                m_lineLoops.addVertices(vertices);
            }
            
            void addLineLooop(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == LineType_Loop);
                m_lineLoops.addPrimitive(vertices);
            }
            
            void endLineLoop() {
                assert(m_currentType == LineType_Loop);
                m_lineLoops.endPrimitive();
                end();
            }
            
            void clear() {
                m_lines.clear();
                m_lineStrips.clear();
                m_lineLoops.clear();
            }
        private:
            void begin(const LineType type) {
                assert(m_currentType == LineType_Unset);
                assert(type != LineType_Unset);
                m_currentType = type;
            }
            
            void end() {
                assert(m_currentType != LineType_Unset);
                m_currentType = LineType_Unset;
            }
        };
    }
}

#endif
