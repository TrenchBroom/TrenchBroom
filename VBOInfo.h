//
//  VBOInfo.h
//  TrenchBroom
//
//  Created by Kristian Duske on 31.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface VBOInfo : NSObject {
    @private
    int* indices;
    int count;
}

- (void)addIndex:(int)theIndex;

@end
