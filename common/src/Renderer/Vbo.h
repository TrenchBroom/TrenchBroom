/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_Vbo
#define TrenchBroom_Vbo

#include "SharedPointer.h"
#include "Renderer/GL.h"

#include <cassert>
#include <cstring>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class VboBlock;
        
        class CompareVboBlocksByCapacity {
        public:
            bool operator() (const VboBlock* lhs, const VboBlock* rhs) const;
        };
        
        class Vbo;
        class ActivateVbo {
        private:
            Vbo& m_vbo;
            bool m_wasActive;
        public:
            ActivateVbo(Vbo& vbo);
            ~ActivateVbo();
        };
        
        class Vbo {
        public:
            typedef std::shared_ptr<Vbo> Ptr;
        private:
            typedef enum {
                State_Inactive = 0,
                State_Active = 1,
                State_PartiallyMapped = 2,
                State_FullyMapped = 3
            } State;
        private:
            typedef std::vector<VboBlock*> VboBlockArray;
            static const float GrowthFactor;
            
            size_t m_totalCapacity;
            size_t m_freeCapacity;
            VboBlockArray m_freeBlocks;
            VboBlockArray m_usedBlocks;
            VboBlock* m_firstBlock;
            VboBlock* m_lastBlock;
            State m_state;

            GLenum m_type;
            GLenum m_usage;
            GLuint m_vboId;
        public:
            Vbo(const size_t initialCapacity, const GLenum type = GL_ARRAY_BUFFER, const GLenum usage = GL_DYNAMIC_DRAW);
            ~Vbo();
            
            VboBlock* allocateBlock(const size_t capacity);

            bool active() const;
            void activate();
            void deactivate();
        private:
            friend class ActivateVbo;
            friend class VboBlock;
            
            GLenum type() const;
            
            void free();
            void freeBlock(VboBlock* block);

            void increaseCapacityToAccomodate(const size_t capacity);
            void increaseCapacity(size_t delta);
            VboBlockArray::iterator findFreeBlock(size_t minCapacity);
            void insertFreeBlock(VboBlock* block);
            void removeFreeBlock(VboBlock* block);
            void removeFreeBlock(VboBlockArray::iterator it);

            bool partiallyMapped() const;
            void mapPartially();
            void unmapPartially();
            
            bool fullyMapped() const;
            unsigned char* map();
            void unmap();

            bool checkBlockChain() const;
        };
    }
}

#endif /* defined(TrenchBroom_Vbo) */
