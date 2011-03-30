//
//  FloatData.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface FloatData : NSObject {
    @private
    NSMutableData* data;
    int count;
}

- (id)initDataWithCapacity:(int)capacity;

- (void)appendFloat:(float)value;
- (const void*)bytes;
- (int)count;

@end
