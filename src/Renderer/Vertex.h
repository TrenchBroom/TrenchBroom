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

#ifndef TrenchBroom_Vertex_h
#define TrenchBroom_Vertex_h

namespace TrenchBroom {
    namespace Renderer {
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        class VertexSpec;
        
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        class Vertex {
        public:
            typedef VertexSpec<A1, A2, A3, A4, A5> Spec;
            typedef std::vector<Vertex<A1, A2, A3, A4, A5> > List;
        private:
            typename A1::ElementType m_v1;
            typename A2::ElementType m_v2;
            typename A3::ElementType m_v3;
            typename A4::ElementType m_v4;
            typename A5::ElementType m_v5;
        public:
            Vertex(const typename A1::ElementType& v1 = typename A1::ElementType(),
                   const typename A2::ElementType& v2 = typename A2::ElementType(),
                   const typename A3::ElementType& v3 = typename A3::ElementType(),
                   const typename A4::ElementType& v4 = typename A4::ElementType(),
                   const typename A5::ElementType& v5 = typename A5::ElementType()) :
            m_v1(v1),
            m_v2(v2),
            m_v3(v3),
            m_v4(v4),
            m_v5(v5) {}
        };
    }
}

#endif
