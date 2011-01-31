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
    int index;
    int count;
}

- (id)initWithIndex:(int)theIndex count:(int)theCount;

- (int)index;
- (int)count;

@end
