//
//  VBOMemBlock.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface VBOMemBlock : NSObject {
    int index;
    int size;
}

- (id)initWithIndex:(int)anIndex size:(int)aSize;

- (int)index;
- (int)size;

- (BOOL)mergeWith:(VBOMemBlock *)aMemBlock;
@end
