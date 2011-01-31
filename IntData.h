//
//  IntData.h
//  TrenchBroom
//
//  Created by Kristian Duske on 31.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface IntData : NSObject {
    @private
    NSMutableData* data;
    int count;
}

- (void)appendInt:(int)value;
- (const void*)bytes;
- (int)count;

@end
