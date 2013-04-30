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

#include <GL/glew.h>
#include "Utility/Color.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <cstring>
#include <exception>
#include <sstream>
#include <vector>

//#define _DEBUG_VBO 1

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class VboBlock;

        class Vbo {
        public:
            typedef enum {
                VboInactive = 0,
                VboActive   = 1,
                VboMapped   = 2
            } VboState;
        private:
            struct MemBlock {
                typedef std::vector<MemBlock> List;
                
                size_t start;
                size_t length;
                
                MemBlock(size_t i_start, size_t i_length) :
                start(i_start),
                length(i_length) {}
                
                MemBlock() {}
            };

            GLenum m_type;
            size_t m_totalCapacity;
            size_t m_freeCapacity;
            std::vector<VboBlock*> m_freeBlocks;
            VboBlock* m_first;
            VboBlock* m_last;
            unsigned char* m_buffer;
            GLuint m_vboId;
            VboState m_state;
            size_t findFreeBlockInRange(size_t address, size_t capacity, size_t start, size_t length);
            size_t findFreeBlock(size_t address, size_t capacity);
            void insertFreeBlock(VboBlock& block);
            void removeFreeBlock(VboBlock& block);
            void resizeVbo(size_t newCapacity);
            void resizeBlock(VboBlock& block, size_t newCapacity);
            VboBlock* packBlock(VboBlock& block);
            void checkBlockChain();
            void checkFreeBlocks();
            friend class VboBlock;
        public:
            Vbo(GLenum type, size_t capacity);
            ~Vbo();
            void activate();
            void deactivate();
            void map();
            void unmap();
            
            inline VboState state() const {
                return m_state;
            }
            
            void ensureFreeCapacity(size_t capacity);
            VboBlock* allocBlock(size_t capacity);
            VboBlock* freeBlock(VboBlock& block);
            void freeAllBlocks();
            void pack();
            bool ownsBlock(VboBlock& block);
        };

        class SetVboState {
        private:
            Vbo& m_vbo;
            Vbo::VboState m_oldState;
        public:
            SetVboState(Vbo& vbo, Vbo::VboState newState) :
            m_vbo(vbo),
            m_oldState(m_vbo.state()) {
                if (m_oldState != newState) {
                    switch (m_oldState) {
                        case Vbo::VboInactive:
                            m_vbo.activate();
                            if (newState == Vbo::VboMapped)
                                m_vbo.map();
                            break;
                        case Vbo::VboActive:
                            if (newState == Vbo::VboInactive)
                                m_vbo.deactivate();
                            else
                                m_vbo.map();
                            break;
                        case Vbo::VboMapped:
                            m_vbo.unmap();
                            if (newState == Vbo::VboInactive)
                                m_vbo.deactivate();
                            break;
                    }
                }
            }
            
            ~SetVboState() {
                if (m_vbo.state() != m_oldState) {
                    switch (m_vbo.state()) {
                        case Vbo::VboInactive:
                            m_vbo.activate();
                            if (m_oldState == Vbo::VboMapped)
                                m_vbo.map();
                            break;
                        case Vbo::VboActive:
                            if (m_oldState == Vbo::VboInactive)
                                m_vbo.deactivate();
                            else
                                m_vbo.map();
                            break;
                        case Vbo::VboMapped:
                            m_vbo.unmap();
                            if (m_oldState == Vbo::VboInactive)
                                m_vbo.deactivate();
                            break;
                    }
                }
            }
        };
        
        class VboBlock {
        private:
            Vbo& m_vbo;
            void insertBetween(VboBlock* previousBlock, VboBlock* nextBlock);
            friend class Vbo;

            size_t m_address;
            size_t m_capacity;
            bool m_free;
            VboBlock* m_previous;
            VboBlock* m_next;
        public:
            inline VboBlock(Vbo& vbo, size_t address, size_t capacity) :
            m_vbo(vbo),
            m_address(address),
            m_capacity(capacity),
            m_free(true),
            m_previous(NULL),
            m_next(NULL) {}

            inline size_t address() const {
                return m_address;
            }

            inline size_t capacity() const {
                return m_capacity;
            }

            inline bool free() const {
                return m_free;
            }

            inline size_t writeBuffer(const unsigned char* buffer, size_t offset, size_t length) {
                assert(offset >= 0 && offset + length <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, buffer, length);
                return offset + length;
            }

            inline size_t writeByte(unsigned char b, size_t offset) {
                assert(offset >= 0 && offset < m_capacity);
                m_vbo.m_buffer[m_address + offset] = b;
                return offset + 1;
            }

            inline size_t writeFloat(float f, size_t offset) {
                assert(offset >= 0 && offset + sizeof(float) <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &f, sizeof(float));
                return offset + sizeof(float);
            }

            inline size_t writeUInt32(size_t i, size_t offset) {
                assert(offset >= 0 && offset + sizeof(size_t) <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &i, sizeof(size_t));
                return offset + sizeof(size_t);
            }

            inline size_t writeColor(const Color& color, size_t offset) {
                assert(offset >= 0 && offset + 4 <= m_capacity);
                m_vbo.m_buffer[m_address + offset + 0] = static_cast<unsigned char>(color.r() * 0xFF);
                m_vbo.m_buffer[m_address + offset + 1] = static_cast<unsigned char>(color.g() * 0xFF);
                m_vbo.m_buffer[m_address + offset + 2] = static_cast<unsigned char>(color.b() * 0xFF);
                m_vbo.m_buffer[m_address + offset + 3] = static_cast<unsigned char>(color.a() * 0xFF);
                return offset + 4;
            }

            template<class T>
            inline size_t writeVec(const T& vec, size_t offset) {
                assert(offset >= 0 && offset + sizeof(T) <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &vec, sizeof(T));
                return offset + sizeof(T);
            }

            template<class T>
            inline size_t writeVecs(const std::vector<T>& vecs, size_t offset) {
                size_t size = static_cast<size_t>(vecs.size() * sizeof(T));
                assert(offset >= 0 && offset + size <= m_capacity);
                memcpy(m_vbo.m_buffer + m_address + offset, &(vecs[0]), size);
                return offset + size;
            }

            void freeBlock();

            inline int compare(size_t address, size_t capacity) {
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
                std::stringstream msg;
                msg << m_msg << " (OpenGL error " << m_glError << ")";
			    return msg.str().c_str();
			}
 		};
    }
}

#endif
