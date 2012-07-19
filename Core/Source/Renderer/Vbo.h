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

#include <cassert>
#include <cstring>
#include <vector>
#include <exception>

#include "Utilities/VecMath.h"
#include "GL/GLee.h"

//#define _DEBUG_VBO 1

namespace TrenchBroom {
    namespace Renderer {
        class VboBlock;

        class Vbo {
        private:
            unsigned int m_totalCapacity;
            unsigned int m_freeCapacity;
            std::vector<VboBlock*> m_freeBlocks;
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
            VboBlock* allocBlock(unsigned int capacity);
            VboBlock* freeBlock(VboBlock& block);
            void freeAllBlocks();
            void pack();
            bool ownsBlock(VboBlock& block);
        };

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
            
            unsigned int writeBuffer(const unsigned char* buffer, unsigned int offset, unsigned int length) {
                assert(offset >= 0 && offset + length <= capacity);
                memcpy(m_vbo.m_buffer + address + offset, buffer, length);
                return offset + length;
            }
            
            unsigned int writeByte(unsigned char b, unsigned int offset) {
                assert(offset >= 0 && offset < capacity);
                m_vbo.m_buffer[address + offset] = b;
                return offset + 1;
            }
            
            unsigned int writeFloat(float f, unsigned int offset) {
                assert(offset >= 0 && offset + sizeof(float) <= capacity);
                memcpy(m_vbo.m_buffer + address + offset, &f, sizeof(float));
                return offset + sizeof(float);
            }
            
            unsigned int writeUInt32(unsigned int i, unsigned int offset) {
                assert(offset >= 0 && offset + sizeof(unsigned int) <= capacity);
                memcpy(m_vbo.m_buffer + address + offset, &i, sizeof(unsigned int));
                return offset + sizeof(unsigned int);
            }
            
            unsigned int writeColor(const Vec4f& color, unsigned int offset) {
                assert(offset >= 0 && offset + 4 <= capacity);
                offset = writeByte((unsigned char)(color.x * 0xFF), offset);
                offset = writeByte((unsigned char)(color.y * 0xFF), offset);
                offset = writeByte((unsigned char)(color.z * 0xFF), offset);
                offset = writeByte((unsigned char)(color.w * 0xFF), offset);
                return offset;
            }

            template<class T>
            unsigned int writeVec(const T& vec, unsigned int offset) {
                assert(offset >= 0 && offset + sizeof(T) <= capacity);
                memcpy(m_vbo.m_buffer + address + offset, &vec, sizeof(T));
                return offset + sizeof(T);
            }
            
            template<class T>
            unsigned int writeVecs(const std::vector<T>& vecs, unsigned int offset) {
                unsigned int size = vecs.size() * sizeof(T);
                assert(offset >= 0 && offset + size <= capacity);
                memcpy(m_vbo.m_buffer + address + offset, &(vecs[0]), size);
                return offset + size;
            }
            
            void freeBlock();
            int compare(unsigned int anAddress, unsigned int aCapacity);
        };

		class VboException : public std::exception {
		protected:
			Vbo& m_vbo;
			GLenum m_glError;
			std::string m_msg;
		public:
			VboException(Vbo& vbo, const std::string& msg) : m_vbo(vbo), m_glError(glGetError()), m_msg(msg) {}

			virtual const char* what() const throw() {
			    return m_msg.c_str();
			}
 		};
    }
}

#endif
