//
//  VBOMemBlock.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface VBOMemBlock : NSObject {
    int capacity;
    BOOL free;
    VBOMemBlock* next;
    VBOMemBlock* previous;
}

- (id)initWithBlockCapacity:(int)aSize;

- (BOOL)free;
- (int)capacity;

- (void)setFree:(BOOL)value;
- (void)setCapacity:(int)aSize;

- (VBOMemBlock *)previous;
- (VBOMemBlock *)next;

- (void)setPrevious:(VBOMemBlock *)memBlock;
- (void)setNext:(VBOMemBlock *)memBlock;
@end
