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

#ifndef TrenchBroom_VertexSpec
#define TrenchBroom_VertexSpec

#include "Renderer/GL.h"
#include "Renderer/AttributeSpec.h"
#include "Renderer/Vertex.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        template <typename _A1>
        class VertexSpec1 {
        public:
            using A1 = _A1;
            using Vertex = Vertex1<_A1>;
            static const size_t Size;
        public:
            static void setup(const size_t baseOffset) {
                _A1::setup(0, Size, baseOffset);
            }
            
            static void cleanup() {
                _A1::cleanup(0);
            }
        private:
            VertexSpec1();
        };
        
        template <typename A1>
        const size_t VertexSpec1<A1>::Size = sizeof(VertexSpec1<A1>::Vertex);

        template <typename _A1, typename _A2>
        class VertexSpec2 {
        public:
            using A1 = _A1;
            using A2 = _A2;
            using Vertex = Vertex2<_A1, _A2>;
            static const size_t Size;
        public:
            static void setup(const size_t baseOffset) {
                _A1::setup(0, Size, baseOffset);
                _A2::setup(1, Size, baseOffset + _A1::Size);
            }
            
            static void cleanup() {
                _A2::cleanup(1);
                _A1::cleanup(0);
            }
        private:
            VertexSpec2();
        };
        
        template <typename A1, typename A2>
        const size_t VertexSpec2<A1, A2>::Size = sizeof(VertexSpec2<A1, A2>::Vertex);

        template <typename _A1, typename _A2, typename _A3>
        class VertexSpec3 {
        public:
            using A1 = _A1;
            using A2 = _A2;
            using A3 = _A3;
            using Vertex = Vertex3<_A1, _A2, _A3>;
            static const size_t Size;
        public:
            static void setup(const size_t baseOffset) {
                _A1::setup(0, Size, baseOffset);
                _A2::setup(1, Size, baseOffset + _A1::Size);
                _A3::setup(2, Size, baseOffset + _A1::Size + _A2::Size);
            }
            
            static void cleanup() {
                _A3::cleanup(2);
                _A2::cleanup(1);
                _A1::cleanup(0);
            }
        private:
            VertexSpec3();
        };
        
        template <typename A1, typename A2, typename A3>
        const size_t VertexSpec3<A1, A2, A3>::Size = sizeof(VertexSpec3<A1, A2, A3>::Vertex);

        template <typename _A1, typename _A2, typename _A3, typename _A4>
        class VertexSpec4 {
        public:
            using A1 = _A1;
            using A2 = _A2;
            using A3 = _A3;
            using A4 = _A4;
            using Vertex = Vertex4<_A1, _A2, _A3, _A4>;
            static const size_t Size;
        public:
            static void setup(const size_t baseOffset) {
                _A1::setup(0, Size, baseOffset);
                _A2::setup(1, Size, baseOffset + _A1::Size);
                _A3::setup(2, Size, baseOffset + _A1::Size + _A2::Size);
                _A4::setup(3, Size, baseOffset + _A1::Size + _A2::Size + _A3::Size);
            }
            
            static void cleanup() {
                _A4::cleanup(3);
                _A3::cleanup(2);
                _A2::cleanup(1);
                _A1::cleanup(0);
            }
        private:
            VertexSpec4();
        };
        
        template <typename A1, typename A2, typename A3, typename A4>
        const size_t VertexSpec4<A1, A2, A3, A4>::Size = sizeof(VertexSpec4<A1, A2, A3, A4>::Vertex);

        template <typename _A1, typename _A2, typename _A3, typename _A4, typename _A5>
        class VertexSpec5 {
        public:
            using A1 = _A1;
            using A2 = _A2;
            using A3 = _A3;
            using A4 = _A4;
            using A5 = _A5;
            using Vertex = Vertex5<_A1, _A2, _A3, _A4, _A5>;
            static const size_t Size;
        public:
            static void setup(const size_t baseOffset) {
                _A1::setup(0, Size, baseOffset);
                _A2::setup(1, Size, baseOffset + _A1::Size);
                _A3::setup(2, Size, baseOffset + _A1::Size + _A2::Size);
                _A4::setup(3, Size, baseOffset + _A1::Size + _A2::Size + _A3::Size);
                _A5::setup(4, Size, baseOffset + _A1::Size + _A2::Size + _A3::Size + _A4::Size);
            }
            
            static void cleanup() {
                _A5::cleanup(4);
                _A4::cleanup(3);
                _A3::cleanup(2);
                _A2::cleanup(1);
                _A1::cleanup(0);
            }
        private:
            VertexSpec5();
        };
        
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        const size_t VertexSpec5<A1, A2, A3, A4, A5>::Size = sizeof(VertexSpec5<A1, A2, A3, A4, A5>::Vertex);

        namespace VertexSpecs {
            using P2     = VertexSpec1<AttributeSpecs::P2>;
            using P3     = VertexSpec1<AttributeSpecs::P3>;
            using P2C4   = VertexSpec2<AttributeSpecs::P2, AttributeSpecs::C4>;
            using P3C4   = VertexSpec2<AttributeSpecs::P3, AttributeSpecs::C4>;
            using P2T2   = VertexSpec2<AttributeSpecs::P2, AttributeSpecs::T02>;
            using P3T2   = VertexSpec2<AttributeSpecs::P3, AttributeSpecs::T02>;
            using P2T2C4 = VertexSpec3<AttributeSpecs::P2, AttributeSpecs::T02, AttributeSpecs::C4>;
            using P3T2C4 = VertexSpec3<AttributeSpecs::P3, AttributeSpecs::T02, AttributeSpecs::C4>;
            using P3N    = VertexSpec2<AttributeSpecs::P3, AttributeSpecs::N>;
            using P3NC4  = VertexSpec3<AttributeSpecs::P3, AttributeSpecs::N, AttributeSpecs::C4>;
            using P3NT2  = VertexSpec3<AttributeSpecs::P3, AttributeSpecs::N, AttributeSpecs::T02>;
        }
    }
}

#endif /* defined(TrenchBroom_VertexSpec) */
