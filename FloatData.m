//
//  FloatData.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FloatData.h"


@implementation FloatData
- (id)init {
    if (self = [super init]) {
        data = [[NSMutableData alloc] init];
        count = 0;
    }
    
    return self;
}

- (id)initDataWithCapacity:(int)capacity {
    if (self = [super init]) {
        data = [[NSMutableData alloc] initWithLength:capacity * sizeof(int)];
        count = 0;
    }
    
    return self;
}

- (void)appendFloat:(float)value {
    [data appendBytes:(char *)&value length:sizeof(float)];
    count++;
}

- (const void*)bytes{
    return [data bytes];
}

- (int)count {
    return count;
}

- (void)dealloc {
    [data release];
    [super dealloc];
}
@end
