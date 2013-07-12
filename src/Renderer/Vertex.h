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
        template <typename A1>
        class VertexSpec1;
        template <typename A1, typename A2>
        class VertexSpec2;
        template <typename A1, typename A2, typename A3>
        class VertexSpec3;
        template <typename A1, typename A2, typename A3, typename A4>
        class VertexSpec4;
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        class VertexSpec5;
        
        template <typename A1>
        class Vertex1 {
        public:
            typedef VertexSpec1<A1> Spec;
            typedef std::vector<Vertex1<A1> > List;
            
            typename A1::ElementType v1;
            
            Vertex1() {}

            Vertex1(const typename A1::ElementType& i_v1) :
            v1(i_v1) {}
            
            inline bool operator==(const Vertex1<A1>& other) const {
                return v1 == other.v1;
            }
        };
        
        template <typename A1, typename A2>
        class Vertex2 {
        public:
            typedef VertexSpec2<A1, A2> Spec;
            typedef std::vector<Vertex2<A1, A2> > List;
            
            typename A1::ElementType v1;
            typename A2::ElementType v2;
            
            Vertex2() {}

            Vertex2(const typename A1::ElementType& i_v1,
                    const typename A2::ElementType& i_v2) :
            v1(i_v1),
            v2(i_v2) {}
            
            inline bool operator==(const Vertex2<A1, A2>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2);
            }
        };
        
        template <typename A1, typename A2, typename A3>
        class Vertex3 {
        public:
            typedef VertexSpec3<A1, A2, A3> Spec;
            typedef std::vector<Vertex3<A1, A2, A3> > List;
            
            typename A1::ElementType v1;
            typename A2::ElementType v2;
            typename A3::ElementType v3;
            
            Vertex3() {}
            
            Vertex3(const typename A1::ElementType& i_v1,
                    const typename A2::ElementType& i_v2,
                    const typename A3::ElementType& i_v3) :
            v1(i_v1),
            v2(i_v2),
            v3(i_v3) {}
            
            inline bool operator==(const Vertex3<A1, A2, A3>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2 &&
                        v3 == other.v3);
            }
        };
        
        template <typename A1, typename A2, typename A3, typename A4>
        class Vertex4 {
        public:
            typedef VertexSpec4<A1, A2, A3, A4> Spec;
            typedef std::vector<Vertex4<A1, A2, A3, A4> > List;
            
            typename A1::ElementType v1;
            typename A2::ElementType v2;
            typename A3::ElementType v3;
            typename A4::ElementType v4;
            
            Vertex4() {}
            
            Vertex4(const typename A1::ElementType& i_v1,
                    const typename A2::ElementType& i_v2,
                    const typename A3::ElementType& i_v3,
                    const typename A4::ElementType& i_v4) :
            v1(i_v1),
            v2(i_v2),
            v3(i_v3),
            v4(i_v4) {}
            
            inline bool operator==(const Vertex4<A1, A2, A3, A4>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2 &&
                        v3 == other.v3 &&
                        v4 == other.v4);
            }
        };

        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        class Vertex5 {
        public:
            typedef VertexSpec5<A1, A2, A3, A4, A5> Spec;
            typedef std::vector<Vertex5<A1, A2, A3, A4, A5> > List;

            typename A1::ElementType v1;
            typename A2::ElementType v2;
            typename A3::ElementType v3;
            typename A4::ElementType v4;
            typename A5::ElementType v5;

            Vertex5() {}
            
            Vertex5(const typename A1::ElementType& i_v1,
                    const typename A2::ElementType& i_v2,
                    const typename A3::ElementType& i_v3,
                    const typename A4::ElementType& i_v4,
                    const typename A5::ElementType& i_v5) :
            v1(i_v1),
            v2(i_v2),
            v3(i_v3),
            v4(i_v4),
            v5(i_v5) {}
            
            inline bool operator==(const Vertex5<A1, A2, A3, A4, A5>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2 &&
                        v3 == other.v3 &&
                        v4 == other.v4 &&
                        v5 == other.v5);
            }
        };
    }
}

#endif
