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

#ifndef TrenchBrooVertex_h
#define TrenchBrooVertex_h

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T>
        class Vertex1 {
        public:
            typedef std::vector<Vertex1<T> > List;
            
            T value;
            
            Vertex1(const T& i_value) :
            value(i_value) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif
    
#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T1, typename T2>
        class Vertex2 {
        public:
            typedef std::vector<Vertex2<T1, T2> > List;
            
            T1 value1;
            T2 value2;

            Vertex2(const T1& i_value1, const T2& i_value2) :
            value1(i_value1),
            value2(i_value2) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif

#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T1, typename T2, typename T3>
        class Vertex3 {
        public:
            typedef std::vector<Vertex3<T1, T2, T3> > List;

            T1 value1;
            T2 value2;
            T3 value3;

            Vertex3(const T1& i_value1, const T2& i_value2, const T3& i_value3) :
            value1(i_value1),
            value2(i_value2),
            value3(i_value3) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif

#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T1, typename T2, typename T3, typename T4>
        class Vertex4 {
        public:
            typedef std::vector<Vertex4<T1, T2, T3, T4> > List;

            T1 value1;
            T2 value2;
            T3 value3;
            T4 value4;

            Vertex4(const T1& i_value1, const T2& i_value2, const T3& i_value3, const T4& i_value4) :
            value1(i_value1),
            value2(i_value2),
            value3(i_value3),
            value4(i_value4) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif

        typedef Vertex1<Vec3f> VP3;
        typedef Vertex2<Vec3f,Vec2f> VP3T2;
        typedef Vertex3<Vec3f,Vec3f,Vec2f> VP3N3T2;
    }
}

#endif
