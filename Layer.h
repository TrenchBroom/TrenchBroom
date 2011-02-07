//
//  Layer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RenderContext.h"
#import "Brush.h"

@protocol Layer

- (void)render:(RenderContext *)renderContext;

- (void)addBrush:(Brush *)theBrush;
- (void)removeBrush:(Brush *)theBrush;

@end
