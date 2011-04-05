//
//  BrushGuideRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLFont;
@class Camera;
@protocol Brush;

@interface BrushGuideRenderer : NSObject {
    NSMutableDictionary* glStrings;
    NSMutableSet* brushes;
    GLFont* glFont;
    Camera* camera;
}

- (id)initWithCamera:(Camera *)theCamera glFont:(GLFont *)theGlFont;

- (void)addBrush:(id <Brush>)brush;
- (void)removeBrush:(id <Brush>)brush;

- (void)render;

@end
