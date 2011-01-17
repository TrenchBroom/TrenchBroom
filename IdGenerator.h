//
//  IdGenerator.h
//  TrenchBroom
//
//  Created by Kristian Duske on 17.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface IdGenerator : NSObject {
    @private
    int currentId;
}

+ (IdGenerator *)sharedGenerator;

- (NSNumber *)getId;

@end
