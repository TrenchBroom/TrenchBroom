/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
                return fromLists(std::begin(list), count, offset1, stride1);
            }

            template <typename I1>
            static List fromLists(I1 cur1,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1) {
                List result;
                result.reserve(count);
                std::advance(cur1, static_cast<typename I1::difference_type>(offset1));
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex1(*cur1));
                    std::advance(cur1, static_cast<typename I1::difference_type>(stride1));
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
                return fromLists(std::begin(list1), std::begin(list2), count, offset1, stride1, offset2, stride2);
            }

            template <typename I1, typename I2>
            static List fromLists(I1 cur1, I2 cur2,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1) {
                List result;
                result.reserve(count);
                std::advance(cur1, static_cast<typename I1::difference_type>(offset1));
                std::advance(cur2, static_cast<typename I2::difference_type>(offset2));
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex2(*cur1, *cur2));
                    std::advance(cur1, static_cast<typename I1::difference_type>(stride1));
                    std::advance(cur2, static_cast<typename I2::difference_type>(stride2));
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
                return fromLists(std::begin(list1), std::begin(list2), std::begin(list3), count, offset1, stride1, offset2, stride2, offset3, stride3);
            }

            template <typename I1, typename I2, typename I3>
            static List fromLists(I1 cur1, I2 cur2, I3 cur3,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1,
                                  const size_t offset3 = 0, const size_t stride3 = 1) {
                List result;
                result.reserve(count);
                std::advance(cur1, static_cast<typename I1::difference_type>(offset1));
                std::advance(cur2, static_cast<typename I2::difference_type>(offset2));
                std::advance(cur3, static_cast<typename I3::difference_type>(offset3));
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex3(*cur1, *cur2, *cur3));
                    std::advance(cur1, static_cast<typename I1::difference_type>(stride1));
                    std::advance(cur2, static_cast<typename I2::difference_type>(stride2));
                    std::advance(cur3, static_cast<typename I3::difference_type>(stride3));
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
                return fromLists(std::begin(list1), std::begin(list2), std::begin(list3), std::begin(list4), count, offset1, stride1, offset2, stride2, offset3, stride3, offset4, stride4);
            }
            
            template <typename I1, typename I2, typename I3, typename I4>
            static List fromLists(I1 cur1, I2 cur2, I3 cur3, I4 cur4,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1,
                                  const size_t offset3 = 0, const size_t stride3 = 1,
                                  const size_t offset4 = 0, const size_t stride4 = 1) {
                List result;
                result.reserve(count);
                std::advance(cur1, static_cast<typename I1::difference_type>(offset1));
                std::advance(cur2, static_cast<typename I2::difference_type>(offset2));
                std::advance(cur3, static_cast<typename I3::difference_type>(offset3));
                std::advance(cur4, static_cast<typename I4::difference_type>(offset4));
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex4(*cur1, *cur2, *cur3, *cur4));
                    std::advance(cur1, static_cast<typename I1::difference_type>(stride1));
                    std::advance(cur2, static_cast<typename I2::difference_type>(stride2));
                    std::advance(cur3, static_cast<typename I3::difference_type>(stride3));
                    std::advance(cur4, static_cast<typename I4::difference_type>(stride4));
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
                return fromLists(std::begin(list1), std::begin(list2), std::begin(list3), std::begin(list4), std::begin(list5), count, offset1, stride1, offset2, stride2, offset3, stride3, offset4, stride4, offset5, stride5);
            }
            
            template <typename I1, typename I2, typename I3, typename I4, typename I5>
            static List fromLists(I1 cur1, I2 cur2, I3 cur3, I4 cur4, I5 cur5,
                                  const size_t count,
                                  const size_t offset1 = 0, const size_t stride1 = 1,
                                  const size_t offset2 = 0, const size_t stride2 = 1,
                                  const size_t offset3 = 0, const size_t stride3 = 1,
                                  const size_t offset4 = 0, const size_t stride4 = 1,
                                  const size_t offset5 = 0, const size_t stride5 = 1) {
                List result;
                result.reserve(count);
                std::advance(cur1, static_cast<typename I1::difference_type>(offset1));
                std::advance(cur2, static_cast<typename I2::difference_type>(offset2));
                std::advance(cur3, static_cast<typename I3::difference_type>(offset3));
                std::advance(cur4, static_cast<typename I4::difference_type>(offset4));
                std::advance(cur5, static_cast<typename I5::difference_type>(offset5));
                for (size_t i = 0; i < count; ++i) {
                    result.push_back(Vertex5(*cur1, *cur2, *cur3, *cur4, *cur5));
                    std::advance(cur1, static_cast<typename I1::difference_type>(stride1));
                    std::advance(cur2, static_cast<typename I2::difference_type>(stride2));
                    std::advance(cur3, static_cast<typename I3::difference_type>(stride3));
                    std::advance(cur4, static_cast<typename I4::difference_type>(stride4));
                    std::advance(cur5, static_cast<typename I5::difference_type>(stride5));
                }
                return result;
            }
        };
    }
}

#endif
