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

#include <vector>

#include "Utilities/VecMath.h"
#include "GL/GLee.h"

// #define _DEBUG_VBO 1

using namespace std;

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class VboBlock;

        class VboBlock {
        private:
            Vbo& m_vbo;
            void insertBetween(VboBlock* previousBlock, VboBlock* nextBlock);
            friend class Vbo;
        public:
            unsigned int address;
            unsigned int capacity;
            bool free;
            VboBlock* previous;
            VboBlock* next;

            VboBlock(Vbo& vbo, int address, int capacity);
            unsigned int writeBuffer(const unsigned char* buffer, unsigned int offset, unsigned int length);
            unsigned int writeByte(unsigned char b, unsigned int offset);
            unsigned int writeFloat(float f, unsigned int offset);
            unsigned int writeUInt32(unsigned int i, unsigned int offset);
            unsigned int writeColor(const Vec4f& color, unsigned int offset);
            unsigned int writeVec(const Vec4f& vec, unsigned int offset);
            unsigned int writeVec(const Vec3f& vec, unsigned int offset);
            unsigned int writeVec(const Vec2f& vec, unsigned int offset);
            void freeBlock();
            int compare(unsigned int anAddress, unsigned int aCapacity);
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
            unsigned int findFreeBlockInRange(unsigned int address, unsigned int capacity, unsigned int start, unsigned int length);
            unsigned int findFreeBlock(unsigned int address, unsigned int capacity);
            void insertFreeBlock(VboBlock& block);
            void removeFreeBlock(VboBlock& block);
            void resizeVbo(unsigned int newCapacity);
            void resizeBlock(VboBlock& block, unsigned int newCapacity);
            VboBlock* packBlock(VboBlock& block);
            void checkBlockChain();
            void checkFreeBlocks();
            friend class VboBlock;
        public:
            Vbo(GLenum type, unsigned int capacity);
            ~Vbo();
            void activate();
            void deactivate();
            void map();
            void unmap();
            VboBlock& allocBlock(unsigned int capacity);
            VboBlock& freeBlock(VboBlock& block);
            void freeAllBlocks();
            void pack();
            bool ownsBlock(VboBlock& block);
        };
    }
}

#endif
