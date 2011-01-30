//
//  VBOArrayEntry.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface VBOArrayEntry : NSObject {
    @private
    id object;
    int index;
    int count;
}

- (id)initWithObject:(id)theObject index:(int)theIndex count:(int)theCount;

- (id)object;
- (int)index;
- (int)count;

@end
