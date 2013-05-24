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

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;

        class VboBlock {
        private:
            friend class Vbo;

            bool m_free;
            size_t m_offset;
            size_t m_capacity;
            VboBlock* m_previous;
            VboBlock* m_next;
        public:
            VboBlock(const size_t offset, const size_t capacity, VboBlock* previous, VboBlock* next);
            
            inline size_t capacity() const {
                return m_capacity;
            }
            
            void deallocate();
        private:
            inline VboBlock* previous() const {
                return m_previous;
            }
            
            inline void setPrevious(VboBlock* previous) {
                m_previous = previous;
            }
            
            inline VboBlock* next() const {
                return m_next;
            }
            
            inline void setNext(VboBlock* next) {
                m_next = next;
            }
            
            inline bool isFree() const {
                return m_free;
            }
            
            inline void setFree(const bool free) {
                m_free = free;
            }
            
            inline void setCapacity(const size_t capacity) {
                m_capacity = capacity;
            }
            
            VboBlock* split(const size_t capacity);
            VboBlock* createSuccessor(const size_t capacity);
        };
        
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
            SetVboState(Vbo& vbo, const VboState::Type newState);
            ~SetVboState();
        };
        
        class Vbo {
        private:
            friend class SetVboState;
            
            typedef std::vector<VboBlock*> VboBlockList;
            
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
            
            inline const size_t capacity() const {
                return m_totalCapacity;
            }
            
            inline const size_t freeCapacity() const {
                return m_freeCapacity;
            }
            
            inline const VboState::Type state() const {
                return m_state;
            }
            
            VboBlock& allocateBlock(const size_t capacity);
        private:
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
            
            void increaseCapacity(const size_t delta);
            VboBlockList::iterator findFreeBlock(const size_t minCapacity);
            void insertFreeBlock(VboBlock* block);
            void removeFreeBlock(VboBlock* block);
            void removeFreeBlock(const VboBlockList::iterator it);
        };
    }
}

#endif /* defined(__TrenchBroom__Vbo__) */
