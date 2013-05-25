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

#ifndef __TrenchBroom__AttributeArray__
#define __TrenchBroom__AttributeArray__

#include "TrenchBroom.h"
#include "VecMath.h"

#include "GL/GL.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T>
        class Attribute1 {
        public:
            typedef std::vector<Attribute1<T> > List;
        private:
            T m_value;
        public:
            Attribute1(const T& value) :
            m_value(value) {}
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
        class Attribute2 {
        public:
            typedef std::vector<Attribute2<T1, T2> > List;
        private:
            T1 m_value1;
            T2 m_value2;
        public:
            Attribute2(const T1& value1, const T2& value2) :
            m_value1(value1),
            m_value2(value2) {}
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
        class Attribute3 {
        public:
            typedef std::vector<Attribute3<T1, T2, T3> > List;
        private:
            T1 m_value1;
            T2 m_value2;
            T3 m_value3;
        public:
            Attribute3(const T1& value1, const T2& value2, const T3& value3) :
            m_value1(value1),
            m_value2(value2),
            m_value3(value3) {}
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
        class Attribute4 {
        public:
            typedef std::vector<Attribute4<T1, T2, T3, T4> > List;
        private:
            T1 m_value1;
            T2 m_value2;
            T3 m_value3;
            T4 m_value4;
        public:
            Attribute4(const T1& value1, const T2& value2, const T3& value3, const T4& value4) :
            m_value1(value1),
            m_value2(value2),
            m_value3(value3),
            m_value4(value4) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif

        typedef Attribute1<Vec3f> V3Attr;
        typedef Attribute2<Vec3f,Vec2f> V3T2Attr;
        typedef Attribute3<Vec3f,Vec3f,Vec2f> V3N3T2Attr;
        
        class AttributeArray {
        private:
            VboBlock* m_block;
        public:
            template <typename T>
            explicit AttributeArray(Vbo& vbo, const typename std::vector<Attribute1<T> >& data) :
            m_block(NULL) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Attribute1<T>));
                m_block->writeBuffer(0, data);
            }
            
            template <typename T1, typename T2>
            explicit AttributeArray(Vbo& vbo, const typename std::vector<Attribute2<T1, T2> >& data) :
            m_block(NULL) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Attribute2<T1, T2>));
                m_block->writeBuffer(0, data);
            }
            
            template <typename T1, typename T2, typename T3>
            explicit AttributeArray(Vbo& vbo, const typename std::vector<Attribute3<T1, T2, T3> >& data) :
            m_block(NULL) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Attribute3<T1, T2, T3>));
                m_block->writeBuffer(0, data);
            }
            
            template <typename T1, typename T2, typename T3, typename T4>
            explicit AttributeArray(Vbo& vbo, const typename std::vector<Attribute4<T1, T2, T3, T4> >& data) :
            m_block(NULL) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Attribute4<T1, T2, T3, T4>));
                m_block->writeBuffer(0, data);
            }
            
            AttributeArray(AttributeArray& other) :
            m_block(NULL) {
                std::swap(m_block, other.m_block);
            }
            
            inline AttributeArray& operator= (AttributeArray& other) {
                if (m_block != NULL) {
                    m_block->free();
                    m_block = NULL;
                }
                std::swap(m_block, other.m_block);
                return *this;
            }
            
            ~AttributeArray() {
                if (m_block != NULL) {
                    m_block->free();
                    m_block = NULL;
                }
            }
        };
    }
}

#endif /* defined(__TrenchBroom__AttributeArray__) */
