//
//  BrushGuideRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLFontManager;
@class Camera;
@protocol Brush;

@interface BrushGuideRenderer : NSObject {
    NSMutableDictionary* glStrings;
    NSMutableSet* brushes;
    GLFontManager* fontManager;
    NSFont* font;
    Camera* camera;
}

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

- (void)addBrush:(id <Brush>)brush;
- (void)removeBrush:(id <Brush>)brush;

- (void)render;

@end
