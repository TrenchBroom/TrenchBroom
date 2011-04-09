//
//  TrackingLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "TrackingLayer.h"
#import "BoundsRenderer.h"
#import "Brush.h"
#import "Face.h"
#import "Camera.h"
#import "GLFontManager.h"

@implementation TrackingLayer

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if (self = [super init]) {
        brushGuideRenderer = [[BoundsRenderer alloc] initWithCamera:theCamera fontManager:theFontManager font:theFont];
    }
    
    return self;
}

- (void)addBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
}

- (void)removeBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
}

- (void)addFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
}

- (void)render:(RenderContext *)renderContext {
    glColor4f(1, 1, 0, 1);
    glDisable(GL_DEPTH_TEST);
    [brushGuideRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)dealloc {
    [brushGuideRenderer release];
    [super dealloc];
}

@end
