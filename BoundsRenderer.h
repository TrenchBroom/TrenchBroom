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
    NSMutableSet* brushes;
    NSMutableDictionary* brushCounts;
    GLFontManager* fontManager;
    NSFont* font;
    TBoundingBox bounds;
    GLString* widthStr;
    GLString* heightStr;
    GLString* depthStr;
    BOOL valid;
    Camera* camera;
}

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

- (void)addBrush:(id <Brush>)brush;
- (void)removeBrush:(id <Brush>)brush;

- (void)render;

@end
