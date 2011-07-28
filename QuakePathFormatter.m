//
//  QuakePathFormatter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "QuakePathFormatter.h"

@implementation QuakePathFormatter

- (NSString *)stringForObjectValue:(id)obj {
    if ([obj isKindOfClass:[NSString class]])
        return [NSString stringWithString:obj];
    return nil;
}

- (BOOL)getObjectValue:(id *)obj forString:(NSString *)string errorDescription:(NSString **)error {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL directory;
    BOOL exists = [fileManager fileExistsAtPath:string isDirectory:&directory];
    if (!exists) {
        if (error)
            *error = [NSString stringWithFormat:@"%@ does not exist", string];
        return NO;
    }
    
    if (!directory) {
        if (error)
            *error = [NSString stringWithFormat:@"%@ is not a directory", string];
        return NO;
    }
    
    *obj = [NSString stringWithString:string];
    return YES;
}

@end
