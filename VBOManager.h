//
//  VBOManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface VBOManager : NSObject {
    int size;
    NSMutableArray* freeMemBySize;
    NSMutableArray* freeMemByIndex;
}

- (id)initWithSize:(int)aSize;

- (void)insertMemBlock:(VBOMemBlock *)aMemBlock;
- (void)removeMemBlock:(VBOMemBlock *)aMemBlock;

- (VBOMemBlock*)getMemBlockSize:(int)aSize;

@end
