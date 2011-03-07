//
//  Options.h
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    RM_TEXTURED,
    RM_FLAT,
    RM_WIREFRAME
} ERenderMode;

extern NSString* const OptionsChanged;

@class Grid;

@interface Options : NSObject {
    @private
    Grid* grid;
    ERenderMode renderMode;
}

- (Grid *)grid;
- (ERenderMode)renderMode;

- (void)setRenderMode:(ERenderMode)theRenderMode;

@end
