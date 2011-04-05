//
//  TrackingLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "TrackingLayer.h"
#import "BrushGuideRenderer.h"
#import "Brush.h"
#import "Face.h"
#import "Camera.h"
#import "GLFont.h"

@implementation TrackingLayer

- (id)initWithCamera:(Camera *)theCamera glFont:(GLFont *)theGlFont {
    if (self = [super init]) {
        brushGuideRenderer = [[BrushGuideRenderer alloc] initWithCamera:theCamera glFont:theGlFont];
    }
    
    return self;
}

- (void)addBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    [brushGuideRenderer addBrush:theBrush];
}

- (void)removeBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    [brushGuideRenderer removeBrush:theBrush];
}

- (void)addFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
}

- (void)render:(RenderContext *)renderContext {
    glColor4f(1, 1, 1, 1);
    glDisable(GL_DEPTH_TEST);
    [brushGuideRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)dealloc {
    [brushGuideRenderer release];
    [super dealloc];
}

@end
