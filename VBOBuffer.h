//
//  VBOManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

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

- (void)writeBuffer:(const void*)theBuffer address:(int)theAddress count:(int)theCount;
- (void)writeFloat:(float)f address:(int)theAddress;
- (void)writeVector3f:(TVector3f *)theVector address:(int)theAddress;
- (void)writeVector2f:(TVector2f *)theVector address:(int)theAddress;

- (VBOMemBlock *)allocMemBlock:(int)capacity;
- (VBOMemBlock *)freeMemBlock:(VBOMemBlock *)memBlock;
- (void)freeAllBlocks;
- (void)pack;

@end
