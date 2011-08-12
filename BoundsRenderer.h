//
//  BrushGuideRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class GLFontManager;
@class GLString;
@class Camera;
@protocol Brush;

@interface BoundsRenderer : NSObject {
    GLFontManager* fontManager;
    TBoundingBox bounds;
    BOOL boundsSet;
    BOOL valid;
    GLString* glStrings[3];
    Camera* camera;
}

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager;

- (void)setBounds:(TBoundingBox *)theBounds;

- (void)renderColor:(const TVector4f *)theColor;

@end
