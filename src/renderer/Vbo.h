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

#ifndef __TrenchBroom__Vbo__
#define __TrenchBroom__Vbo__

#include "SharedPointer.h"
#include <GL/GL.h>

#include <cstring>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class VboBlock;
        
        class CompareVboBlocksByCapacity {
        public:
            bool operator() (const VboBlock* lhs, const VboBlock* rhs) const;
        };
        
        namespace VboState {
            typedef unsigned int Type;
            static const Type Inactive  = 0;
            static const Type Active    = 1;
            static const Type Mapped    = 2;
        }
        
        class Vbo;
        class SetVboState {
        private:
            Vbo& m_vbo;
            VboState::Type m_previousState;
        public:
            SetVboState(Vbo& vbo);
            ~SetVboState();
            
            void active();
            void mapped();
        private:
            void setState(const VboState::Type newState);
        };
        
        class Vbo {
        private:
            typedef std::vector<VboBlock*> VboBlockList;
            static const float GrowthFactor;
            
            size_t m_totalCapacity;
            size_t m_freeCapacity;
            VboBlockList m_freeBlocks;
            VboBlockList m_usedBlocks;
            VboBlock* m_firstBlock;
            VboBlock* m_lastBlock;
            VboState::Type m_state;

            GLenum m_type;
            GLenum m_usage;
            GLuint m_vboId;
            unsigned char* m_buffer;
        public:
            Vbo(const size_t initialCapacity, const GLenum type, const GLenum usage = GL_DYNAMIC_DRAW);
            ~Vbo();
            
            inline const VboState::Type state() const {
                return m_state;
            }
            
            VboBlock* allocateBlock(const size_t capacity);
        private:
            friend class SetVboState;
            friend class VboBlock;
            
            inline bool isActive() const {
                return m_state > VboState::Inactive;
            }
            
            inline bool isMapped() const {
                return m_state == VboState::Mapped;
            }
            
            void activate();
            void deactivate();
            void map();
            void unmap();
            
            void freeBlock(VboBlock* block);
            
            template <typename T>
            inline size_t writeElement(const size_t address, const T& element) {
                assert(isMapped());
                const size_t size = sizeof(T);
                assert(address + size < m_totalCapacity);
                reinterpret_cast<T>(m_buffer + address) = element;
                return size;
            }
            
            template <typename T>
            inline size_t writeElements(const size_t address, const std::vector<T>& elements) {
                assert(isMapped());
                const size_t size = elements.size() * sizeof(T);
                assert(address + size < m_totalCapacity);
                
                typename std::vector<T>::const_iterator it, end;
                for (it = elements.begin(), end = elements.end(); it != end; ++it)
                    reinterpret_cast<T>(m_buffer + address) = *it;
                return size;
            }
            
            template <typename T>
            inline size_t writeBuffer(const size_t address, const std::vector<T>& buffer) {
                assert(isMapped());
                const size_t size = buffer.size() * sizeof(T);
                assert(address + size < m_totalCapacity);
                
                const T* ptr = &(buffer[0]);
                memcpy(m_buffer + address, ptr, size);
                return size;
            }

            void increaseCapacityToAccomodate(const size_t capacity);
            void increaseCapacity(const size_t delta);
            VboBlockList::iterator findFreeBlock(const size_t minCapacity);
            void insertFreeBlock(VboBlock* block);
            void removeFreeBlock(VboBlock* block);
            void removeFreeBlock(const VboBlockList::iterator it);

            bool checkBlockChain() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Vbo__) */
