//
//  ChoiceArgument.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ChoiceArgument : NSObject {
    int key;
    NSString* value;
}

- (id)initWithKey:(int)theKey value:(NSString *)theValue;

- (int)key;
- (NSString *)value;

@end
