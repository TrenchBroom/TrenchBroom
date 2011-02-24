//
//  VBOMemBlock.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    BS_FREE, // memory block is free
    BS_USED_INVALID, // memory block is in use, but invalid
    BS_USED_VALID // memory block is in use and valid
} EVBOMemBlockState;

@class VBOBuffer;
@class Vector3f;
@class Vector2f;

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
- (int)capacity;
- (EVBOMemBlockState)state;
                                                                       
- (void)setCapacity:(int)aSize;
- (void)setState:(EVBOMemBlockState)theState;

- (void)activate;
- (void)deactivate;

- (int)writeFloat:(float)theFloat offset:(int)theOffset;
- (int)writeVector3f:(Vector3f *)theVector offset:(int)theOffset;
- (int)writeVector2f:(Vector2f *)theVector offset:(int)theOffset;

- (VBOMemBlock *)previous;
- (VBOMemBlock *)next;

- (void)setPrevious:(VBOMemBlock *)memBlock;
- (void)setNext:(VBOMemBlock *)memBlock;

- (void)free;
@end
