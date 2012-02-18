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

#import <Cocoa/Cocoa.h>
#import "Math.h"

extern NSString* const BufferNotMappedException;

typedef struct TVboBlockTag {
    struct TVboTag* vbo;
    unsigned int address;
    unsigned int capacity;
    BOOL free;
    struct TVboBlockTag* next;
    struct TVboBlockTag* previous;
} VboBlock;

typedef struct TVboTag {
    unsigned int totalCapacity;
    unsigned int freeCapacity;
    VboBlock** freeBlocks;
    int freeBlocksCount;
    int freeBlocksCapacity;
    VboBlock* firstBlock;
    VboBlock* lastBlock;
    uint8_t* buffer;
    GLuint vboId;
    GLenum type;
    BOOL active;
    BOOL mapped;
} Vbo;

int writeBuffer(const uint8_t* buffer, uint8_t* vbo, int address, int count);
int writeByte(unsigned char b, uint8_t* vbo, int address);
int writeFloat(float f, uint8_t* vbo, int address);
int writeColor4fAsBytes(const TVector4f* color, uint8_t* vbo, int address);
int writeVector4f(const TVector4f* vector, uint8_t* vbo, int address);
int writeVector3f(const TVector3f* vector, uint8_t* vbo, int address);
int writeVector2f(const TVector2f* vector, uint8_t* vbo, int address);

void initVbo(Vbo* vbo, GLenum type, int capacity);
void freeVbo(Vbo* vbo);
void activateVbo(Vbo* vbo);
void deactivateVbo(Vbo* vbo);
void mapVbo(Vbo* vbo);
void unmapVbo(Vbo* vbo);
VboBlock* allocVboBlock(Vbo* vbo, int capacity);
VboBlock* freeVboBlock(VboBlock* block);
void freeAllVboBlocks(Vbo* vbo);
void packVbo(Vbo* vbo);
