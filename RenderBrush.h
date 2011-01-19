//
//  RenderBrush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RenderContext.h"

@class Brush;

@interface RenderBrush : NSObject {
    @private
    BOOL valid;
    Brush* brush;
}

- (id)initWithBrush:(Brush *)aBrush;

- (Brush *)brush;

- (void)renderWithContext:(id <RenderContext>)renderContext;

@end
