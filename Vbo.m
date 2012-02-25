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

#import "Vbo.h"

int writeBuffer(const uint8_t* buffer, uint8_t* vbo, int address, int count) {
    memcpy(vbo + address, buffer, count);
    return address + count;
}

int writeByte(unsigned char b, uint8_t* vbo, int address){
    vbo[address] = b;
    return address + 1;
}

int writeFloat(float f, uint8_t* vbo, int address) {
    for (int i = 0; i < 4; i++)
        vbo[address + i] = ((char *)&f)[i];
    return address + sizeof(float);
}

int writeColor4fAsBytes(const TVector4f* color, uint8_t* vbo, int address) {
    int a = address;
    a = writeByte(color->x * 0xFF, vbo, a);
    a = writeByte(color->y * 0xFF, vbo, a);
    a = writeByte(color->z * 0xFF, vbo, a);
    a = writeByte(color->w * 0xFF, vbo, a);
    return a;
}

int writeVector4f(const TVector4f* vector, uint8_t* vbo, int address) {
    int a = address;
    a = writeFloat(vector->x, vbo, a);
    a = writeFloat(vector->y, vbo, a);
    a = writeFloat(vector->z, vbo, a);
    a = writeFloat(vector->w, vbo, a);
    return a;
}

int writeVector3f(const TVector3f* vector, uint8_t* vbo, int address) {
    int a = address;
    a = writeFloat(vector->x, vbo, a);
    a = writeFloat(vector->y, vbo, a);
    a = writeFloat(vector->z, vbo, a);
    return a;
}

int writeVector2f(const TVector2f* vector, uint8_t* vbo, int address) {
    int a = address;
    a = writeFloat(vector->x, vbo, a);
    a = writeFloat(vector->y, vbo, a);
    return a;
}

void checkVbo(Vbo* vbo) {
    VboBlock* block = vbo->firstBlock;
    assert(block->previous == NULL);
    
    while (block->next != NULL) {
        assert(block->vbo == vbo);
        VboBlock* previous = block;
        block = block->next;
        
        assert(block->previous == previous);
    }
    
    assert(block == vbo->lastBlock);
    
    for (int i = 0; i < vbo->freeBlocksCount; i++) {
        block = vbo->freeBlocks[i];
        assert(block->free);
        assert(block->previous == NULL || !block->previous->free);
        assert(block->next == NULL || !block->next->free);

        if (i < vbo->freeBlocksCount - 1)
            assert(block->capacity <= vbo->freeBlocks[i + 1]->capacity);
    }
}
 
void initVbo(Vbo* vbo, GLenum type, int capacity) {
    vbo->totalCapacity = capacity;
    vbo->freeCapacity = capacity;
    vbo->type = type;
    vbo->firstBlock = malloc(sizeof(VboBlock));
    vbo->firstBlock->vbo = vbo;
    vbo->firstBlock->address = 0;
    vbo->firstBlock->capacity = capacity;
    vbo->firstBlock->previous = NULL;
    vbo->firstBlock->next = NULL;
    vbo->firstBlock->free = YES;
    vbo->lastBlock = vbo->firstBlock;
    
    vbo->freeBlocksCapacity = 0xFF;
    vbo->freeBlocks = malloc(vbo->freeBlocksCapacity * sizeof(VboBlock*));
    vbo->freeBlocks[0] = vbo->firstBlock;
    vbo->freeBlocksCount = 1;
    
    // checkVbo(vbo);
}

void freeVbo(Vbo* vbo) {
    // checkVbo(vbo);

    if (vbo->mapped)
        unmapVbo(vbo);
    if (vbo->active)
        deactivateVbo(vbo);
    if (vbo->vboId != 0)
        glDeleteBuffers(1, &vbo->vboId);
    free(vbo->freeBlocks);
    
    VboBlock* block = vbo->firstBlock;
    while (block != NULL) {
        VboBlock* next = block->next;
        free(block);
        block = next;
    }
}

void activateVbo(Vbo* vbo) {
    assert(vbo != NULL);
    assert(!vbo->active);
    
    // checkVbo(vbo);

    if (vbo->vboId == 0) {
        glGenBuffers(1, &vbo->vboId);
        glBindBuffer(vbo->type, vbo->vboId);
        glBufferData(vbo->type, vbo->totalCapacity, NULL, GL_DYNAMIC_DRAW);
    } else {
        glBindBuffer(vbo->type, vbo->vboId);
    }
    
    assert(glGetError() == GL_NO_ERROR);
    vbo->active = YES;
    
    // checkVbo(vbo);
}

void deactivateVbo(Vbo* vbo) {
    assert(vbo != NULL);
    assert(vbo->active);
    
    // checkVbo(vbo);
    glBindBuffer(vbo->type, 0);
    vbo->active = NO;
    // checkVbo(vbo);
}

void mapVbo(Vbo* vbo) {
    assert(vbo != NULL);
    assert(vbo->active);
    assert(!vbo->mapped);
    
    // checkVbo(vbo);
    vbo->buffer = glMapBuffer(vbo->type, GL_WRITE_ONLY);
    assert(glGetError() == GL_NO_ERROR);
    assert(vbo->buffer != NULL);
    vbo->mapped = YES;
    // checkVbo(vbo);
}

void unmapVbo(Vbo* vbo) {
    assert(vbo != NULL);
    assert(vbo->active);
    assert(vbo->mapped);
    
    // checkVbo(vbo);
    glUnmapBuffer(vbo->type);
    assert(glGetError() == GL_NO_ERROR);
    vbo->buffer = NULL;
    vbo->mapped = NO;
    // checkVbo(vbo);
}

int findFreeVboBlockInRange(Vbo* vbo, int capacity, NSRange range) {
    if (range.length== 1) {
        VboBlock* block = vbo->freeBlocks[range.location];
        if (block->capacity >= capacity)
            return range.location;
        return vbo->freeBlocksCount;
    }
    
    int s = range.length / 2;
    int l = findFreeVboBlockInRange(vbo, capacity, NSMakeRange(range.location, s));
    if (l < vbo->freeBlocksCount)
        return l;
    return findFreeVboBlockInRange(vbo, capacity, NSMakeRange(range.location + s, range.length - s));
}

int findFreeVboBlock(Vbo* vbo, int capacity) {
    if (vbo->freeBlocksCount == 0)
        return 0;
    return findFreeVboBlockInRange(vbo, capacity, NSMakeRange(0, vbo->freeBlocksCount));
}

void insertFreeVboBlock(VboBlock* block) {
    assert(block->free);
    
    Vbo* vbo = block->vbo;
    int index = findFreeVboBlock(vbo, block->capacity);
    assert(index >= 0 && index <= vbo->freeBlocksCount);

    if (vbo->freeBlocksCount == vbo->freeBlocksCapacity) {
        vbo->freeBlocksCapacity *= 2;
        vbo->freeBlocks = realloc(vbo->freeBlocks, vbo->freeBlocksCapacity * sizeof(VboBlock*));
    }

    for (int i = vbo->freeBlocksCount; i > index; i--)
        vbo->freeBlocks[i] = vbo->freeBlocks[i - 1];

    vbo->freeBlocks[index] = block;
    vbo->freeBlocksCount++;
}

void removeFreeVboBlock(VboBlock* block) {
    assert(block->free);

    Vbo* vbo = block->vbo;
    int candidateIndex = findFreeVboBlock(vbo, block->capacity);
    
    VboBlock* candidate = vbo->freeBlocks[candidateIndex];
    int index = candidateIndex;
    
    while (index > 0 && candidate != block && candidate->capacity == block->capacity)
        candidate = vbo->freeBlocks[--index];
    
    if (candidate != block) {
        index = candidateIndex + 1;
        candidate = vbo->freeBlocks[index];
        while (index < vbo->freeBlocksCount - 1 && candidate != block && candidate->capacity == block->capacity)
            candidate = vbo->freeBlocks[++index];
    }

    assert(candidate == block);
    for (int i = index; i < vbo->freeBlocksCount - 1; i++)
        vbo->freeBlocks[i] = vbo->freeBlocks[i + 1];
    vbo->freeBlocks[--vbo->freeBlocksCount] = NULL;
}

void resizeVboBlock(VboBlock* block, int capacity) {
    if (capacity == block->capacity)
        return;
    
    if (block->free) {
        removeFreeVboBlock(block);
        block->capacity = capacity;
        insertFreeVboBlock(block);
    }
}

void resizeVbo(Vbo* vbo, int capacity) {
    BOOL wasActive = vbo->active;
    BOOL wasMapped = vbo->mapped;
    
    uint8_t* temp = NULL;
    if (vbo->vboId != 0 && vbo->freeCapacity < vbo->totalCapacity) {
        if (!wasActive)
            activateVbo(vbo);
        if (!wasMapped)
            mapVbo(vbo);
        
        temp = malloc(vbo->totalCapacity);
        memcpy(temp, vbo->buffer, vbo->totalCapacity);
    }
    
    int addedCapacity = capacity - vbo->totalCapacity;
    vbo->freeCapacity = capacity - (vbo->totalCapacity - vbo->freeCapacity);
    vbo->totalCapacity = capacity;
    
    if (vbo->lastBlock->free) {
        resizeVboBlock(vbo->lastBlock, vbo->lastBlock->capacity + addedCapacity);
    } else {
        VboBlock* block = malloc(sizeof(VboBlock));
        block->vbo = vbo;
        block->address = vbo->lastBlock->address + vbo->lastBlock->capacity;
        block->capacity = addedCapacity;
        block->free = YES;

        block->previous = vbo->lastBlock;
        block->next = NULL;
        vbo->lastBlock->next = block;
        vbo->lastBlock = block;

        insertFreeVboBlock(block);
    }
    
    if (vbo->vboId != 0) {
        if (vbo->mapped)
            unmapVbo(vbo);
        if (vbo->active)
            deactivateVbo(vbo);
        glDeleteBuffers(1, &vbo->vboId);
        vbo->vboId = 0;
    }

    if (temp != NULL) {
        if (!vbo->active)
            activateVbo(vbo);
        if (!vbo->mapped)
            mapVbo(vbo);
        
        memcpy(vbo->buffer, temp, vbo->totalCapacity - addedCapacity);
        free(temp);
        temp = NULL;
        
        if (!wasMapped)
            unmapVbo(vbo);
        if (!wasActive)
            deactivateVbo(vbo);
    } else {
        if (wasActive && !vbo->active)
            activateVbo(vbo);
        if (wasMapped && !vbo->mapped)
            mapVbo(vbo);
    }
}

VboBlock* allocVboBlock(Vbo* vbo, int capacity) {
    assert(capacity > 0);
    
    // checkVbo(vbo);
    if (capacity > vbo->freeCapacity) {
        resizeVbo(vbo, 2 * vbo->totalCapacity);
        return allocVboBlock(vbo, capacity);
    }
    
    int index = findFreeVboBlock(vbo, capacity);
    if (index >= vbo->freeBlocksCount) {
        resizeVbo(vbo, 2 * vbo->totalCapacity);
        return allocVboBlock(vbo, capacity);
    }

    VboBlock* block = vbo->freeBlocks[index];
    for (int i = index; i < vbo->freeBlocksCount - 1; i++)
        vbo->freeBlocks[i] = vbo->freeBlocks[i + 1];
    vbo->freeBlocks[--vbo->freeBlocksCount] = NULL;
    
    // split block
    if (capacity < block->capacity) {
        VboBlock* remainder = malloc(sizeof(VboBlock));
        remainder->vbo = vbo;
        remainder->address = block->address + capacity;
        remainder->capacity = block->capacity - capacity;
        remainder->free = YES;
        block->capacity = capacity;
        
        VboBlock* next = block->next;
        
        remainder->previous = block;
        remainder->next = next;
        if (next != NULL)
            next->previous = remainder;
        block->next = remainder;
        
        insertFreeVboBlock(remainder);
        if (vbo->lastBlock == block)
            vbo->lastBlock = remainder;
    }
    
    block->free = NO;
    vbo->freeCapacity -= block->capacity;

    // checkVbo(vbo);
    
    return block;
}

VboBlock* freeVboBlock(VboBlock* block) {
    Vbo* vbo = block->vbo;
    // checkVbo(vbo);

    VboBlock* previous = block->previous;
    VboBlock* next = block->next;
    
    assert(block != previous);
    assert(block != next);
    
    vbo->freeCapacity += block->capacity;
    block->free = YES;
    
    if (previous != NULL && previous->free && next != NULL && next->free) {
        resizeVboBlock(previous, previous->capacity + block->capacity + next->capacity);

        if (vbo->lastBlock == next)
            vbo->lastBlock = previous;
        
        removeFreeVboBlock(next);
        
        VboBlock* nextNext = next->next;
        previous->next = nextNext;
        if (nextNext != NULL)
            nextNext->previous = previous;

        block->previous = NULL;
        block->next = NULL;
        next->previous = NULL;
        next->next = NULL;
        
        free(block);
        free(next);
        
        // checkVbo(vbo);
        return previous;
    }
    
    if (previous != NULL && previous->free) {
        resizeVboBlock(previous, previous->capacity + block->capacity);

        if (vbo->lastBlock == block)
            vbo->lastBlock = previous;
        
        previous->next = next;
        if (next != NULL)
            next->previous = previous;
        
        block->previous = NULL;
        block->next = NULL;
        free(block);
        
        // checkVbo(vbo);
        return previous;
    }

    if (next != NULL && next->free) {
        if (vbo->lastBlock == next)
            vbo->lastBlock = block;
        
        block->capacity += next->capacity;
        block->free = YES;
        
        insertFreeVboBlock(block);
        removeFreeVboBlock(next);
        
        VboBlock* nextNext = next->next;
        block->next = nextNext;
        if (nextNext != NULL)
            nextNext->previous = block;
        
        next->previous = NULL;
        next->next = NULL;
        free(next);
        
        // checkVbo(vbo);
        return block;
    }
    
    insertFreeVboBlock(block);
    // checkVbo(vbo);
    return block;
}

void freeAllVboBlocks(Vbo* vbo) {
    // checkVbo(vbo);
    if (vbo->freeCapacity == vbo->totalCapacity)
        return;
    
    VboBlock* block = vbo->firstBlock;
    do {
        if (!block->free)
            block = freeVboBlock(block);
        block = block->next;
    } while (block != NULL);
}

VboBlock* packVboBlock(VboBlock* block) {
    VboBlock* first = block->next;
    if (first == NULL)
        return NULL;

    Vbo* vbo = block->vbo;
    VboBlock* previous = block->previous;
    VboBlock* last = first;
    int size = 0;
    int address = first->address;
    
    do {
        last->address -= block->capacity;
        size += last->capacity;
        previous = last;
        last = last->next;
    } while (last != NULL && !last->free);
    
    if (size <= block->capacity) {
        memcpy(vbo->buffer + block->address, vbo->buffer + address, size);
    } else {
        uint8_t* temp = malloc(size);
        memcpy(temp, vbo->buffer + address, size);
        memcpy(vbo->buffer + block->address, temp, size);
        free(temp);
    }
    
    if (last != NULL) {
        last->address -= block->capacity;
        resizeVboBlock(last, last->capacity + block->capacity);
    } else {
        VboBlock* newBlock = malloc(sizeof(VboBlock));
        newBlock->vbo = vbo;
        newBlock->address = previous->address + previous->capacity;
        newBlock->capacity = block->capacity;
        newBlock->free = YES;
        
        insertFreeVboBlock(newBlock);
        previous->next = newBlock;
        newBlock->previous = previous;
        newBlock->next = NULL;
        vbo->lastBlock = newBlock;
    }
    
    if (vbo->firstBlock == block)
        vbo->firstBlock = block->next;
    
    removeFreeVboBlock(block);
    if (block->previous != NULL)
        block->previous->next = block->next;
    if (block->next != NULL)
        block->next->previous = block->previous;
    block->previous = NULL;
    block->next = NULL;
    free(block);
    
    return last;
}

void packVbo(Vbo* vbo) {
    assert(vbo->mapped);
    
    if (vbo->totalCapacity == vbo->freeCapacity || (vbo->lastBlock->free && vbo->lastBlock->capacity == vbo->freeCapacity))
        return;
    
    // checkVbo(vbo);
    // find first free block
    VboBlock* freeBlock = vbo->firstBlock;
    while (freeBlock != NULL && !freeBlock->free)
        freeBlock = freeBlock->next;
    
    while (freeBlock != NULL && freeBlock->next != NULL)
        freeBlock = packVboBlock(freeBlock);
    // checkVbo(vbo);
}
