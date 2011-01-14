//
//  RenderBrush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Brush;
@class RenderContext;

@interface RenderBrush : NSObject {
    @private
    NSMutableSet* polygons;
    BOOL polygonsValid;
    Brush* brush;
}

- (id)initWithBrush:(Brush *)aBrush;

- (Brush *)brush;
- (void)setBrush:(Brush *)aBrush;

- (void)renderWithContext:(RenderContext *)context;

- (void)brushChanged:(NSNotification *)notification;

@end
