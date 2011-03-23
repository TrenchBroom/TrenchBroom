//
//  VBOManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const BufferNotMappedException;

@class VBOMemBlock;
@class Vector3f;
@class Vector2f;

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

- (void)mapBuffer;
- (void)unmapBuffer;

- (void)writeFloat:(float)f address:(int)theAddress;
- (void)writeVector3f:(Vector3f *)theVector address:(int)theAddress;
- (void)writeVector2f:(Vector2f *)theVector address:(int)theAddress;

- (VBOMemBlock *)allocMemBlock:(int)capacity;
- (void)freeMemBlock:(VBOMemBlock *)memBlock;
- (void)freeAllBlocks;
- (void)pack;

@end
