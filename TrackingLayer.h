//
//  TrackingBrushLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BrushLayer.h"

@class BoundsRenderer;
@class Camera;
@class GLFontManager;

@interface TrackingLayer : NSObject <BrushLayer> {
    BoundsRenderer* brushGuideRenderer;
}

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

@end
