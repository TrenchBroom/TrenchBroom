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
        
        struct GetVertexComponent1 {
            template <typename V> const typename V::Spec::A1::ElementType& operator()(const V& v) const { return v.v1; }
        };
        
        struct GetVertexComponent2 {
            template <typename V> const typename V::Spec::A2::ElementType& operator()(const V& v) const { return v.v2; }
        };
        
        struct GetVertexComponent3 {
            template <typename V> const typename V::Spec::A3::ElementType& operator()(const V& v) const { return v.v3; }
        };
        
        struct GetVertexComponent4 {
            template <typename V> const typename V::Spec::A4::ElementType& operator()(const V& v) const { return v.v4; }
        };
        
        struct GetVertexComponent5 {
            template <typename V> const typename V::Spec::A5::ElementType& operator()(const V& v) const { return v.v5; }
        };
        
        template <typename A1>
        class Vertex1 {
        public:
            typedef VertexSpec1<A1> Spec;
            typedef std::vector<Vertex1<A1> > List;
            
            typename A1::ElementType v1;
            
            Vertex1() {}

            Vertex1(const typename A1::ElementType& i_v1) :
            v1(i_v1) {}
            
            bool operator==(const Vertex1<A1>& other) const {
                return v1 == other.v1;
            }
            
            static List fromLists(const std::vector<typename A1::ElementType>& list,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1) {
                List result;
                size_t index1 = offset1;
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex1(list[index1]));
                    index1 += stride1;
                }
                return result;
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
            
            bool operator==(const Vertex2<A1, A2>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2);
            }
            
            static List fromLists(const std::vector<typename A1::ElementType>& list1,
                                  const std::vector<typename A2::ElementType>& list2,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1) {
                List result;
                size_t index1 = offset1, index2 = offset2;
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex2(list1[index1], list2[index2]));
                    index1 += stride1;
                    index2 += stride2;
                }
                return result;
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
            
            bool operator==(const Vertex3<A1, A2, A3>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2 &&
                        v3 == other.v3);
            }
            
            static List fromLists(const std::vector<typename A1::ElementType>& list1,
                                  const std::vector<typename A2::ElementType>& list2,
                                  const std::vector<typename A3::ElementType>& list3,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1,
                                  const size_t offset3 = 0, const size_t stride3 = 1) {
                List result;
                size_t index1 = offset1, index2 = offset2, index3 = offset3;
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex3(list1[index1], list2[index2], list3[index3]));
                    index1 += stride1;
                    index2 += stride2;
                    index3 += stride3;
                }
                return result;
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
            
            bool operator==(const Vertex4<A1, A2, A3, A4>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2 &&
                        v3 == other.v3 &&
                        v4 == other.v4);
            }
            
            static List fromLists(const std::vector<typename A1::ElementType>& list1,
                                  const std::vector<typename A2::ElementType>& list2,
                                  const std::vector<typename A3::ElementType>& list3,
                                  const std::vector<typename A4::ElementType>& list4,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1,
                                  const size_t offset3 = 0, const size_t stride3 = 1,
                                  const size_t offset4 = 0, const size_t stride4 = 1) {
                List result;
                size_t index1 = offset1, index2 = offset2, index3 = offset3, index4 = offset4;
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex4(list1[index1], list2[index2], list3[index3], list4[index4]));
                    index1 += stride1;
                    index2 += stride2;
                    index3 += stride3;
                    index4 += stride4;
                }
                return result;
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
            
            bool operator==(const Vertex5<A1, A2, A3, A4, A5>& other) const {
                return (v1 == other.v1 &&
                        v2 == other.v2 &&
                        v3 == other.v3 &&
                        v4 == other.v4 &&
                        v5 == other.v5);
            }
            
            static List fromLists(const std::vector<typename A1::ElementType>& list1,
                                  const std::vector<typename A2::ElementType>& list2,
                                  const std::vector<typename A3::ElementType>& list3,
                                  const std::vector<typename A4::ElementType>& list4,
                                  const std::vector<typename A5::ElementType>& list5,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1,
                                  const size_t offset3 = 0, const size_t stride3 = 1,
                                  const size_t offset4 = 0, const size_t stride4 = 1,
                                  const size_t offset5 = 0, const size_t stride5 = 1) {
                List result;
                size_t index1 = offset1, index2 = offset2, index3 = offset3, index4 = offset4, index5 = offset5;
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex5(list1[index1], list2[index2], list3[index3], list4[index4], list5[index5]));
                    index1 += stride1;
                    index2 += stride2;
                    index3 += stride3;
                    index4 += stride4;
                    index5 += stride5;
                }
                return result;
            }
        };
    }
}

#endif
