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

#ifndef TrenchBroom_Vbo
#define TrenchBroom_Vbo

#include "Macros.h"
#include "Renderer/GL.h"

#include <memory>
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
            explicit ActivateVbo(Vbo& vbo);
            ~ActivateVbo();
        };

        class Vbo {
        public:
            using Ptr = std::shared_ptr<Vbo>;
        private:
            typedef enum {
                State_Inactive = 0,
                State_Active = 1,
                State_PartiallyMapped = 2,
                State_FullyMapped = 3
            } State;
        private:
            using VboBlockList = std::vector<VboBlock*>;
            static const float GrowthFactor;

            size_t m_totalCapacity;
            size_t m_freeCapacity;
            VboBlockList m_freeBlocks;
            VboBlockList m_blocksPendingFree;
            VboBlock* m_firstBlock;
            VboBlock* m_lastBlock;
            State m_state;

            GLenum m_type;
            GLenum m_usage;
            GLuint m_vboId;
        public:
            explicit Vbo(size_t initialCapacity, GLenum type = GL_ARRAY_BUFFER, GLenum usage = GL_DYNAMIC_DRAW);
            ~Vbo();

            deleteCopyAndMove(Vbo)

            VboBlock* allocateBlock(size_t capacity);

            bool active() const;
            void activate();
            void deactivate();
        private:
            friend class ActivateVbo;
            friend class VboBlock;

            GLenum type() const;

            void free();
            void enqueueBlockForFreeing(VboBlock* block);
            void freeBlock(VboBlock* block);
        public:
            void freePendingBlocks();
            
        private:
            void increaseCapacityToAccomodate(size_t capacity);
            void increaseCapacity(size_t delta);
            VboBlockList::iterator findFreeBlock(size_t minCapacity);
            void insertFreeBlock(VboBlock* block);
            void removeFreeBlock(VboBlock* block);
            void removeFreeBlock(VboBlockList::iterator it);

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
