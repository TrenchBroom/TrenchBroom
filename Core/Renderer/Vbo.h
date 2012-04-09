/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_Vbo_h
#define TrenchBroom_Vbo_h

#include <OpenGL/gl.h>
#include <vector>
#include "VecMath.h"

using namespace std;

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class VboBlock;
        
        class VboBlock {
        private:
            Vbo& m_vbo;
            void insertBetween(VboBlock* previous, VboBlock* next);
            friend class Vbo;
        public:
            unsigned int address;
            unsigned int capacity;
            bool free;
            VboBlock* previous;
            VboBlock* next;

            VboBlock(Vbo& vbo, int address, int capacity);
            int writeBuffer(const unsigned char* buffer, int offset, int length);
            int writeByte(unsigned char b, int offset);
            int writeFloat(float f, int offset);
            int writeColor(const Vec4f& color, int offset);
            int writeVec(const Vec4f& vec, int offset);
            int writeVec(const Vec3f& vec, int offset);
            int writeVec(const Vec2f& vec, int offset);
            void freeBlock();
        };
        
        class Vbo {
        private:
            unsigned int m_totalCapacity;
            unsigned int m_freeCapacity;
            vector<VboBlock*> m_freeBlocks;
            VboBlock* m_first;
            VboBlock* m_last;
            unsigned char* m_buffer;
            GLuint m_vboId;
            GLenum m_type;
            bool m_active;
            bool m_mapped;
            int findFreeBlockInRange(int capacity, int start, int length);
            int findFreeBlock(int capacity);
            void insertFreeBlock(VboBlock& block);
            void removeFreeBlock(VboBlock& block);
            void resizeVbo(int newCapacity);
            void resizeBlock(VboBlock& block, int newCapacity);
            VboBlock* packBlock(VboBlock& block);
            friend class VboBlock;
        public:
            Vbo(GLenum type, int capacity);
            ~Vbo();
            void activate();
            void deactivate();
            void map();
            void unmap();
            VboBlock& allocBlock(int capacity);
            VboBlock& freeBlock(VboBlock& block);
            void pack();
        };
    }
}

#endif
