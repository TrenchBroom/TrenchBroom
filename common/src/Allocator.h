/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Allocator_h
#define TrenchBroom_Allocator_h

#include <cassert>
#include <iostream>
#include <limits>
#include <stack>
#include <vector>

// Undefine this to prevent false positives when looking for memory leaks.
#define TB_ENABLE_ALLOCATOR 1

template <class T, size_t PoolSize = 64, size_t BlocksPerChunk = 256>
class Allocator {
private:
    class Chunk {
    private:
        unsigned char m_firstFreeBlock;
        unsigned char m_numFreeBlocks;
        unsigned char m_blocks[BlocksPerChunk * sizeof(T)];
    public:
        Chunk() :
        m_firstFreeBlock(0),
        m_numFreeBlocks(BlocksPerChunk - 1) {
            for (size_t i = 0; i < BlocksPerChunk - 1; i++)
                m_blocks[i * sizeof(T)] = static_cast<unsigned char>(i + 1);
        }
        
        bool contains(const T* t) const {
            const unsigned char* block = reinterpret_cast<const unsigned char*>(t);
            if (block < m_blocks)
                return false;
            size_t offset = static_cast<size_t>(block - m_blocks);
            return offset < (BlocksPerChunk - 1) * sizeof(T);
        }
        
        T* allocate() {
            if (m_numFreeBlocks == 0)
                return NULL;
            
            unsigned char* block = m_blocks + m_firstFreeBlock * sizeof(T);
            m_firstFreeBlock = *block;
            m_numFreeBlocks--;
            return reinterpret_cast<T*>(block);
        }
        
        void deallocate(T* t) {
            assert(m_numFreeBlocks < BlocksPerChunk - 1);
            assert(contains(t));
            
            unsigned char* block = reinterpret_cast<unsigned char*>(t);
            assert(block >= m_blocks);
            size_t offset = static_cast<size_t>(block - m_blocks);
            assert(offset % sizeof(T) == 0);
            
            size_t index = offset / sizeof(T);
            assert(index < BlocksPerChunk);
            
            *block = m_firstFreeBlock;
            m_firstFreeBlock = static_cast<unsigned char>(index);
            m_numFreeBlocks++;
        }
        
        bool empty() const {
            return m_numFreeBlocks == BlocksPerChunk - 1;
        }
        
        bool full() const {
            return m_numFreeBlocks == 0;
        }
    };
    
    typedef std::vector<Chunk*> ChunkList;
    typedef std::stack<T*> Pool;
    
    static Pool& pool() {
        static Pool p;
        return p;
    }
    
    static ChunkList& fullChunks() {
        static ChunkList chunks;
        return chunks;
    }
    
    static ChunkList& mixedChunks() {
        static ChunkList chunks;
        return chunks;
    }
    
    static ChunkList emptyChunks() {
        static ChunkList chunks;
        return chunks;
    }
public:
#ifdef TB_ENABLE_ALLOCATOR
    void* operator new(size_t size) {
        assert(size == sizeof(T));
        
        if (!pool().empty()) {
            T* t = pool().top();
            pool().pop();
            return t;
        }
        
        Chunk* chunk = NULL;
        if (mixedChunks().empty()) {
            if (!emptyChunks().empty()) {
                chunk = emptyChunks().back();
                emptyChunks().pop_back();
            } else {
                chunk = new Chunk();
            }
        } else {
            chunk = mixedChunks().back();
            mixedChunks().pop_back();
        }
        
        assert(!chunk->full());
        T* block = chunk->allocate();
        
        if (chunk->full())
            fullChunks().push_back(chunk);
            else
                mixedChunks().push_back(chunk);
                return block;
    }
    
    void operator delete(void* block) {
        T* t = reinterpret_cast<T*>(block);
        
        if (PoolSize > 0 && pool().size() < PoolSize) {
            pool().push(t);
            return;
        }
        
        typename ChunkList::reverse_iterator fullIt, fullEnd, mixedIt, mixedEnd;
        fullIt = fullChunks().rbegin();
        fullEnd = fullChunks().rend();
        mixedIt = mixedChunks().rbegin();
        mixedEnd = mixedChunks().rend();
        
        Chunk* chunk = NULL;
        while (fullIt < fullEnd || mixedIt < mixedEnd) {
            if (fullIt < fullEnd) {
                Chunk* fullChunk = *fullIt;
                if (fullChunk->contains(t)) {
                    chunk = fullChunk;
                    break;
                }
                ++fullIt;
            }
            if (mixedIt < mixedEnd) {
                Chunk* mixedChunk = *mixedIt;
                if (mixedChunk->contains(t)) {
                    chunk = mixedChunk;
                    break;
                }
                ++mixedIt;
            }
        }
        
        assert(chunk != NULL);
        
        if (chunk->full()) {
            fullChunks().erase((fullIt + 1).base());
            mixedChunks().push_back(chunk);
        }
        
        chunk->deallocate(t);
        
        if (chunk->empty()) {
            mixedChunks().erase((mixedIt + 1).base());
            if (emptyChunks().size() < 2)
                emptyChunks().push_back(chunk);
                else
                    delete chunk;
        }
    }
#endif
};

#endif
