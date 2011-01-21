//
//  WadEntry.h
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WadEntry : NSObject {
    @private
    NSString* name;
}

- (id)initWithName:(NSString *)theName;

- (NSString *)name;

@end
