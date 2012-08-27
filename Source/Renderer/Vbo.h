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
#include <exception>
#include <vector>

#include "Utility/Color.h"
#include "Utility/GLee.h"
#include "Utility/VecMath.h"

//#define _DEBUG_VBO 1

using namespace TrenchBroom::Math;

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

            unsigned int m_address;
            unsigned int m_capacity;
            bool m_free;
            VboBlock* m_previous;
            VboBlock* m_next;
        public:
            inline VboBlock(Vbo& vbo, int address, int capacity) : m_vbo(vbo), m_address(address), m_capacity(capacity), m_free(true), m_previous(NULL), m_next(NULL) {}
            
            inline unsigned int address() const {
                return m_address;
            }
            
            inline unsigned int capacity() const {
                return m_capacity;
            }
            
            inline bool free() const {
                return m_free;
            }
            
            inline unsigned int writeBuffer(const unsigned char* buffer, unsigned int offset, unsigned int length) {
                assert(offset >= 0 && offset + length <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, buffer, length);
                return offset + length;
            }
            
            inline unsigned int writeByte(unsigned char b, unsigned int offset) {
                assert(offset >= 0 && offset < m_capacity);
                m_vbo.m_buffer[m_address + offset] = b;
                return offset + 1;
            }
            
            inline unsigned int writeFloat(float f, unsigned int offset) {
                assert(offset >= 0 && offset + sizeof(float) <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &f, sizeof(float));
                return offset + sizeof(float);
            }
            
            inline unsigned int writeUInt32(unsigned int i, unsigned int offset) {
                assert(offset >= 0 && offset + sizeof(unsigned int) <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &i, sizeof(unsigned int));
                return offset + sizeof(unsigned int);
            }
            
            inline unsigned int writeColor(const Color& color, unsigned int offset) {
                assert(offset >= 0 && offset + 4 <= m_capacity);
                offset = writeByte(static_cast<unsigned char>(color.x * 0xFF), offset);
                offset = writeByte(static_cast<unsigned char>(color.y * 0xFF), offset);
                offset = writeByte(static_cast<unsigned char>(color.z * 0xFF), offset);
                offset = writeByte(static_cast<unsigned char>(color.w * 0xFF), offset);
                return offset;
            }

            template<class T>
            inline unsigned int writeVec(const T& vec, unsigned int offset) {
                assert(offset >= 0 && offset + sizeof(T) <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &vec, sizeof(T));
                return offset + sizeof(T);
            }
            
            template<class T>
            inline unsigned int writeVecs(const std::vector<T>& vecs, unsigned int offset) {
                unsigned int size = static_cast<unsigned int>(vecs.size() * sizeof(T));
                assert(offset >= 0 && offset + size <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &(vecs[0]), size);
                return offset + size;
            }
            
            void freeBlock();
            
            inline int compare(unsigned int address, unsigned int capacity) {
                if (m_capacity < capacity) return -1;
                if (m_capacity > capacity) return 1;
                if (m_address < address) return -1;
                if (m_address > address) return 1;
                return 0;
            }
        };

		class VboException : public std::exception {
		protected:
			Vbo& m_vbo;
			std::string m_msg;
			GLenum m_glError;
		public:
			VboException(Vbo& vbo, const std::string& msg, GLenum glError) throw() : m_vbo(vbo), m_msg(msg), m_glError(glError) {}
            ~VboException() throw() {}
            
			virtual const char* what() const throw() {
			    return m_msg.c_str();
			}
 		};
    }
}

#endif
