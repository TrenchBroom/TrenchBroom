//
//  VBOMemBlock.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    BS_FREE, // memory block is free
    BS_USED_INVALID, // memory block is in use, but invalid
    BS_USED_VALID // memory block is in use and valid
} EVBOMemBlockState;

@class VBOBuffer;

@interface VBOMemBlock : NSObject {
    @private
    int address;
    int capacity;
    EVBOMemBlockState state;
    VBOMemBlock* next;
    VBOMemBlock* previous;
    VBOBuffer* vboBuffer;
}

- (id)initBlockIn:(VBOBuffer *)theVboBuffer at:(int)theAddress capacity:(int)theCapacity;

- (int)address;
- (void)setAddress:(int)theAddress;
- (int)capacity;
- (EVBOMemBlockState)state;
- (VBOBuffer *)vbo;
                                                                       
- (void)setCapacity:(int)aSize;
- (void)setState:(EVBOMemBlockState)theState;

- (void)activate;
- (void)deactivate;

- (int)writeBuffer:(const void*)theBuffer offset:(int)theOffset count:(int)theCount;
- (int)writeByte:(unsigned char)theByte offset:(int)theOffset;
- (int)writeFloat:(float)theFloat offset:(int)theOffset;
- (int)writeColor4fAsBytes:(const TVector4f *)theVector offset:(int)theOffset;
- (int)writeVector4f:(const TVector4f *)theVector offset:(int)theOffset;
- (int)writeVector3f:(const TVector3f *)theVector offset:(int)theOffset;
- (int)writeVector2f:(const TVector2f *)theVector offset:(int)theOffset;

- (VBOMemBlock *)previous;
- (VBOMemBlock *)next;

- (void)setPrevious:(VBOMemBlock *)memBlock;
- (void)setNext:(VBOMemBlock *)memBlock;

- (void)insertBetweenPrevious:(VBOMemBlock *)previousBlock next:(VBOMemBlock *)nextBlock;
- (void)remove;

- (void)free;
@end
