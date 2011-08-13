//
//  VBOManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

int writeBuffer(const uint8_t* buffer, uint8_t* vbo, int address, int count);
int writeByte(unsigned char b, uint8_t* vbo, int address);
int writeFloat(float f, uint8_t* vbo, int address);
int writeColor4fAsBytes(const TVector4f* color, uint8_t* vbo, int address);
int writeVector4f(const TVector4f* vector, uint8_t* vbo, int address);
int writeVector3f(const TVector3f* vector, uint8_t* vbo, int address);
int writeVector2f(const TVector2f* vector, uint8_t* vbo, int address);

extern NSString* const BufferNotMappedException;

@class VBOMemBlock;

@interface VBOBuffer : NSObject {
    @private
    int totalCapacity;
    int freeCapacity;
    NSMutableArray* freeBlocksByCapacity;
    VBOMemBlock* firstBlock;
    VBOMemBlock* lastBlock;
    uint8_t* buffer;
    GLuint vboId;
    BOOL active;
}

- (id)initWithTotalCapacity:(int)capacity;

- (int)totalCapacity;
- (int)freeCapacity;

- (void)activate;
- (void)deactivate;
- (BOOL)active;

- (void)mapBuffer;
- (void)unmapBuffer;
- (BOOL)mapped;
- (uint8_t *)buffer;

- (VBOMemBlock *)allocMemBlock:(int)capacity;
- (VBOMemBlock *)freeMemBlock:(VBOMemBlock *)memBlock;
- (void)freeAllBlocks;
- (void)pack;

@end
