//
//  VBOManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOMemBlock;

@interface VBOManager : NSObject {
    @private
    int totalCapacity;
    int freeCapacity;
    NSMutableArray* freeBlocksByCapacity;
    VBOMemBlock *firstBlock;
}

- (id)initWithTotalCapacity:(int)capacity;

- (int)totalCapacity;
- (int)freeCapacity;

- (VBOMemBlock *)allocMemBlock:(int)capacity;
- (void)freeMemBlock:(VBOMemBlock *)memBlock;

@end
